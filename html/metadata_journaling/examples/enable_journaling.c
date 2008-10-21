/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * This example shows how to use the Journaling feature. It simulates a typical
 * application that loops through the following steps:
 *   Calculate data
 *   Write Data to file
 *   Flush file
 *   Write a timecounter to file indicating success of last write/flush
 *   Repeat
 *
 * When the data file is opened again for updates, it retrieves the value
 * of the timecounter which indicates data are for sure good up to that time
 * step. It does more updates from that time step on.
 */

#include "hdf5.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define H5FILE_NAME        	"JournalEG.h5"
#define H5JournalFILE_NAME 	H5FILE_NAME".jnl"
#define H5recovertoolname 	"h5recover"
#define H5dumptoolname 		"h5dump"
#define ProgName	 	"enable_journaling"	/* program name */
#define DATASETNAME "IntArray"
#define TIMESTEPNAME "TimeStep"
#define NX     10                      /* dataset initial dimensions */
#define NY     10
#define CHUNKX 2			/* chunk dimensions */
#define CHUNKY 10
#define RANK   2
#define THRESHOLD       256
#define ALIGNMENT       512

/* Global variables */

/* protocols */
int writedata(hid_t file, hid_t dataset, int begin, int end, hid_t timestep);
void helppage(void);
int set_journal(hid_t faccpl, const char * journalname);

/* Display the online help page */
void
helppage(void)
{
    printf(
	"Usage:\n"
	"%s -[c|w]\n"
	"\t-c\tCreate a new file (%s)\n"
	"\t-w\tWrite more rows to the file with Journaling (%s) for crash test\n",
	ProgName, H5FILE_NAME, H5JournalFILE_NAME
    );
    printf("To try this program, run:\n");
    printf("\t%s -c\n", ProgName);
    printf("\t%s %s\n", H5dumptoolname, H5FILE_NAME);
    printf("\t%s -w\t\t# Crash it by ^C\n", ProgName);
    printf("\t%s %s (This should fail)\n", H5dumptoolname, H5FILE_NAME);
    printf("\t%s -j %s %s\n", H5recovertoolname, H5JournalFILE_NAME, H5FILE_NAME);
    printf("\t%s %s (This should succeed)\n", H5dumptoolname, H5FILE_NAME);
}


int
set_journal(hid_t faccpl, const char * journalname) 
{
    H5AC2_jnl_config_t jnl_config;

    /* get current journaling configuration */
    jnl_config.version = H5AC2__CURR_JNL_CONFIG_VER;

    if ( H5Pget_jnl_config(faccpl, &jnl_config) < 0 ) {
	fprintf(stderr, "H5Pget_jnl_config on faccpl failed\n");
	return(-1);
    }

    jnl_config.enable_journaling = 1;   /* turn on journaling */
    jnl_config.journal_recovered = 0;
    jnl_config.jbrb_buf_size = 8*1024;  /* multiples of sys buffer size*/
    jnl_config.jbrb_num_bufs = 2;
    jnl_config.jbrb_use_aio = 0;        /* only sync IO is supported */
    jnl_config.jbrb_human_readable = 1; /* only readable form is supported */
    strcpy(jnl_config.journal_file_path, journalname);

    if ( H5Pset_jnl_config(faccpl, &jnl_config) < 0 ) {
	fprintf(stderr, "H5Pset_jnl_config on faccpl failed\n");
	return(-1);
    }
    return(0);
}


