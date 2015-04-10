/* Simple benchmark for appending new records to a v1 B-tree
 */

/* Header files needed */
#include <stdlib.h>
#include "benchmark.h"
#include "H5Bprivate.h"		/* B-link trees				*/


/* Local Macros */
#define BENCH_NAME              "btree"
#define DEFAULT_FILENAME        "btree.h5"


/* Local Typedefs */

/*
 * B-tree key.
 *
 * The record's file address is part of the B-tree and not part of the key.
 */
typedef struct btree_key_t {
    uint64_t    val;                    /* Value for key */
} btree_key_t;

/* User data for B-tree callbacks */
typedef struct btree_ud_t {
    uint64_t    val;                    /* Value for key */
    haddr_t     addr;                   /* Address for key */
    H5RC_t     *btree_shared;           /* Ref-counted info for B-tree nodes */
} btree_ud_t;

/* Local Prototypes */

/* B-tree callbacks */
static H5RC_t *btree_get_shared(const H5F_t *f, const void *_udata);
static herr_t btree_new_node(H5F_t *f, hid_t dxpl_id, H5B_ins_t, void *_lt_key,
    void *_udata, void *_rt_key, haddr_t *addr_p /*out*/);
static int btree_cmp2(H5F_t *f, hid_t dxpl_id, void *_lt_key, void *_udata,
    void *_rt_key);
static int btree_cmp3(H5F_t *f, hid_t dxpl_id, void *_lt_key, void *_udata,
    void *_rt_key);
static htri_t btree_found(H5F_t *f, hid_t dxpl_id, haddr_t addr, const void *_lt_key,
    void *_udata);
static H5B_ins_t btree_insert(H5F_t *f, hid_t dxpl_id, haddr_t addr, void *_lt_key,
    hbool_t *lt_key_changed, void *_md_key, void *_udata,
    void *_rt_key, hbool_t *rt_key_changed, haddr_t *new_node/*out*/);
static herr_t btree_decode_key(const H5F_t *f, const H5B_t *bt, const uint8_t *raw,
    void *_key);
static herr_t btree_encode_key(const H5F_t *f, const H5B_t *bt, uint8_t *raw,
    void *_key);
static herr_t btree_debug_key(FILE *stream, H5F_t *f, hid_t dxpl_id, int indent,
    int fwidth, const void *key, const void *udata);


/* Local Variables */

/* inherits B-tree like properties from H5B */
H5B_class_t H5B_BENCH[1] = {{
    H5B_ISTORE_ID,		/*id (this test is similar to chunked dataset indices) */
    sizeof(btree_key_t),	/*sizeof_nkey		*/
    btree_get_shared,		/*get_shared		*/
    btree_new_node,		/*new			*/
    btree_cmp2,			/*cmp2			*/
    btree_cmp3,			/*cmp3			*/
    btree_found,		/*found			*/
    btree_insert,		/*insert		*/
    FALSE,			/*follow min branch?	*/
    FALSE,			/*follow max branch?	*/
    NULL,          		/*remove		*/
    btree_decode_key,		/*decode		*/
    btree_encode_key,		/*encode		*/
    btree_debug_key,		/*debug			*/
}};


/* Local Routines */


/*-------------------------------------------------------------------------
 * Function:	btree_get_shared
 *
 * Purpose:	Returns the shared B-tree info for the specified UDATA.
 *
 * Return:	Success:	Pointer to the raw B-tree page for this dataset
 *		Failure:	Can't fail
 *
 * Programmer:	Quincey Koziol
 *		Tuesday, December  2, 2008
 *
 *-------------------------------------------------------------------------
 */
static H5RC_t *
btree_get_shared(const H5F_t UNUSED *f, const void *_udata)
{
    const btree_ud_t *udata = (const btree_ud_t *)_udata;

    HDassert(udata);
    HDassert(udata->btree_shared);

    /* Increment reference count on B-tree info */
    H5RC_INC(udata->btree_shared);

    /* Return the pointer to the ref-count object */
    return(udata->btree_shared);
} /* end btree_get_shared() */


