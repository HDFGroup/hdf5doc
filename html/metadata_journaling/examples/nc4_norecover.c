/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
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
 * This is the companion to the recover example program for netCDF-4 with the 
 * h5recover tool.  It creates two netCDF-4 data files, one the test file, the 
 * other the control file, which is synced after every write to preserve the data.
 * The two files are intended to contain identical data. The control file will
 * be completed with no error but the test file, though written with the same
 * data, is intentionally crashed without proper file flush or close. For this
 * non-recovery example the test file is unrecoverable.
 *
 *
 * Creator: Larry Knox, October 21, 2008.
 */


#include <hdf5.h>
#include <signal.h>
#include <sys/time.h>
#include <math.h>
#include <stdlib.h>

#include <netcdf.h>
#include <nc_tests.h>

#define RANK   2
#define NX     NC_UNLIMITED                    /* dataset dimensions */
#define NY     16 
#define ChunkX 256
#define ChunkY 16
#define H5FILE_NAME        "tnorecover.nc"
#define CTL_H5FILE_NAME    "CTL"H5FILE_NAME	/* control file name */

//#define NXSTRIDE 1
//#define NXSTRIDE 16
//#define NXSTRIDE 256
//#define NXSTRIDE 512
#define NXSTRIDE 32768

/* Handle errors by printing an error message and exiting with a
 * non-zero status. */
#define ERRCODE 2
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}

/* Global variables */
int		fileid, ctl_fileid;         /* file id and control file id*/


int
create_files(char *filename, char *ctl_filename)
{
    int 	retval, typeid, varid;

    nc_set_log_level(5);


    /*
     * Create a new file and the control file using H5F_ACC_TRUNC access,
     * default file creation properties, and default file
     * access properties.
     */
    retval = nc_create(filename, NC_CLOBBER | NC_NETCDF4, &fileid);
    retval += nc_create(ctl_filename, NC_CLOBBER | NC_NETCDF4, &ctl_fileid);

    return retval;
}

int
close_file(hid_t fid)
{
    nc_close(fid);
}

void
writer(int rank)
{
   int 	x_dimid, y_dimid, ctlx_dimid, ctly_dimid, nvarid, ctlnvarid, ctlvarid[5000], varid[5000];
   hid_t       dataset;         /* dataset handle */
   //hid_t       datatype, dataspace, plist;      /* handles */
   herr_t      status;                             
   int         i, j, k, l;
   int dimids[rank];
   int ctldimids[rank];
   int chunks[rank];
   int ctlchunks[rank];
   int data_out[NXSTRIDE][NY];
   int x, y, retval;
   size_t start[]={0,0};
   size_t  count[]={NXSTRIDE,NY};
   double value;

   printf("Define dimensions.\n");
   /* Define the dimensions. */
   if ((retval = nc_def_dim(fileid, "x", NX, &x_dimid)))
      ERR(retval);
   if ((retval = nc_def_dim(fileid, "y", NY, &y_dimid)))
      ERR(retval);
   /* Set up variable data. */
   dimids[0] = x_dimid;
   dimids[1] = y_dimid;
   chunks[0] = 4;
   chunks[1] = 4;

   if ((retval = nc_def_dim(ctl_fileid, "x", NC_UNLIMITED, &ctlx_dimid)))
      ERR(retval);
   if ((retval = nc_def_dim(ctl_fileid, "y", NY, &ctly_dimid)))
      ERR(retval);
   /* Set up variable data. */
   ctldimids[0] = ctlx_dimid;
   ctldimids[1] = ctly_dimid;
   ctlchunks[0] = 4;
   ctlchunks[1] = 4;
   
   // Define the variables. 
   if ((retval = nc_def_var(fileid, "data", NC_INT, rank,
                            dimids, &nvarid)))
      ERR(retval);
   if ((retval = nc_def_var_chunking(fileid, nvarid, 0, chunks)))
      ERR(retval);

   if ((retval = nc_def_var(ctl_fileid, "data", NC_INT, rank,
                            ctldimids, &ctlnvarid)))
      ERR(retval);
   if ((retval = nc_def_var_chunking(ctl_fileid, ctlnvarid, 0, ctlchunks)))
      ERR(retval);

   printf("Syncing control file.\n");
   if ((retval = nc_sync(ctl_fileid)))
      ERR(retval);
   printf("Done.\n");

   for (i = 0; i < 10; i++) {
      for (j = 0; j < NXSTRIDE; j++) {
         for (k = 0; k < NY; k++) {
            data_out[j][k] = i + j + k;
         }
      }

      start[0] = i * NXSTRIDE;        
      if ((retval = nc_put_vara_int(fileid, nvarid, start, count, &data_out[0][0])))
         ERR(retval);
      if ((retval = nc_put_vara_int(ctl_fileid, ctlnvarid, start, count, &data_out[0][0])))
         ERR(retval);
       
      printf("Syncing control file: i=%d.\n", i);
      if ((retval = nc_sync(ctl_fileid)))
         ERR(retval);
      printf("Change DataLastUpdated value to %d.\n", i);
      value = (double)i;
      if (nc_put_att_double(ctl_fileid, NC_GLOBAL, "DataLastUpdated", NC_DOUBLE, 1, &value))
         ERR(retval);
      
      if ((retval = nc_sync(ctl_fileid)))
         ERR(retval);
      printf("Done.\n");

      if (i == 3) { 
         pid_t mypid = getpid();
         kill(mypid, SIGTERM);      /* Terminate myself */
         break;
      }
    
      printf("Syncing test file.");
      if ((retval = nc_sync(fileid)))
         ERR(retval);
      if (nc_put_att_double(fileid, NC_GLOBAL, "DataLastUpdated", NC_DOUBLE, 1, &value))
         ERR(retval);
      if ((retval = nc_sync(fileid)))
         ERR(retval);
      printf("Done.\n");
      
   }

    return;
}     

int
main (int ac, char **av)
{
    hsize_t     dims[RANK]={NX,NY};              /* dataset dimensions */
    hsize_t     dimschunk[RANK]={ChunkX,ChunkY}; /* dataset chunk dimensions */

    /* create/open both files. */
    create_files(H5FILE_NAME, CTL_H5FILE_NAME);
    
    /* create datasets in files. */
    writer(RANK);
    
    /* Close files - will only be reached if kill in writer is disabled. */
    close_file(fileid);
    close_file(ctl_fileid);

    return(0);
}     

