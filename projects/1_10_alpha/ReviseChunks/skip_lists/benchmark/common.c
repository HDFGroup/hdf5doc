/* Common routines for data structure comparison benchmarks */

/* Header files needed */
#include "benchmark.h"
#include "H5Fprivate.h"		/* Files				*/

/* Package common variables */

/* Default configuration settings */
const bench_config_t def_conf_g = {
    /* Configured parameters */
    NULL,                       /* File name */
    BENCH_WRITE,                /* Type of benchmark to run */
    HADDR_UNDEF,                /* Address of data structure to open */
    (size_t)(1024 * 1024),      /* Number of records to operate on */
    (hbool_t)FALSE,             /* Verbose output? */

    /* Gathered information */
    {0, 0},                     /* Time stamp before opening file */
    {0, 0},                     /* Time stamp before performing I/O operation */
    {0, 0},                     /* Time stamp after performing I/O operation */
    {0, 0}                      /* Time stamp after closing file */
};




/*
 * Display command line usage
 */
__attribute__((noreturn)) void
usage(const char *bench_name)
{
    printf("Usage: %s [-h] [-v] [-n <number of records>] [-f <filename>] [-a <address of data structure>] [-m <mode>]\n", bench_name);
    printf("Options:\n");
    printf("  -h Display this message\n");
    printf("  -v Verbose feedback during I/O\n");
    printf("  -n <number of records>: Number of records to read/write (default: 1048576)\n");
    printf("  -f <filename>: Name of file to read/write (default: %s.h5)\n", bench_name);
    printf("  -a <address of data structure>: Address of data structure to read (required for reading)\n");
    printf("  -m <mode>: Mode of operation: 'read' or 'write' (default: write)\n");
    exit(1);
} /* end usage() */


/*
 * Parse any command line arguments
 */
void
parse_cmd_line(const char *bench_name, bench_config_t *conf, int argc, char *argv[])
{
    int cur_arg = 1;    /* Current argument to process */

    /* Loop over arguments */
    while(cur_arg < argc) {
        /* All command line options start with '-' */
        if(argv[cur_arg][0] != '-')
            usage(bench_name);

        /* Process command line switches & arguments */
        switch(argv[cur_arg][1]) {
            case 'm':
                /* Advance to next argument */
                cur_arg++;
                if(cur_arg >= argc)
                    usage(bench_name);

                /* Get mode setting */
                if(!strcmp(argv[cur_arg], "write"))
                    conf->mode = BENCH_WRITE;
                else if(!strcmp(argv[cur_arg], "read"))
                    conf->mode = BENCH_READ;
                else
                    usage(bench_name);
                break;

            case 'f':
                /* Release previous filename */
                free(conf->filename);

                /* Advance to next argument */
                cur_arg++;
                if(cur_arg >= argc)
                    usage(bench_name);

                /* Make a copy of the filename */
                conf->filename = strdup(argv[cur_arg]);
                break;

            case 'n':
                /* Advance to next argument */
                cur_arg++;
                if(cur_arg >= argc)
                    usage(bench_name);

                /* Get the number of records to operate with */
                conf->nrecs = (size_t)atoi(argv[cur_arg]);
                if(conf->nrecs == 0)
                    usage(bench_name);
                break;

            case 'a':
                /* Advance to next argument */
                cur_arg++;
                if(cur_arg >= argc)
                    usage(bench_name);

                /* Get the address of the data structure */
                conf->ds_addr = (size_t)atoi(argv[cur_arg]);
                if(!H5F_addr_defined(conf->ds_addr) || H5F_addr_eq(conf->ds_addr, 0))
                    usage(bench_name);
                break;

            case 'v':
                /* Set the "verbose" flag */
                conf->verbose = TRUE;
                break;

            default:
                usage(bench_name);
        } /* end switch */

        /* Advance to next argument */
        cur_arg++;
    } /* end while */

    /* Check for valid data structure address when reading */
    if(conf->mode == BENCH_READ && !H5F_addr_defined(conf->ds_addr))
        usage(bench_name);
} /* end parse_cmd_line() */

/*
 * Display configuration settings
 */
void
display_conf(const bench_config_t *conf)
{
    printf("\nConfigurable parameters: ('-h' for usage)\n");
    printf("\tBenchmark mode: %s\n", (conf->mode == BENCH_WRITE ? "write" : "read"));
    printf("\tFile name: '%s'\n", conf->filename);
    printf("\tNumber of records: %llu\n", (unsigned long_long)conf->nrecs);
    HDfprintf(stdout, "\tAddress of data structure: %a\n", conf->ds_addr);
    printf("\tVerbose: %s\n", (conf->verbose ? "Yes" : "No"));
} /* end display_conf() */

/*
 * Display timing results
 */
void
display_results(const bench_config_t *conf)
{
    double before_open = conf->before_open.tv_sec + (double)(conf->before_open.tv_usec / (double)(1000.0 * 1000.0));
    double after_close = conf->after_close.tv_sec + (double)(conf->after_close.tv_usec / (double)(1000.0 * 1000.0));
    double before_io = conf->before_io.tv_sec + (double)(conf->before_io.tv_usec / (double)(1000.0 * 1000.0));
    double after_io = conf->after_io.tv_sec + (double)(conf->after_io.tv_usec / (double)(1000.0 * 1000.0));

    printf("\nElapsed time between file open & close: %f\n", (after_close - before_open));
    printf("Elapsed time during I/O operations: %f\n", (after_io - before_io));
} /* end display_results() */