/*-------------------------------------------------------------------------
 * Function:	btree_new_node
 *
 * Purpose:	Adds a new entry to B-tree.
 *
 * Return:	Success:	Non-negative. The address of leaf is returned
 *				through the ADDR argument.  It is also added
 *				to the UDATA.
 *
 * 		Failure:	Negative
 *
 * Programmer:	Quincey Koziol
 *		Tuesday, December  2, 2008
 *
 *-------------------------------------------------------------------------
 */
static herr_t
btree_new_node(H5F_t UNUSED *f, hid_t UNUSED dxpl_id, H5B_ins_t UNUSED op,
    void *_lt_key, void *_udata, void *_rt_key, haddr_t *addr_p/*out*/)
{
    btree_key_t	*lt_key = (btree_key_t *) _lt_key;
    btree_key_t	*rt_key = (btree_key_t *) _rt_key;
    btree_ud_t	*udata = (btree_ud_t *) _udata;

    /* check args */
    HDassert(lt_key);
    HDassert(rt_key);
    HDassert(udata);
    HDassert(addr_p);

    /* Allocate new storage - not needed, just set the record value as the address */
    *addr_p = (haddr_t)udata->addr;

    /* The left key describes the new address */
    lt_key->val = udata->val;

    /* Reset the right key value */
    if(H5B_INS_LEFT != op)
        rt_key->val = udata->val + 1;

    return(SUCCEED);
} /* end btree_new_node() */


/*-------------------------------------------------------------------------
 * Function:	btree_cmp2
 *
 * Purpose:	Compares two keys, sort of like strcmp().  The UDATA pointer
 *		is only to supply extra information not carried in the keys
 *		and is not compared against the keys.
 *
 * Return:	Success:	-1 if LT_KEY is less than RT_KEY;
 *				1 if LT_KEY is greater than RT_KEY;
 *				0 if LT_KEY and RT_KEY are equal.
 *
 *		Failure:	FAIL (same as LT_KEY<RT_KEY)
 *
 * Programmer:	Quincey Koziol
 *		Tuesday, December  2, 2008
 *
 *-------------------------------------------------------------------------
 */
static int
btree_cmp2(H5F_t UNUSED *f, hid_t UNUSED dxpl_id, void *_lt_key,
    void UNUSED *udata, void *_rt_key)
{
    btree_key_t	*lt_key = (btree_key_t *) _lt_key;
    btree_key_t	*rt_key = (btree_key_t *) _rt_key;
    int		ret_value;

    HDassert(lt_key);
    HDassert(rt_key);

    /* Compare the key values */
    if(lt_key->val < rt_key->val)
        ret_value = -1;
    else if(lt_key->val > rt_key->val)
        ret_value = 1;
    else
        ret_value = 0;

    return(ret_value);
} /* end btree_cmp2() */


/*-------------------------------------------------------------------------
 * Function:	btree_cmp3
 *
 * Purpose:	Compare the requested datum UDATA with the left and right
 *		keys of the B-tree.
 *
 * Return:	Success:	negative if the value of UDATA is less
 *				than the value of LT_KEY.
 *
 *				positive if the value of UDATA is
 *				greater than or equal the value of
 *				RT_KEY.
 *
 *				zero otherwise.	 The value of UDATA is
 *				not necessarily contained within the address
 *				space represented by LT_KEY, but a key that
 *				would describe the UDATA value address
 *				would fall lexicographically between LT_KEY
 *				and RT_KEY.
 *
 *		Failure:	FAIL (same as UDATA < LT_KEY)
 *
 * Programmer:	Quincey Koziol
 *		Tuesday, December  2, 2008
 *
 *-------------------------------------------------------------------------
 */
static int
btree_cmp3(H5F_t UNUSED *f, hid_t UNUSED dxpl_id, void *_lt_key, void *_udata,
    void *_rt_key)
{
    btree_key_t	*lt_key = (btree_key_t *) _lt_key;
    btree_key_t	*rt_key = (btree_key_t *) _rt_key;
    btree_ud_t	*udata = (btree_ud_t *) _udata;
    int		ret_value = 0;

    HDassert(lt_key);
    HDassert(rt_key);
    HDassert(udata);

    if(udata->val >= rt_key->val)
        ret_value = 1;
    else if(udata->val < lt_key->val)
        ret_value = (-1);

    return(ret_value);
} /* end btree_cmp3() */


