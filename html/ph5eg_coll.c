/*
 * Collective read/write for fixed dimension datasets.
 */

#include <ph5eg.h>

/*
 * Example of using the parallel HDF5 library to create two datasets
 * in one HDF5 file with collective parallel access support.
 * The Datasets are of sizes (number-of-mpi-processes x DIM1) x DIM2.
 * Each process controls only a slab of size DIM1 x DIM2 within each
 * dataset. [Note: not so yet.  Datasets are of sizes DIM1xDIM2 and
 * each process controls a hyperslab within.]
 */

void
dataset_writeAll(char *filename)
{
    hid_t fid;                  /* HDF5 file ID */
    hid_t acc_tpl;		/* File access templates */
    hid_t xfer_plist;		/* Dataset transfer properties list */
    hid_t sid;   		/* Dataspace ID */
    hid_t file_dataspace;	/* File dataspace ID */
    hid_t mem_dataspace;	/* memory dataspace ID */
    hid_t dataset1, dataset2;	/* Dataset ID */
    hid_t datatype;		/* Datatype ID */
    hsize_t dims[RANK] = {DIM1,DIM2};	/* dataset dim sizes */
    DATATYPE data_array1[DIM1][DIM2];	/* data buffer */

    hssize_t   start[RANK];			/* for hyperslab setting */
    hsize_t count[RANK], stride[RANK];		/* for hyperslab setting */

    herr_t ret;         	/* Generic return value */
    int mpi_size, mpi_rank;
    
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;

    if (verbose)
	printf("Collective write test on file %s\n", filename);

    /* set up MPI parameters */
    MPI_Comm_size(MPI_COMM_WORLD,&mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD,&mpi_rank);

    /* -------------------
     * START AN HDF5 FILE 
     * -------------------*/
    /* setup file access template with parallel IO access. */
    acc_tpl = H5Pcreate (H5P_FILE_ACCESS);
    VRFY((acc_tpl != FAIL), "H5Pcreate access succeeded");
    /* set Parallel access with communicator */
    ret = H5Pset_mpi(acc_tpl, comm, info);     
    VRFY((ret != FAIL), "H5Pset_mpi succeeded");

    /* create the file collectively */
    fid=H5Fcreate(filename,H5F_ACC_TRUNC,H5P_DEFAULT,acc_tpl);
    VRFY((fid != FAIL), "H5Fcreate succeeded");

    /* Release file-access template */
    ret=H5Pclose(acc_tpl);
    VRFY((ret != FAIL), "");


    /* --------------------------
     * Define the dimensions of the overall datasets
     * and create the dataset
     * ------------------------- */
    /* setup dimensionality object */
    sid = H5Screate_simple (RANK, dims, NULL);
    VRFY((sid != FAIL), "H5Screate_simple succeeded");

    
    /* create a dataset collectively */
    dataset1 = H5Dcreate(fid, DATASETNAME1, H5T_NATIVE_INT, sid, H5P_DEFAULT);
    VRFY((dataset1 != FAIL), "H5Dcreate succeeded");

    /* create another dataset collectively */
    datatype = H5Tcopy(H5T_NATIVE_INT);
    ret = H5Tset_order(datatype, H5T_ORDER_LE);
    VRFY((ret != FAIL), "H5Tset_order succeeded");

    dataset2 = H5Dcreate(fid, DATASETNAME2, datatype, sid, H5P_DEFAULT);
    VRFY((dataset2 != FAIL), "H5Dcreate 2 succeeded");

    /*
     * Set up dimensions of the slab this process accesses.
     */

    /* Dataset1: each process takes a block of rows. */
    slab_set(mpi_rank, mpi_size, start, count, stride, BYROW);

    /* create a file dataspace independently */
    file_dataspace = H5Dget_space (dataset1);				    
    VRFY((file_dataspace != FAIL), "H5Dget_space succeeded");
    ret=H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, start, stride, count, NULL); 
    VRFY((ret != FAIL), "H5Sset_hyperslab succeeded");

    /* create a memory dataspace independently */
    mem_dataspace = H5Screate_simple (RANK, count, NULL);
    VRFY((mem_dataspace != FAIL), "");

    /* fill the local slab with some trivial data */
    dataset_fill(start, count, stride, &data_array1[0][0]);
    MESG("data_array initialized");
    if (verbose){
	MESG("data_array created");
	dataset_print(start, count, stride, &data_array1[0][0]);
    }

    /* set up the collective transfer properties list */
    xfer_plist = H5Pcreate (H5P_DATASET_XFER);
    VRFY((xfer_plist != FAIL), "");
    ret=H5Pset_xfer(xfer_plist, H5D_XFER_COLLECTIVE);
    VRFY((ret != FAIL), "H5Pcreate xfer succeeded");

    /* write data collectively */
    ret = H5Dwrite(dataset1, H5T_NATIVE_INT, mem_dataspace, file_dataspace,
	    xfer_plist, data_array1);					    
    VRFY((ret != FAIL), "H5Dwrite dataset1 succeeded");

    /* release all temporary handles. */
    /* Could have used them for dataset2 but it is cleaner */
    /* to create them again.*/
    H5Sclose(file_dataspace);
    H5Sclose(mem_dataspace);
    H5Pclose(xfer_plist);

    /* Dataset2: each process takes a block of columns. */
    slab_set(mpi_rank, mpi_size, start, count, stride, BYCOL);

    /* put some trivial data in the data_array */
    dataset_fill(start, count, stride, &data_array1[0][0]);
    MESG("data_array initialized");
    if (verbose){
	MESG("data_array created");
	dataset_print(start, count, stride, &data_array1[0][0]);
    }

    /* create a file dataspace independently */
    file_dataspace = H5Dget_space (dataset1);				    
    VRFY((file_dataspace != FAIL), "H5Dget_space succeeded");
    ret=H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, start, stride, count, NULL); 
    VRFY((ret != FAIL), "H5Sset_hyperslab succeeded");

    /* create a memory dataspace independently */
    mem_dataspace = H5Screate_simple (RANK, count, NULL);
    VRFY((mem_dataspace != FAIL), "");

    /* fill the local slab with some trivial data */
    dataset_fill(start, count, stride, &data_array1[0][0]);
    MESG("data_array initialized");
    if (verbose){
	MESG("data_array created");
	dataset_print(start, count, stride, &data_array1[0][0]);
    }

    /* set up the collective transfer properties list */
    xfer_plist = H5Pcreate (H5P_DATASET_XFER);
    VRFY((xfer_plist != FAIL), "");
    ret=H5Pset_xfer(xfer_plist, H5D_XFER_COLLECTIVE);
    VRFY((ret != FAIL), "H5Pcreate xfer succeeded");

    /* write data independently */
    ret = H5Dwrite(dataset2, H5T_NATIVE_INT, mem_dataspace, file_dataspace,
	    xfer_plist, data_array1);					    
    VRFY((ret != FAIL), "H5Dwrite dataset2 succeeded");

    /* release all temporary handles. */
    H5Sclose(file_dataspace);
    H5Sclose(mem_dataspace);
    H5Pclose(xfer_plist);


    /*
     * All writes completed.  Close datasets collectively
     */					    
    ret=H5Dclose(dataset1);
    VRFY((ret != FAIL), "H5Dclose1 succeeded");
    ret=H5Dclose(dataset2);
    VRFY((ret != FAIL), "H5Dclose2 succeeded");

    /* release all IDs created */
    H5Sclose(sid);

    /* close the file collectively */					    
    H5Fclose(fid);							    
}

