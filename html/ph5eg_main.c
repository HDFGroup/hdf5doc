/*
 * Main driver of the Parallel HDF5 examples
 */

#include <ph5eg.h>

/* global variables */
int nerrors = 0;			/* errors count */
int verbose = 0;			/* verbose, default as no. */

#ifdef POOMA_ARCH
char *fileprefix = "pfs:/pfs/multi/tmp_1/your_own";
int fileprefixlen = 29;
#else
char *fileprefix = NULL;		/* file prefix, default as NULL */
int fileprefixlen = 0;			/* file prefix length, default as 0 */
#endif

herr_t (*old_func)(void*);		/* previous error handler */
void *old_client_data;			/* previous error handler arg.*/

/* other option flags */
int doread=1;				/* read test */
int dowrite=1;				/* write test */

/*
 * Show command usage
 */
void
usage()
{
    printf("Usage: testphdf5 [-r] [-w] [-v] [-f <prefix>]\n");
    printf("\t-f <prefix>\tfilename prefix\n");
    printf("\t-r\t\tno read\n");
    printf("\t-w\t\tno write\n");
    printf("\t-v\t\tverbose on\n");
    printf("\tdefault do write then read\n");
    printf("\n");
}


/*
 * parse the command line options
 */
int
parse_options(int argc, char **argv){
    while (--argc){
	if (**(++argv) != '-'){
	    break;
	}else{
	    switch(*(*argv+1)){
		case 'r':   doread = 0;
			    break;
		case 'w':   dowrite = 0;
			    break;
		case 'v':   verbose = 1;
			    break;
		case 'f':   if (--argc <= 0) {
				nerrors++;
				return(1);
			    } else if (**(++argv) == '-') {
				nerrors++;
				return(1);
			    } else if (**(argv) == '"') {
				fileprefixlen = strlen(*(argv)+1)-1;
				fileprefix = (char *)HDmalloc(fileprefixlen+1);
        			if (!fileprefix) {
				    printf("%s\n","memory allocation failed");
				    nerrors++;
				    return(1);
				}
				fileprefix = strncpy(fileprefix,*(argv)+1,fileprefixlen);
			    } else {
				fileprefixlen = strlen(*(argv));
				fileprefix = (char *)HDmalloc(fileprefixlen+1);
        			if (!fileprefix) {
				    printf("%s\n","memory allocation failed");
				    nerrors++;
				    return(1);
				}
				fileprefix = strncpy(fileprefix,*(argv),fileprefixlen);
			    }
			    if (fileprefixlen < 5) {
				nerrors++;
				return(1);
			    }
			    break;
		default:    usage();
			    nerrors++;
			    return(1);
	    }
	}
    }
    return(0);
}


main(int argc, char **argv)
{
    char    *filenames[]={ "ParaEg1.h5f",
			   "ParaEg2.h5f",
			   "ParaEg3.h5f" };

    int mpi_size, mpi_rank;				/* mpi variables */
    int i;
    char *tmpptr;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    if (MAINPROCESS){
	printf("===================================\n");
	printf("PHDF5 TESTS START\n");
	printf("===================================\n");
    }

    /* Make sure datasets can be divided into equal chunks by the processes */
    if ((DIM1 % mpi_size) || (DIM2 % mpi_size)){
	if (MAINPROCESS)
	    printf("DIM1(%d) and DIM2(%d) must be multiples of processes(%d)\n",
		    DIM1, DIM2, mpi_size);
	nerrors++;
	goto finish;
    }

    if (parse_options(argc, argv) != 0)
	goto finish;

    if (fileprefix != NULL) {
	for (i=0;i<3;i++) {
	    tmpptr = filenames[i];
            filenames[i] = (char *)HDmalloc ( fileprefixlen + strlen(tmpptr) + 2);
            if (!filenames[i]) {
		printf("%s\n","memory allocation failed");
		nerrors++;
		goto finish;
	    }
	    filenames[i] = strcpy(filenames[i],fileprefix);
	    if (fileprefix[fileprefixlen-1] != '/') filenames[i] = strcat(filenames[i],"/");
	    filenames[i] = strcat(filenames[i],tmpptr);
	    H5MM_xfree(tmpptr);
	}
    }

    if (dowrite){
	MPI_BANNER("testing dataset using split communicators...");
	test_split_comm_access(filenames);

	MPI_BANNER("testing dataset independent write...");
	dataset_writeInd(filenames[0]);

	MPI_BANNER("testing dataset collective write...");
	dataset_writeAll(filenames[1]);

	MPI_BANNER("testing extendible dataset independent write...");
	extend_writeInd(filenames[2]);
    }
    if (doread){
	MPI_BANNER("testing dataset independent read...");
	dataset_readInd(filenames[0]);

	MPI_BANNER("testing dataset collective read...");
	dataset_readAll(filenames[1]);

	MPI_BANNER("testing extendible dataset independent read...");
	extend_readInd(filenames[2]);
    }

    if (!(dowrite || doread)){
	usage();
	nerrors++;
    }

finish:
    if (MAINPROCESS){		/* only process 0 reports */
	printf("===================================\n");
	if (nerrors){
	    printf("***PHDF5 tests detected %d errors***\n", nerrors);
	}
	else{
	    printf("PHDF5 tests finished with no errors\n");
	}
	printf("===================================\n");
    }
    MPI_Finalize();

    return(nerrors);
}