/*-------------------------------------------------------------------------
 * Function:	btree_found
 *
 * Purpose:	This function is called when the B-tree search engine has
 *		found the leaf entry that points to a record 
 *		represented by UDATA.  The LT_KEY is the left key (the one
 *		that describes the record) and RT_KEY is the right key (the
 *		one that describes the next or last record).
 *
 * Note:	It's possible that the record isn't really found.  For
 *		instance, in a sparse dataset the requested record might fall
 *		between two keys in which case this function is
 *		called with the maximum keys less than the
 *		requested record key.
 *
 * Return:	Non-negative (TRUE/FALSE) on success with information about the
 *              record returned through the UDATA argument. Negative on failure.
 *
 * Programmer:	Quincey Koziol
 *		Tuesday, December  2, 2008
 *
 *-------------------------------------------------------------------------
 */
static htri_t
btree_found(H5F_t UNUSED *f, hid_t UNUSED dxpl_id, haddr_t addr,
    const void *_lt_key, void *_udata)
{
    btree_ud_t	   *udata = (btree_ud_t *) _udata;
    const btree_key_t *lt_key = (const btree_key_t *) _lt_key;
    htri_t      ret_value = FALSE;       /* Return value */

    /* Check arguments */
    HDassert(udata);
    HDassert(lt_key);

    /* Is this *really* the requested record? */
    if(udata->val == lt_key->val) {
        udata->addr = addr;
        ret_value = TRUE;
    } /* end if */

    return(ret_value);
} /* end btree_found() */


/*-------------------------------------------------------------------------
 * Function:	btree_insert
 *
 * Purpose:	This function is called when the B-tree insert engine finds
 *		the node to use to insert a new record.  The UDATA argument
 *		points to a struct that describes the key value being
 *		added to the B-tree.  This function allocates space for the
 *		data and returns information through UDATA describing a
 *		file chunk to receive (part of) the data.
 *
 *		The LT_KEY is always the key describing the record for
 *		address ADDR. On entry, UDATA describes the key value
 *		for the record (in the `key' field). On return,
 *              UDATA describes the logical address for the record.
 *
 * Return:	Success:	An insertion command for the caller, one of
 *				the H5B_INS_* constants.  The address of the
 *				new record is returned through the NEW_NODE
 *				argument.
 *
 *		Failure:	H5B_INS_ERROR
 *
 * Programmer:	Quincey Koziol
 *		Tuesday, December  2, 2008
 *
 *-------------------------------------------------------------------------
 */
static H5B_ins_t
btree_insert(H5F_t UNUSED *f, hid_t UNUSED dxpl_id, haddr_t UNUSED addr,
    void *_lt_key, hbool_t UNUSED *lt_key_changed,
    void *_md_key, void *_udata,
    void UNUSED *_rt_key, hbool_t UNUSED *rt_key_changed,
    haddr_t *new_node_p/*out*/)
{
    btree_key_t	*lt_key = (btree_key_t *) _lt_key;
    btree_key_t	*md_key = (btree_key_t *) _md_key;
    btree_ud_t	*udata = (btree_ud_t *) _udata;
    H5B_ins_t ret_value;

    /* check args */
    HDassert(lt_key);
    HDassert(md_key);
    HDassert(udata);

    /* Check for overwriting existing value */
    if(lt_key->val == udata->val) {
        if(H5F_addr_eq(udata->addr, addr)) {
            udata->addr = addr;
            ret_value = H5B_INS_NOOP;
        } /* end if */
        else {
            *new_node_p = udata->addr;
            ret_value = H5B_INS_CHANGE;
        } /* end else */
    } /* end if */
    else {
        md_key->val = udata->val;
        *new_node_p = udata->addr;
        ret_value = H5B_INS_RIGHT;
    } /* end else */

    return(ret_value);
} /* end btree_insert() */


