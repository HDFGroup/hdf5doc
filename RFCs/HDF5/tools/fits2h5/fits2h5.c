
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdf.ncsa.uiuc.edu/HDF5/doc/Copyright.html.  If you do not have     *
 * access to either file, you may request a copy from hdfhelp@ncsa.uiuc.edu. *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <string.h>
#include <stdio.h>
#include "fitsio.h"
#include "hdf5.h"
#if 1
#include "H5TBprivate.h"
#include "H5IMprivate.h"

#endif

void         make_fits_test();
void         print_error( int status);
static char* make_h5filename( const char *str );

/*-------------------------------------------------------------------------
 * fits2hdf5
 *
 * Purpose: Convert a FITS file to HDF5. Usage:  fits2hdf5 FILENAME, where FILENAME
 *  is a FITS filename
 *
 * Return: FITS error status
 *
 * Programmer: Pedro Vicente Nunes, pvn@ncsa.uiuc.edu
 *
 * Date: December 7, 2005
 *
 * Comments:
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */

int main(int argc, char *argv[])
{
 hid_t    fid;                     /* HDF5 file ID */
 fitsfile *fptr;                   /* FITS file pointer */
 char     *filename;               /* HDF5 filename */
 char     keyname[FLEN_KEYWORD];
 char     colname[FLEN_VALUE];
 char     coltype[FLEN_VALUE];
 int      status = 0;              /* CFITSIO status value MUST be initialized to zero! */
 int      hdupos;
 int      hdutype;
 int      bitpix;
 int      naxis;
 int      ncols;
 int      ii;
 long     naxes[10];
 hsize_t  dims[10];
 long     nrows;
 int      anynull;
 long     fpixel;
 long     npixels;
 int      nullval;
 void     *buf;
 char     dsetname[100];
 
 
 make_fits_test();
 
/*-------------------------------------------------------------------------
 * print a usage message if no FITS filename given
 *-------------------------------------------------------------------------
 */
 
 if (argc != 2) {
  printf("Usage:  fits2h5 filename\n");
  printf("\n");
  printf("Convert a FITS file to HDF5 \n");
  return(0);
 }
 
/*-------------------------------------------------------------------------
 * open and read FITS file, saving HDF5 objects along the way
 *-------------------------------------------------------------------------
 */
 
 if (!fits_open_file(&fptr, argv[1], READONLY, &status))
 {
  
/*-------------------------------------------------------------------------
 * create a HDF5 file
 *-------------------------------------------------------------------------
 */
  
  filename = make_h5filename(argv[1]);
  
  /* create a file using default properties */
  if ((fid=H5Fcreate(filename,H5F_ACC_TRUNC,H5P_DEFAULT,H5P_DEFAULT))<0)
   goto out;
  
  free(filename);
  
  /* get the current HDU position */
  fits_get_hdu_num(fptr, &hdupos);  
  
  /* main loop for each HDU */
  for (; !status; hdupos++)   
  {
   /* get the HDU type */
   fits_get_hdu_type(fptr, &hdutype, &status);  
   
   printf("\nHDU #%d  ", hdupos);
   
  /*-------------------------------------------------------------------------
   * primary array or image HDU
   *-------------------------------------------------------------------------
   */
   
   if (hdutype == IMAGE_HDU)  
   {
    fits_get_img_param(fptr, 10, &bitpix, &naxis, naxes, &status);
    
    printf("Array:  NAXIS = %d,  BITPIX = %d\n", naxis, bitpix);
    for (ii = 0; ii < naxis; ii++)
    {
     printf("   NAXIS%d = %ld\n",ii+1, naxes[ii]); 
     
    }
    
   /*-------------------------------------------------------------------------
    * read image
    *-------------------------------------------------------------------------
    */
    
    /* number of pixels in the image */
    npixels = 1;
    for (ii = 0; ii < naxis; ii++)
    {
     dims[ii] = naxes[ii];
     npixels *= naxes[ii];    
    }
      
    /* first pixel */
    fpixel   = 1;
    /* don't check for null values in the image */
    nullval  = 0;  
    
    buf = malloc(npixels * sizeof(int) );

   
    
    if (fits_read_img(fptr, TINT, fpixel, npixels, &nullval, buf, &anynull, &status) )
     print_error( status );
    
    /*-------------------------------------------------------------------------
     * save image
     *-------------------------------------------------------------------------
     */
    
    sprintf(dsetname,"IMAGE_%d",hdupos);
#if 1
    if ( H5LTmake_dataset_int(fid, dsetname, naxis, dims, buf ) < 0 )
        goto out;
    
   
#endif
    
    free(buf);
    buf = NULL;
   }
   
  /*-------------------------------------------------------------------------
   * table HDU
   *-------------------------------------------------------------------------
   */
   
   else  
   {
    fits_get_num_rows(fptr, &nrows, &status);
    fits_get_num_cols(fptr, &ncols, &status);
    
    if (hdutype == ASCII_TBL)
     printf("ASCII Table:  ");
    else
     printf("Binary Table:  ");
    
    printf("%d columns x %ld rows\n", ncols, nrows);
    printf(" COL NAME             FORMAT\n");
    
    for (ii = 1; ii <= ncols; ii++)
    {
     fits_make_keyn("TTYPE", ii, keyname, &status); /* make keyword */
     fits_read_key(fptr, TSTRING, keyname, colname, NULL, &status);
     fits_make_keyn("TFORM", ii, keyname, &status); /* make keyword */
     fits_read_key(fptr, TSTRING, keyname, coltype, NULL, &status);
     
     printf(" %3d %-16s %-16s\n", ii, colname, coltype);
    }
   }
   
  /*-------------------------------------------------------------------------
   * try move to next extension
   *-------------------------------------------------------------------------
   */
   
   fits_movrel_hdu(fptr, 1, NULL, &status);  
  }
  
  
 /*-------------------------------------------------------------------------
  * close 
  *-------------------------------------------------------------------------
  */
  
  /* reset normal error */
  if (status == END_OF_FILE) status = 0; 
  fits_close_file(fptr, &status);
  
  if (H5Fclose(fid)<0)
   goto out;
 }
 
out:
 
 /* print any error message */
 if (status) 
 {
  fits_report_error(stderr, status); 
 }
 
 return(status);
}