/*
 * Example of using the parallel HDF5 library to read two datasets
 * in one HDF5 file with collective parallel access support.
 * The Datasets are of sizes (number-of-mpi-processes x DIM1) x DIM2.
 * Each process controls only a slab of size DIM1 x DIM2 within each
 * dataset. [Note: not so yet.  Datasets are of sizes DIM1xDIM2 and
 * each process controls a hyperslab within.]
 */

void
dataset_readAll(char *filename)
{
    hid_t fid;                  /* HDF5 file ID */
    hid_t acc_tpl;		/* File access templates */
    hid_t xfer_plist;		/* Dataset transfer properties list */
    hid_t sid;   		/* Dataspace ID */
    hid_t file_dataspace;	/* File dataspace ID */
    hid_t mem_dataspace;	/* memory dataspace ID */
    hid_t dataset1, dataset2;	/* Dataset ID */
    hsize_t dims[] = {DIM1,DIM2};   	/* dataset dim sizes */
    DATATYPE data_array1[DIM1][DIM2];	/* data buffer */
    DATATYPE data_origin1[DIM1][DIM2];	/* expected data buffer */

    hssize_t   start[RANK];			/* for hyperslab setting */
    hsize_t count[RANK], stride[RANK];		/* for hyperslab setting */

    herr_t ret;         	/* Generic return value */
    int mpi_size, mpi_rank;

    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;

    if (verbose)
	printf("Collective read test on file %s\n", filename);

    /* set up MPI parameters */
    MPI_Comm_size(MPI_COMM_WORLD,&mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD,&mpi_rank);

    /* -------------------
     * OPEN AN HDF5 FILE 
     * -------------------*/
    /* setup file access template with parallel IO access. */
    acc_tpl = H5Pcreate (H5P_FILE_ACCESS);
    VRFY((acc_tpl != FAIL), "H5Pcreate access succeeded");
    /* set Parallel access with communicator */
    ret = H5Pset_mpi(acc_tpl, comm, info);     
    VRFY((ret != FAIL), "H5Pset_mpi succeeded");

    /* open the file collectively */
    fid=H5Fopen(filename,H5F_ACC_RDONLY,acc_tpl);
    VRFY((fid != FAIL), "H5Fopen succeeded");

    /* Release file-access template */
    ret=H5Pclose(acc_tpl);
    VRFY((ret != FAIL), "");


    /* --------------------------
     * Open the datasets in it
     * ------------------------- */
    /* open the dataset1 collectively */
    dataset1 = H5Dopen(fid, DATASETNAME1);
    VRFY((dataset1 != FAIL), "H5Dopen succeeded");

    /* open another dataset collectively */
    dataset2 = H5Dopen(fid, DATASETNAME2);
    VRFY((dataset2 != FAIL), "H5Dopen 2 succeeded");

    /*
     * Set up dimensions of the slab this process accesses.
     */

    /* Dataset1: each process takes a block of columns. */
    slab_set(mpi_rank, mpi_size, start, count, stride, BYCOL);

    /* create a file dataspace independently */
    file_dataspace = H5Dget_space (dataset1);				    
    VRFY((file_dataspace != FAIL), "H5Dget_space succeeded");
    ret=H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, start, stride, count, NULL); 
    VRFY((ret != FAIL), "H5Sset_hyperslab succeeded");

    /* create a memory dataspace independently */
    mem_dataspace = H5Screate_simple (RANK, count, NULL);
    VRFY((mem_dataspace != FAIL), "");

    /* fill dataset with test data */
    dataset_fill(start, count, stride, &data_origin1[0][0]);
    MESG("data_array initialized");
    if (verbose){
	MESG("data_array created");
	dataset_print(start, count, stride, &data_origin1[0][0]);
    }

    /* set up the collective transfer properties list */
    xfer_plist = H5Pcreate (H5P_DATASET_XFER);
    VRFY((xfer_plist != FAIL), "");
    ret=H5Pset_xfer(xfer_plist, H5D_XFER_COLLECTIVE);
    VRFY((ret != FAIL), "H5Pcreate xfer succeeded");

    /* read data collectively */
    ret = H5Dread(dataset1, H5T_NATIVE_INT, mem_dataspace, file_dataspace,
	    xfer_plist, data_array1);					    
    VRFY((ret != FAIL), "H5Dread succeeded");

    /* verify the read data with original expected data */
    ret = dataset_vrfy(start, count, stride, &data_array1[0][0], &data_origin1[0][0]);
    if (ret) nerrors++;

    /* release all temporary handles. */
    /* Could have used them for dataset2 but it is cleaner */
    /* to create them again.*/
    H5Sclose(file_dataspace);
    H5Sclose(mem_dataspace);
    H5Pclose(xfer_plist);

    /* Dataset2: each process takes a block of rows. */
    slab_set(mpi_rank, mpi_size, start, count, stride, BYROW);

    /* create a file dataspace independently */
    file_dataspace = H5Dget_space (dataset1);				    
    VRFY((file_dataspace != FAIL), "H5Dget_space succeeded");
    ret=H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, start, stride, count, NULL); 
    VRFY((ret != FAIL), "H5Sset_hyperslab succeeded");

    /* create a memory dataspace independently */
    mem_dataspace = H5Screate_simple (RANK, count, NULL);
    VRFY((mem_dataspace != FAIL), "");

    /* fill dataset with test data */
    dataset_fill(start, count, stride, &data_origin1[0][0]);
    MESG("data_array initialized");
    if (verbose){
	MESG("data_array created");
	dataset_print(start, count, stride, &data_origin1[0][0]);
    }

    /* set up the collective transfer properties list */
    xfer_plist = H5Pcreate (H5P_DATASET_XFER);
    VRFY((xfer_plist != FAIL), "");
    ret=H5Pset_xfer(xfer_plist, H5D_XFER_COLLECTIVE);
    VRFY((ret != FAIL), "H5Pcreate xfer succeeded");

    /* read data independently */
    ret = H5Dread(dataset2, H5T_NATIVE_INT, mem_dataspace, file_dataspace,
	    xfer_plist, data_array1);					    
    VRFY((ret != FAIL), "H5Dread succeeded");

    /* verify the read data with original expected data */
    ret = dataset_vrfy(start, count, stride, &data_array1[0][0], &data_origin1[0][0]);
    if (ret) nerrors++;

    /* release all temporary handles. */
    H5Sclose(file_dataspace);
    H5Sclose(mem_dataspace);
    H5Pclose(xfer_plist);


    /*
     * All reads completed.  Close datasets collectively
     */					    
    ret=H5Dclose(dataset1);
    VRFY((ret != FAIL), "H5Dclose1 succeeded");
    ret=H5Dclose(dataset2);
    VRFY((ret != FAIL), "H5Dclose2 succeeded");

    /* close the file collectively */					    
    H5Fclose(fid);							    
}