/*-------------------------------------------------------------------------
 * Function:	btree_decode_key
 *
 * Purpose:	Decodes a raw key into a native key for the B-tree
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Tuesday, December  2, 2008
 *
 *-------------------------------------------------------------------------
 */
static herr_t
btree_decode_key(const H5F_t UNUSED *f, const H5B_t UNUSED *bt, const uint8_t *raw,
    void *_key)
{
    btree_key_t	*key = (btree_key_t *) _key;

    /* check args */
    HDassert(raw);
    HDassert(key);

    /* decode */
    UINT64DECODE(raw, key->val);

    return(SUCCEED);
} /* end btree_decode_key() */


/*-------------------------------------------------------------------------
 * Function:	btree_encode_key
 *
 * Purpose:	Encode a key from native format to raw format.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		Tuesday, December  2, 2008
 *
 *-------------------------------------------------------------------------
 */
static herr_t
btree_encode_key(const H5F_t UNUSED *f, const H5B_t UNUSED *bt, uint8_t *raw,
    void *_key)
{
    btree_key_t	*key = (btree_key_t *) _key;

    /* check args */
    HDassert(raw);
    HDassert(key);

    /* encode */
    UINT64ENCODE(raw, key->val);

    return(SUCCEED);
} /* end btree_encode_key() */


/*-------------------------------------------------------------------------
 * Function:	btree_debug_key
 *
 * Purpose:	Prints a key.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *              Tuesday, December  2, 2008
 *
 *-------------------------------------------------------------------------
 */
static herr_t
btree_debug_key(FILE *stream, H5F_t UNUSED *f, hid_t UNUSED dxpl_id, int indent,
    int fwidth, const void *_key, const void UNUSED *_udata)
{
    const btree_key_t	*key = (const btree_key_t *)_key;

    HDassert(key);

    HDfprintf(stream, "%*s%-*s %llu\n", indent, "", fwidth, "Value:", (unsigned long_long)key->val);

    return(SUCCEED);
} /* end btree_debug_key() */