int
main (int ac, char **av)
{
    hid_t       file, dataset;         /* file and dataset handles */
    hid_t       dataspace;   			/* data array handle */
    hid_t       timestep;   			/* timestep handle */
    hsize_t 	maxdims[RANK] = {H5S_UNLIMITED, NY}; /* Dataset dimensions */
						/* with unlimited rows. */
    hsize_t     dimsf[RANK]={NX, NY};           /* initial dataset dimensions */
    hsize_t     chunk[RANK]={CHUNKX, CHUNKY};	/* chunk dimensions */
    hid_t       dsetpl;			/* Dataset property list */
    hid_t       faccpl;			/* File access property list */
    int		cmode=0;		/* Create mode, overrides the others. */
    int		wmode=0;		/* write mod, default no. */
    int		timecounter, maxtimecounter;


    /* Parse different options:
     * Default: Create a new file and new dataset.
     * wmode: write more rows to an existing file with Journaling.
     *
     * How to use this:
     * ./enable_journaling	# create JournalEG.h5 file
     * ./enable_journaling -w	# reopen JournalEG.h5 with Journaling on and
     *			        # add more rows, then crash. JournalEG.h5
     *				# is not readable as an HDF5 file.
     * ./h5recover -j JournalEG.h5.jnl JournalEG.h5	# Recover the file.
     * 
     * 
     */
    if (ac<=1){
	helppage();
	return 1;
    }
    while (ac > 1){
	ac--;
	av++;
        if (strcmp("-c", *av) == 0){
	    cmode++;
	    printf("Create mode\n");
	}else if (strcmp("-w", *av) == 0){
	    wmode++;
	    printf("write mode\n");
	}else{
	    fprintf(stderr, "Unknown option (%s)\n", *av);
	    helppage();
	    return 1 ;
	}
    }

    /* Create a file access property list to be used by both create and 
     * reopen modes.
     * Use latest lib version needed by the Journaling feature.
     */
    faccpl = H5Pcreate(H5P_FILE_ACCESS);
    if (H5Pset_libver_bounds(faccpl, H5F_LIBVER_LATEST, H5F_LIBVER_LATEST) < 0){
	fprintf(stderr, "H5Pset_libver_bounds on data file failed\n");
	H5Pclose(faccpl);
	return(1);
    }
    /* Use alignment so that it is easier to octo dump the data file for */
    /* debugging purpose. */
    if(H5Pset_alignment(faccpl, (hsize_t)THRESHOLD, (hsize_t)ALIGNMENT) < 0){
	fprintf(stderr, "H5Pset_alignment on data file failed\n");
	H5Pclose(faccpl);
	return(1);
    }

    if (cmode){
	/*===================================================
	 * Default:
	 * Create a new file, create a new dataset
	 * of unlimited dimension, initialize size to NX*NY, write data,
         * close file.
	 *===================================================*/
	file = H5Fcreate(H5FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, faccpl);
	H5Pclose(faccpl);

	/*
	 * create the data space for fixed size dataset.
	 * Initial dimension is NX*NY.
	 */
	dataspace = H5Screate_simple(RANK, dimsf, maxdims);

	/* Create dataset creation property list */
	dsetpl = H5Pcreate(H5P_DATASET_CREATE);
	
	/* Enable chunking needed for unlimited dimesion */
	H5Pset_layout(dsetpl, H5D_CHUNKED);
	H5Pset_chunk(dsetpl, 2, chunk);

	/*
	 * Create a new dataset of native int within the file using defined dataspace
	 * and chunked storage type.
	 */
	dataset = H5Dcreate2(file, DATASETNAME, H5T_NATIVE_INT, dataspace,
			    H5P_DEFAULT, dsetpl, H5P_DEFAULT);
	H5Sclose(dataspace);
	H5Pclose(dsetpl);

	/* Also create a simple scalar file attribute to store the timestep counter. */
	dimsf[0] = 1;
	dataspace = H5Screate_simple(0, NULL, NULL);
	timestep = H5Acreate2(file, TIMESTEPNAME, H5T_NATIVE_INT, dataspace,
			    H5P_DEFAULT, H5P_DEFAULT);

	/* write the initial NX rows of data and timestep counter */
	writedata(file, dataset, 0, NX-1, timestep);

	H5Sclose(dataspace);
	H5Dclose(dataset);
	H5Aclose(timestep);
	H5Fclose(file);
	return(0);
    }
    if (wmode){
	/*===================================================
	 * wmode:
	 *    Reopen a previous file with Journaling on, extend the dataset
	 *    to 4NX rows, write data, crash.
	 *===================================================*/

	/* reopen the file with Journaling on. */
	/* Setup file access property list with journaling */

	/* change the following 1 to 0 to see the effect of no journaling. */
#if 1
	if (set_journal(faccpl, H5JournalFILE_NAME) < 0){
            fprintf(stderr, "set_journal on data file failed\n");
            H5Pclose(faccpl);
	    return(1);
	}
#endif
	/* Delete the journal file since journal code does not allow */
	/* existed journal file. */
	remove(H5JournalFILE_NAME);
	/* open the file with journaling property list */
	file = H5Fopen(H5FILE_NAME, H5F_ACC_RDWR, faccpl);

	/* close handle not needed any more. */
	H5Pclose(faccpl);

	/* Open dataset and timestep for modifications. */
	dataset=H5Dopen2(file, DATASETNAME, H5P_DEFAULT);
	timestep=H5Aopen(file, TIMESTEPNAME, H5P_DEFAULT);
	/* retrieve the previous good time step counter value. */
	H5Aread(timestep, H5T_NATIVE_INT, &timecounter);

	/* write another 10*NX new rows but allow crashing */
	maxtimecounter = timecounter + 10*NX;
	printf("Current timecounter=%d, adding more rows up to maximum=%d.\n",
	    timecounter, maxtimecounter);
	while (timecounter < maxtimecounter){
	    printf("adding data from %d to %d\n", timecounter+1,timecounter+NX);
	    writedata(file, dataset, timecounter+1, timecounter+NX, timestep);
	    timecounter += NX;
	}

	/* close all remaining handles. */
	H5Dclose(dataset);
	H5Aclose(timestep);
	H5Fclose(file);
	return(0);
    }
    return(0);
}