/*-------------------------------------------------------------------------
 * Function:    make_fits_test
 *
 * Purpose:     make a FITS test file 
 *
 * Programmer:  Pedro Vicente Nunes, pvn@ncsa.uiuc.edu
 *
 * Date:        December 20, 2005
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */

void make_fits_test()
{
 fitsfile *fptr;                                   /* pointer to the FITS file */
 int      status = 0;                              /* CFITSIO status           */
 char     filename[] = "test1.fit";                /* name for new FITS file   */
 long     fpixel;
 long     nelements;
 int      bitpix   =  LONG_IMG;                    /* 32-bit int pixel values */
 long     naxis    =   2;                          /* 2-dimensional image                */    
 long     naxes[2] = { 4, 3 };                     /* image is 4 pixels wide by 3 rows */
 int      buf[12] = {1,2,3,4,5,6,7,8,9,10,11,12};  /* data buffer */
 
 /* initialize status before calling fitsio routines */
 status = 0;  
 
 /* delete old file if it already exists */
 remove(filename);              
 
 if (fits_create_file(&fptr, filename, &status)) /* create new FITS file */
  print_error( status );      
 
 /* create an image */
 if ( fits_create_img(fptr, bitpix, naxis, naxes, &status) )
  print_error( status );          
 
 /* first pixel to write      */
 fpixel = 1;
 
 /* number of pixels to write */
 nelements = naxes[0] * naxes[1];          
 
 /* write the data array to the FITS file */
 if ( fits_write_img(fptr, TINT, fpixel, nelements, buf, &status) )
  print_error( status );

 /* create an image */
 if ( fits_create_img(fptr, bitpix, naxis, naxes, &status) )
  print_error( status );          
 
 /* first pixel to write      */
 fpixel = 1;
 
 /* number of pixels to write */
 nelements = naxes[0] * naxes[1];          
 
 /* write the data array to the FITS file */
 if ( fits_write_img(fptr, TINT, fpixel, nelements, buf, &status) )
  print_error( status );
 
 /* close the file */
 if ( fits_close_file(fptr, &status) )               
  print_error( status );          
 
}


/*-------------------------------------------------------------------------
 * Function:    print_error
 *
 * Purpose:     Print out cfitsio error messages and exit program
 *
 *-------------------------------------------------------------------------
 */

void print_error( int status)
{
 
 if (status)
 {
  /* print error report */
  fits_report_error(stderr, status); 
  
  /* terminate the program, returning error status */
  exit( status );    
 }
 return;
}

/*-------------------------------------------------------------------------
 * Function:    make_h5filename
 *
 * Purpose:     Append the extension ".h5" to an existing filename and remove any
 *               extension from the original filename
 *
 * Return:      The ".h5" filename
 *
 * Programmer:  Pedro Vicente Nunes, pvn@ncsa.uiuc.edu
 *
 * Date:        December 20, 2005
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static char*
make_h5filename(const char *str)
{
 size_t      len;
 size_t      i, j;
 const char  *cp;
 char        *rcp;
 int         found=0;
 
 if (!str)
  return NULL;
 
 cp = str;
 len = strlen(str);
 j = len;
 
 for (i = 0; i < len; i++) 
 {
  if (*cp == '.') 
  {
   j = i;
   break;
  } 
  cp++;
 }
 
 /* null terminator + case of no extension */
 rcp = malloc(len + 1 + 3);
 strncpy(rcp,str,j+1);
 cp = rcp;
 
 for (i = 0; i < len; i++) 
 {
  if (*cp == '.') 
  {
   strncpy(rcp + i + 1, "h5", strlen("h5"));
   rcp[i+3] = '\0';
   found = 1;
   break;
  }
  cp++;
 }

 /* no extension found */
 if (!found)
 {
  strncpy(rcp + len, ".h5", strlen(".h5"));
  rcp[len+3] = '\0';
 }
  
 return rcp;
}


