/* Simple benchmark for appending new records to a v2 B-tree
 */

/* Header files needed */
#include <stdlib.h>
#include "benchmark.h"
#include "H5B2private.h"	/* v2 B-trees				*/


/* Local Macros */
#define BENCH_NAME              "btree2"
#define DEFAULT_FILENAME        "btree2.h5"


/* Local Typedefs */

/* v2 B-tree record */
typedef struct btree2_rec_t {
    uint64_t    key;            /* Key for record */
    haddr_t     val;            /* "value" for record */
} btree2_rec_t;


/* Local Prototypes */

/* v2 B-tree callbacks */
static herr_t btree2_store(void *nrecord, const void *udata);
static herr_t btree2_retrieve(void *udata, const void *nrecord);
static herr_t btree2_compare(const void *rec1, const void *rec2);
static herr_t btree2_encode(const H5F_t *f, uint8_t *raw,
    const void *nrecord);
static herr_t btree2_decode(const H5F_t *f, const uint8_t *raw,
    void *nrecord);
static herr_t btree2_debug(FILE *stream, const H5F_t *f, hid_t dxpl_id,
    int indent, int fwidth, const void *record, const void *_udata);


/* Local Variables */

const H5B2_class_t H5B2_BENCH[1]={{   /* B-tree class information */
    H5B2_TEST_ID + 1,           /* Type of B-tree */
    sizeof(btree2_rec_t),       /* Size of native record */
    btree2_store,               /* Record storage callback */
    btree2_retrieve,            /* Record retrieval callback */
    btree2_compare,             /* Record comparison callback */
    btree2_encode,              /* Record encoding callback */
    btree2_decode,              /* Record decoding callback */
    btree2_debug                /* Record debugging callback */
}};

/* Local Routines */


/*-------------------------------------------------------------------------
 * Function:	btree2_store
 *
 * Purpose:	Store native information into record for B-tree
 *
 * Return:	Success:	non-negative
 *		Failure:	negative
 *
 * Programmer:	Quincey Koziol
 *              Tuesday, December  2, 2008
 *
 *-------------------------------------------------------------------------
 */
static herr_t
btree2_store(void *nrecord, const void *udata)
{
    *(btree2_rec_t *)nrecord = *(const btree2_rec_t *)udata;

    return(SUCCEED);
} /* btree2_store() */


/*-------------------------------------------------------------------------
 * Function:	btree2_retrieve
 *
 * Purpose:	Retrieve native information from record for B-tree
 *
 * Return:	Success:	non-negative
 *		Failure:	negative
 *
 * Programmer:	Quincey Koziol
 *              Tuesday, December  2, 2008
 *
 *-------------------------------------------------------------------------
 */
static herr_t
btree2_retrieve(void *udata, const void *nrecord)
{
    *(btree2_rec_t *)udata = *(const btree2_rec_t *)nrecord;

    return(SUCCEED);
} /* btree2_retrieve() */


/*-------------------------------------------------------------------------
 * Function:	btree2_compare
 *
 * Purpose:	Compare two native information records, according to some key
 *
 * Return:	<0 if rec1 < rec2
 *              =0 if rec1 == rec2
 *              >0 if rec1 > rec2
 *
 * Programmer:	Quincey Koziol
 *              Tuesday, December  2, 2008
 *
 *-------------------------------------------------------------------------
 */
static herr_t
btree2_compare(const void *rec1, const void *rec2)
{
    return((herr_t)(((const btree2_rec_t *)rec1)->key - ((const btree2_rec_t *)rec2)->key));
} /* btree2_compare() */


/*-------------------------------------------------------------------------
 * Function:	btree2_encode
 *
 * Purpose:	Encode native information into raw form for storing on disk
 *
 * Return:	Success:	non-negative
 *		Failure:	negative
 *
 * Programmer:	Quincey Koziol
 *              Tuesday, December  2, 2008
 *
 *-------------------------------------------------------------------------
 */
static herr_t
btree2_encode(const H5F_t *f, uint8_t *raw, const void *nrecord)
{
    UINT64ENCODE(raw, ((const btree2_rec_t *)nrecord)->key);
    H5F_addr_encode(f, &raw, ((const btree2_rec_t *)nrecord)->val);

    return(SUCCEED);
} /* btree2_encode() */


/*-------------------------------------------------------------------------
 * Function:	btree2_decode
 *
 * Purpose:	Decode raw disk form of record into native form
 *
 * Return:	Success:	non-negative
 *		Failure:	negative
 *
 * Programmer:	Quincey Koziol
 *              Tuesday, December  2, 2008
 *
 *-------------------------------------------------------------------------
 */
static herr_t
btree2_decode(const H5F_t *f, const uint8_t *raw, void *nrecord)
{
    UINT64DECODE(raw, ((btree2_rec_t *)nrecord)->key);
    H5F_addr_decode(f, &raw, &(((btree2_rec_t *)nrecord)->val));

    return(SUCCEED);
} /* btree2_decode() */


/*-------------------------------------------------------------------------
 * Function:	btree2_debug
 *
 * Purpose:	Debug native form of record
 *
 * Return:	Success:	non-negative
 *		Failure:	negative
 *
 * Programmer:	Quincey Koziol
 *              Tuesday, December  2, 2008
 *
 *-------------------------------------------------------------------------
 */