/* writedata:
 * write rows of data to dataset starting from begin to end rows inclusive.
 */
int
writedata(hid_t file, hid_t dataset, int begin, int end, hid_t timestep)
{
    int         data[NX][NY];          /* data to write */
    int         nrows, i, j;
    hid_t	memspace, dataspace;
    hsize_t	dims[RANK], start[RANK], count[RANK];
    herr_t	retcode;
    int		timecounter;


    if ((end < begin) || (begin < 0)){
	fprintf(stderr, "writedata: bad arguments (begin=%d, end=%d)\n", begin, end);
	return(-1);
    }
    nrows = end-begin+1;
    if (nrows>NX){
	fprintf(stderr, "writedata: too many rows (begin=%d, end=%d, NX=%d)\n",
	    begin, end, NX);
	return(-1);
    }
    /* extend the dataset to end rows. */
    dims[0] = end+1;
    dims[1] = NY;
    H5Dset_extent(dataset, dims);

    /* setup memory space. */
    dims[0]=NX;
    dims[1]=NY;
    memspace = H5Screate_simple(RANK, dims, NULL);

    dataspace = H5Dget_space(dataset);
    start[0]=begin;
    start[1]=0;
    count[0]=nrows;
    count[1]=NY;
    if ((retcode = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start,
		    NULL, count, NULL)) <0){
	fprintf(stderr, "H5Sselect_hyperslab failed\n");
	return(-1);
    };

    /* Initialize data buffer */
    for(i = 0; i < nrows; i++)
	for(j = 0; j < NY; j++)
	    data[i][j] = (i+begin)*NY + j;

    /*
     * Write the data to the dataset using default transfer properties.
     * Flush all data.
     * Finally, write the time step counter.
     */
    retcode = H5Dwrite(dataset, H5T_NATIVE_INT, memspace, dataspace, H5P_DEFAULT, data);

    /* Pause 2 seconds to allow manual abort. */
    timecounter=end;
    fprintf(stderr,
	"After data write for time step %d. Pause to allow crashing..."
	"Hit ^C to crash it", timecounter);
    sleep(2);
    fprintf(stderr, "\n");

    H5Sclose(memspace);
    H5Sclose(dataspace);
    H5Fflush(file, H5F_SCOPE_GLOBAL);
    fprintf(stderr, "File data flushed.\n");
    retcode = H5Awrite(timestep, H5T_NATIVE_INT, &timecounter);
    fprintf(stderr, "Time step counter updated.\n");
    
    
    return(retcode);
}