static void
btree_write(bench_config_t *conf)
{
    hid_t fid;                  /* File ID */
    hid_t dxpl;                 /* DXPL ID */
    H5F_t *f;                   /* File pointer */
    H5B_shared_t *shared;       /* Shared B-tree node info */
    haddr_t btree_addr;         /* Address of v1 B-tree created */
    btree_ud_t udata;           /* User data for B-tree callbacks */
    size_t u;                   /* Local index variable */
    herr_t ret;                 /* Return value status */

    /* Create the file to operate on */
    fid = H5Fcreate(conf->filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    HDassert(fid > 0);

    f = (H5F_t *)H5I_object(fid);
    HDassert(f);

    /* Get a copy of the default dataset transfer property list */
    dxpl = H5Pcreate(H5P_DATASET_XFER);
    HDassert(dxpl > 0);

    /* Initialize the user data */
    HDmemset(&udata, 0, sizeof(udata));

    /* Create the shared B-tree info */

    /* Allocate & initialize global info for the shared structure */
    shared = H5B_shared_new(f, H5B_BENCH, (size_t)8);
    HDassert(shared);

    /* Make shared B-tree info reference counted */
    udata.btree_shared = H5RC_create(shared, H5B_shared_free);
    HDassert(udata.btree_shared);

    /* Create a v1 B-tree */
    ret = H5B_create(f, dxpl, H5B_BENCH, &udata, &btree_addr);
HDfprintf(stdout, "Address of B-tree = %a\n", btree_addr);
    HDassert(ret >= 0);
    HDassert(H5F_addr_defined(btree_addr));

    /* Get timestamp before performing I/O */
    gettimeofday(&conf->before_io, NULL);

    /* Insert records into v1 B-tree */
    for(u = 0; u < conf->nrecs; u++) {
        /* Provide some feedback to user */
        if(conf->verbose)
            if(0 == u % (conf->nrecs / 8))
                HDfprintf(stderr, "Writing record: %Zu\n", u);

        /* Set up record info */
        udata.val = (uint64_t)u;
        udata.addr = (haddr_t)u + 1;

        /* Insert record */
        ret = H5B_insert(f, dxpl, H5B_BENCH, btree_addr, &udata);
        HDassert(ret >= 0);

    } /* end for */

    /* Get timestamp after performing I/O */
    gettimeofday(&conf->after_io, NULL);

    /* Decrement the ref. count on the shared B-tree info */
    ret = H5RC_DEC(udata.btree_shared);
    HDassert(ret >= 0);

    /* Close the DXPL we created */
    ret = H5Pclose(dxpl);
    HDassert(ret >= 0);

    /* Close the file we created */
    ret = H5Fclose(fid);
    HDassert(ret >= 0);
}

static void
btree_read(bench_config_t *conf)
{
    hid_t fid;                  /* File ID */
    hid_t dxpl;                 /* DXPL ID */
    H5F_t *f;                   /* File pointer */
    H5B_shared_t *shared;       /* Shared B-tree node info */
    btree_ud_t udata;           /* User data for B-tree callbacks */
    size_t u;                   /* Local index variable */
    herr_t ret;                 /* Return value status */

    /* Open the file to operate on */
    fid = H5Fopen(conf->filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    HDassert(fid > 0);
    f = (H5F_t *)H5I_object(fid);
    HDassert(f);

    /* Get a copy of the default dataset transfer property list */
    dxpl = H5Pcreate(H5P_DATASET_XFER);
    HDassert(fid > 0);

    /* Initialize the user data */
    HDmemset(&udata, 0, sizeof(udata));

    /* Create the shared B-tree info */

    /* Allocate & initialize global info for the shared structure */
    shared = H5B_shared_new(f, H5B_BENCH, (size_t)8);
    HDassert(shared);

    /* Make shared B-tree info reference counted */
    udata.btree_shared = H5RC_create(shared, H5B_shared_free);
    HDassert(udata.btree_shared);

    /* Get timestamp before performing I/O */
    gettimeofday(&conf->before_io, NULL);

    /* Read records from v1 B-tree */
    for(u = 0; u < conf->nrecs; u++) {
        /* Provide some feedback to user */
        if(conf->verbose)
            if(0 == u % (conf->nrecs / 8))
                HDfprintf(stderr, "Reading record: %Zu\n", u);

        /* Set up record info */
        udata.val = (uint64_t)u;
        udata.addr = HADDR_UNDEF;

        /* Find record */
        ret = H5B_find(f, dxpl, H5B_BENCH, conf->ds_addr, &udata);
        HDassert(ret > 0);

        /* Verify record */
        HDassert(udata.addr == ((haddr_t)u + 1));
    } /* end for */

    /* Get timestamp after performing I/O */
    gettimeofday(&conf->after_io, NULL);

    /* Decrement the ref. count on the shared B-tree info */
    ret = H5RC_DEC(udata.btree_shared);
    HDassert(ret >= 0);

    /* Close the DXPL we created */
    ret = H5Pclose(dxpl);
    HDassert(ret >= 0);

    /* Close the file we created */
    ret = H5Fclose(fid);
    HDassert(ret >= 0);
}


int
main(int argc, char *argv[])
{
    bench_config_t conf;        /* Configuration for this run */

    /* Get default configuration settings */
    memcpy(&conf, &def_conf_g, sizeof(conf));
    conf.filename = strdup(DEFAULT_FILENAME);

    /* Parse command line options */
    parse_cmd_line(BENCH_NAME, &conf, argc, argv);

    /* Display configuration settings for this run */
    display_conf(&conf);

    /* Get timestamp before opening file */
    gettimeofday(&conf.before_open, NULL);

    /* Perform operation desired */
    if(conf.mode == BENCH_WRITE)
        btree_write(&conf);
    else
        btree_read(&conf);

    /* Get timestamp after closing file */
    gettimeofday(&conf.after_close, NULL);

    /* Display times */
    display_results(&conf);

    return(0);
}