static herr_t
btree2_debug(FILE *stream, const H5F_t UNUSED *f, hid_t UNUSED dxpl_id,
    int indent, int fwidth, const void *record, const void UNUSED *_udata)
{
    HDassert(record);

    HDfprintf(stream, "%*s%-*s {%Hu, %a}\n", indent, "", fwidth, "Record:",
        ((const btree2_rec_t *)record)->key, ((const btree2_rec_t *)record)->val);

    return(SUCCEED);
} /* btree2_debug() */

static void
btree2_write(bench_config_t *conf)
{
    hid_t fid;                  /* File ID */
    hid_t dxpl;                 /* DXPL ID */
    H5F_t *f;                   /* File pointer */
    haddr_t bt2_addr;           /* Address of v2 B-tree created */
    size_t u;                   /* Local index variable */
    herr_t ret;                 /* Return value status */

    /* Create the file to operate on */
    fid = H5Fcreate(conf->filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    HDassert(fid > 0);
    f = (H5F_t *)H5I_object(fid);
    HDassert(f);

    /* Get a copy of the default dataset transfer property list */
    dxpl = H5Pcreate(H5P_DATASET_XFER);
    HDassert(fid > 0);

    /* Create the v2 B-tree */
    ret = H5B2_create(f, dxpl, H5B2_BENCH, (size_t)512, (size_t)16, 100, 40, &bt2_addr/*out*/);
HDfprintf(stdout, "Address of v2 B-tree = %a\n", bt2_addr);
    HDassert(ret >= 0);
    HDassert(H5F_addr_defined(bt2_addr));

    /* Get timestamp before performing I/O */
    gettimeofday(&conf->before_io, NULL);

    /* Insert records into v2 B-tree */
    for(u = 0; u < conf->nrecs; u++) {
        btree2_rec_t rec;       /* Record to store in v2 B-tree */

        /* Provide some feedback to user */
        if(conf->verbose)
            if(0 == u % (conf->nrecs / 8))
                HDfprintf(stderr, "Writing record: %Zu\n", u);

        /* Set up record info */
        rec.key = (uint64_t)u;
        rec.val = (haddr_t)u + 1;

        /* Insert record */
        ret = H5B2_insert(f, dxpl, H5B2_BENCH, bt2_addr, &rec);
        HDassert(ret >= 0);
    } /* end for */

    /* Get timestamp after performing I/O */
    gettimeofday(&conf->after_io, NULL);

    /* Close the DXPL we created */
    ret = H5Pclose(dxpl);
    HDassert(ret >= 0);

    /* Close the file we created */
    ret = H5Fclose(fid);
    HDassert(ret >= 0);
} /* end btree2_write() */


/*-------------------------------------------------------------------------
 * Function:	btree2_find_cb
 *
 * Purpose:	v2 B-tree find callback
 *
 * Return:	Success:	TRUE/FALSE
 *		Failure:	FAIL
 *
 * Programmer:	Quincey Koziol
 *              Wednesday, December  3, 2008
 *
 *-------------------------------------------------------------------------
 */
static int
btree2_find_cb(const void *_record, void *_op_data)
{
    const btree2_rec_t *record = (const btree2_rec_t *)_record;
    btree2_rec_t *search = (btree2_rec_t *)_op_data;

    if(record->key == search->key) {
        search->val = record->val;
        return(TRUE);
    } /* end if */
    else
        return(FALSE);
} /* end btree2_find_cb() */


static void
btree2_read(bench_config_t *conf)
{
    hid_t fid;                  /* File ID */
    hid_t dxpl;                 /* DXPL ID */
    H5F_t *f;                   /* File pointer */
    size_t u;                   /* Local index variable */
    herr_t ret;                 /* Return value status */

    /* Create the file to operate on */
    fid = H5Fopen(conf->filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    HDassert(fid > 0);
    f = (H5F_t *)H5I_object(fid);
    HDassert(f);

    /* Get a copy of the default dataset transfer property list */
    dxpl = H5Pcreate(H5P_DATASET_XFER);
    HDassert(dxpl > 0);

    /* Get timestamp before performing I/O */
    gettimeofday(&conf->before_io, NULL);

    /* Read records from v2 B-tree */
    for(u = 0; u < conf->nrecs; u++) {
        btree2_rec_t rec;       /* Record to store in v2 B-tree */

        /* Provide some feedback to user */
        if(conf->verbose)
            if(0 == u % (conf->nrecs / 8))
                HDfprintf(stderr, "Reading record: %Zu\n", u);

        /* Set up record info */
        rec.key = (uint64_t)u;
        rec.val = HADDR_UNDEF;

        /* Find record */
        ret = H5B2_find(f, dxpl, H5B2_BENCH, conf->ds_addr, &rec, btree2_find_cb, &rec);
        HDassert(ret >= 0);

        /* Verify record */
        HDassert(rec.val == ((haddr_t)u + 1));
    } /* end for */

    /* Get timestamp after performing I/O */
    gettimeofday(&conf->after_io, NULL);

    /* Close the DXPL we created */
    ret = H5Pclose(dxpl);
    HDassert(ret >= 0);

    /* Close the file we created */
    ret = H5Fclose(fid);
    HDassert(ret >= 0);
} /* end btree2_read() */


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
        btree2_write(&conf);
    else
        btree2_read(&conf);

    /* Get timestamp after closing file */
    gettimeofday(&conf.after_close, NULL);

    /* Display times */
    display_results(&conf);

    return(0);
}

