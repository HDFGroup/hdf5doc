/* Simple benchmark for appending new records to an extensible array
 */

/* Header files needed */
#include <stdlib.h>
#include "benchmark.h"
#include "H5EAprivate.h"	/* Extensible Arrays			*/
#include "H5Vprivate.h"         /* Vector functions			*/


/* Local Macros */
#define BENCH_NAME              "earray"
#define DEFAULT_FILENAME        "earray.h5"

/* Extensible array creation values */
#define ELMT_SIZE               sizeof(haddr_t)
#define MAX_NELMTS_BITS         32                      /* i.e. 4 giga-elements */
#define IDX_BLK_ELMTS           4
#define SUP_BLK_MIN_DATA_PTRS   4
#define DATA_BLK_MIN_ELMTS      16
#define MAX_DBLOCK_PAGE_NELMTS_BITS     10              /* i.e. 1024 elements per data block page */


/* Local Typedefs */


/* Local Prototypes */

/* Extensible array class callbacks */
static herr_t earray_fill(void *nat_blk, size_t nelmts);
static herr_t earray_encode(void *raw, const void *elmt, size_t nelmts);
static herr_t earray_decode(const void *raw, void *elmt, size_t nelmts);
static herr_t earray_debug(FILE *stream, int indent, int fwidth,
    hsize_t idx, const void *elmt);

/* Local Variables */

/* Extensible array testing class information */
const H5EA_class_t H5EA_CLS_BENCH[1]={{
    H5EA_CLS_TEST_ID + 1,       /* Type of Extensible array */
    sizeof(haddr_t),            /* Size of native element */
    earray_fill,                /* Fill block of missing elements callback */
    earray_encode,              /* Element encoding callback */
    earray_decode,              /* Element decoding callback */
    earray_debug                /* Element debugging callback */
}};

static H5F_t *f_g;              /* File pointer */

/* Local Routines */


/*-------------------------------------------------------------------------
 * Function:	earray_fill
 *
 * Purpose:	Fill "missing elements" in block of elements
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
earray_fill(void *nat_blk, size_t nelmts)
{
    /* Local variables */
    haddr_t fill_val = HADDR_UNDEF;          /* Value to fill elements with */

    /* Sanity checks */
    HDassert(nat_blk);
    HDassert(nelmts);

    H5V_array_fill(nat_blk, &fill_val, sizeof(haddr_t), nelmts);

    return(SUCCEED);
} /* end earray_fill() */


/*-------------------------------------------------------------------------
 * Function:	earray_encode
 *
 * Purpose:	Encode an element from "native" to "raw" form
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
earray_encode(void *raw, const void *_elmt, size_t nelmts)
{
    /* Local variables */
    const haddr_t *elmt = (const haddr_t *)_elmt;     /* Convenience pointer to native elements */

    /* Sanity checks */
    HDassert(raw);
    HDassert(elmt);
    HDassert(nelmts);

    /* Encode native elements into raw elements */
    while(nelmts) {
        /* Encode element */
        /* (advances 'raw' pointer */
        H5F_addr_encode(f_g, (uint8_t **)&raw, *elmt);

        /* Advance native element pointer */
        elmt++;

        /* Decrement # of elements to encode */
        nelmts--;
    } /* end while */

    return(SUCCEED);
} /* end earray_encode() */


/*-------------------------------------------------------------------------
 * Function:	earray_decode
 *
 * Purpose:	Decode an element from "raw" to "native" form
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
earray_decode(const void *_raw, void *_elmt, size_t nelmts)
{
    /* Local variables */
    haddr_t *elmt = (haddr_t *)_elmt;     /* Convenience pointer to native elements */
    const uint8_t *raw = (const uint8_t *)_raw; /* Convenience pointer to raw elements */

    /* Sanity checks */
    HDassert(raw);
    HDassert(elmt);
    HDassert(nelmts);

    /* Decode raw elements into native elements */
    while(nelmts) {
        /* Decode element */
        /* (advances 'raw' pointer */
        H5F_addr_decode(f_g, &raw, elmt);

        /* Advance native element pointer */
        elmt++;

        /* Decrement # of elements to decode */
        nelmts--;
    } /* end while */

    return(SUCCEED);
} /* end earray_decode() */


/*-------------------------------------------------------------------------
 * Function:	earray_debug
 *
 * Purpose:	Display an element for debugging
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
earray_debug(FILE *stream, int indent, int fwidth, hsize_t idx, const void *elmt)
{
    /* Local variables */
    char temp_str[128];     /* Temporary string, for formatting */

    /* Sanity checks */
    HDassert(stream);
    HDassert(elmt);

    /* Print element */
    sprintf(temp_str, "Element #%llu:", (unsigned long_long)idx);
    HDfprintf(stream, "%*s%-*s %a\n", indent, "", fwidth, temp_str,
        *(const haddr_t *)elmt);

    return(SUCCEED);
} /* end earray_debug() */

