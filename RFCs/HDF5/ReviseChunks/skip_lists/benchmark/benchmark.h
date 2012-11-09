/* Common information for data structure comparison benchmarks */

#ifndef __BENCHMARK_H
#define __BENCHMARK_H

/* Header files needed */
#include "hdf5.h"
#include "H5Iprivate.h"		/* IDs			  		*/

/* Common typedefs */

/* Mode of operation for benchmark */
typedef enum bench_mode_t {BENCH_WRITE, BENCH_READ} bench_mode_t;

/* Benchmark configuration settings */
typedef struct bench_config_t {
    /* Configured parameters */
    char *filename;             /* Name of file to create */
    bench_mode_t mode;          /* Type of benchmark to run */
    haddr_t ds_addr;            /* Address of data structure to open */
    size_t nrecs;               /* Number of records to operate on */
    hbool_t verbose;            /* Whether to have verbose output */

    /* Gathered information */
    struct timeval before_open; /* Time stamp before opening file */
    struct timeval before_io;   /* Time stamp before performing I/O operation */
    struct timeval after_io;    /* Time stamp after performing I/O operation */
    struct timeval after_close; /* Time stamp after closing file */
} bench_config_t;

/* Common variables */

/* Default configuration */
extern const bench_config_t def_conf_g;


/* Common routines */
__attribute__((noreturn)) void usage(const char *bench_name);
void parse_cmd_line(const char *bench_name, bench_config_t *conf, int argc,
    char *argv[]);
void display_conf(const bench_config_t *conf);
void display_results(const bench_config_t *conf);

#endif /* __BENCHMARK_H */