static void
earray_write(bench_config_t *conf)
{
    hid_t fid;                  /* File ID */
    hid_t dxpl;                 /* DXPL ID */
    H5F_t *f;                   /* File pointer */
    H5EA_t *ea;                 /* Extensible array */
    H5EA_create_t cparam;       /* Extensible array creation parameters */
    haddr_t ea_addr;            /* Address of extensible array created */
    size_t u;                   /* Local index variable */
    herr_t ret;                 /* Return value status */

    /* Create the file to operate on */
    fid = H5Fcreate(conf->filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    HDassert(fid > 0);
    f = (H5F_t *)H5I_object(fid);
    HDassert(f);
f_g = f;

    /* Get a copy of the default dataset transfer property list */
    dxpl = H5Pcreate(H5P_DATASET_XFER);
    HDassert(fid > 0);

    /* Set up the extensible array creation parameters */
    HDmemset(&cparam, 0, sizeof(cparam));
    cparam.cls = H5EA_CLS_BENCH;
    cparam.raw_elmt_size = ELMT_SIZE;
    cparam.max_nelmts_bits = MAX_NELMTS_BITS;
    cparam.idx_blk_elmts = IDX_BLK_ELMTS;
    cparam.sup_blk_min_data_ptrs = SUP_BLK_MIN_DATA_PTRS;
    cparam.data_blk_min_elmts = DATA_BLK_MIN_ELMTS;
    cparam.max_dblk_page_nelmts_bits = MAX_DBLOCK_PAGE_NELMTS_BITS;

    /* Create the extensible array */
    ea = H5EA_create(f, dxpl, &cparam);
    HDassert(ea);
    ret = H5EA_get_addr(ea, &ea_addr);
HDfprintf(stdout, "Address of extensible array = %a\n", ea_addr);
    HDassert(ret >= 0);

    /* Get timestamp before performing I/O */
    gettimeofday(&conf->before_io, NULL);

    /* Insert records into extensible array */
    for(u = 0; u < conf->nrecs; u++) {
        haddr_t val;            /* Record to store in extensible array */

        /* Provide some feedback to user */
        if(conf->verbose)
            if(0 == u % (conf->nrecs / 8))
                HDfprintf(stderr, "Writing record: %Zu\n", u);

        /* Set up record info */
        val = (haddr_t)u + 1;

        /* Set array element value */
        ret = H5EA_set(ea, dxpl, (hsize_t)u, &val);
        HDassert(ret >= 0);
    } /* end for */

    /* Get timestamp after performing I/O */
    gettimeofday(&conf->after_io, NULL);

    /* Close the extensible array */
    ret = H5EA_close(ea, dxpl);
    HDassert(ret >= 0);

    /* Close the DXPL we created */
    ret = H5Pclose(dxpl);
    HDassert(ret >= 0);

    /* Close the file we created */
    ret = H5Fclose(fid);
    HDassert(ret >= 0);
} /* end earray_write() */

static void
earray_read(bench_config_t *conf)
{
    hid_t fid;                  /* File ID */
    hid_t dxpl;                 /* DXPL ID */
    H5F_t *f;                   /* File pointer */
    H5EA_t *ea;                 /* Extensible array */
    size_t u;                   /* Local index variable */
    herr_t ret;                 /* Return value status */

    /* Open the file to operate on */
    fid = H5Fopen(conf->filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    HDassert(fid > 0);
    f = (H5F_t *)H5I_object(fid);
    HDassert(f);
f_g = f;

    /* Get a copy of the default dataset transfer property list */
    dxpl = H5Pcreate(H5P_DATASET_XFER);
    HDassert(dxpl > 0);

    /* Open the extensible array */
    ea = H5EA_open(f, dxpl, conf->ds_addr, H5EA_CLS_BENCH);
    HDassert(ea);

    /* Get timestamp before performing I/O */
    gettimeofday(&conf->before_io, NULL);

    /* Read records from extensible array */
    for(u = 0; u < conf->nrecs; u++) {
        haddr_t val;            /* Record to store in extensible array */

        /* Provide some feedback to user */
        if(conf->verbose)
            if(0 == u % (conf->nrecs / 8))
                HDfprintf(stderr, "Reading record: %Zu\n", u);

        /* Set up record info */
        val = HADDR_UNDEF;

        /* Get array element value */
        ret = H5EA_get(ea, dxpl, (hsize_t)u, &val);
        HDassert(ret >= 0);

        /* Verify record */
        HDassert(val == ((haddr_t)u + 1));
    } /* end for */

    /* Get timestamp after performing I/O */
    gettimeofday(&conf->after_io, NULL);

    /* Close the extensible array */
    ret = H5EA_close(ea, dxpl);
    HDassert(ret >= 0);

    /* Close the DXPL we created */
    ret = H5Pclose(dxpl);
    HDassert(ret >= 0);

    /* Close the file we created */
    ret = H5Fclose(fid);
    HDassert(ret >= 0);
} /* end earray_read() */

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
        earray_write(&conf);
    else
        earray_read(&conf);

    /* Get timestamp after closing file */
    gettimeofday(&conf.after_close, NULL);

    /* Display times */
    display_results(&conf);

    return(0);
}

