/*
 * Independent read/write for extendible datasets.
 */

#include <ph5eg.h>

/*
 * Example of using the parallel HDF5 library to create two extendible
 * datasets in one HDF5 file with independent parallel MPIO access support.
 * The Datasets are of sizes (number-of-mpi-processes x DIM1) x DIM2.
 * Each process controls only a slab of size DIM1 x DIM2 within each
 * dataset.
 */

void
extend_writeInd(char *filename)
{
    hid_t fid;                  /* HDF5 file ID */
    hid_t acc_tpl;		/* File access templates */
    hid_t sid;   		/* Dataspace ID */
    hid_t file_dataspace;	/* File dataspace ID */
    hid_t mem_dataspace;	/* memory dataspace ID */
    hid_t dataset1, dataset2;	/* Dataset ID */
    hsize_t dims[RANK] = {DIM1,DIM2};		/* dataset initial dim sizes */
    hsize_t max_dims[RANK] =
		{H5S_UNLIMITED, H5S_UNLIMITED};	/* dataset maximum dim sizes */
    hsize_t	dimslocal1[RANK] = {DIM1,DIM2};	/* local dataset dim sizes */
    DATATYPE	data_array1[DIM1][DIM2];	/* data buffer */
    hsize_t	chunk_dims[RANK] = {7, 13};	/* chunk sizes */
    hid_t	dataset_pl;			/* dataset create prop. list */

    hssize_t	start[RANK];	/* for hyperslab setting */
    hsize_t	count[RANK];	/* for hyperslab setting */
    hsize_t	stride[RANK];	/* for hyperslab setting */

    herr_t ret;         	/* Generic return value */
    int   i, j;
    int mpi_size, mpi_rank;
    char *fname;
    
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;

    if (verbose)
	printf("Extend independent write test on file %s\n", filename);

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


    /* --------------------------------------------------------------
     * Define the dimensions of the overall datasets and create them.
     * ------------------------------------------------------------- */

    /* set up dataset storage chunk sizes and creation property list */
    if (verbose)
	printf("chunks[]=%d,%d\n", chunk_dims[0], chunk_dims[1]);
    dataset_pl = H5Pcreate(H5P_DATASET_CREATE);
    VRFY((dataset_pl != FAIL), "H5Pcreate succeeded");
    ret = H5Pset_chunk(dataset_pl, RANK, chunk_dims);
    VRFY((ret != FAIL), "H5Pset_chunk succeeded");

    /* setup dimensionality object */
    /* start out with no rows, extend it later. */
    dims[0] = dims[1] = 0;
    sid = H5Screate_simple (RANK, dims, max_dims);
    VRFY((sid != FAIL), "H5Screate_simple succeeded");

    /* create an extendible dataset collectively */
    dataset1 = H5Dcreate(fid, DATASETNAME1, H5T_NATIVE_INT, sid, dataset_pl);
    VRFY((dataset1 != FAIL), "H5Dcreate succeeded");

    /* create another extendible dataset collectively */
    dataset2 = H5Dcreate(fid, DATASETNAME2, H5T_NATIVE_INT, sid, dataset_pl);
    VRFY((dataset2 != FAIL), "H5Dcreate succeeded");

    /* release resource */
    H5Sclose(sid);



    /* -------------------------
     * Test writing to dataset1
     * -------------------------*/
    /* set up dimensions of the slab this process accesses */
    slab_set(mpi_rank, mpi_size, start, count, stride, BYROW);

    /* put some trivial data in the data_array */
    dataset_fill(start, count, stride, &data_array1[0][0]);
    MESG("data_array initialized");
    if (verbose){
	MESG("data_array created");
	dataset_print(start, count, stride, &data_array1[0][0]);
    }

    /* create a memory dataspace independently */
    mem_dataspace = H5Screate_simple (RANK, count, NULL);
    VRFY((mem_dataspace != FAIL), "");

    /* Extend its current dim sizes before writing */
    dims[0] = DIM1;
    dims[1] = DIM2;
    ret = H5Dextend (dataset1, dims);
    VRFY((ret != FAIL), "H5Dextend succeeded");

    /* create a file dataspace independently */
    file_dataspace = H5Dget_space (dataset1);				    
    VRFY((file_dataspace != FAIL), "H5Dget_space succeeded");
    ret=H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, start, stride, count, NULL); 
    VRFY((ret != FAIL), "H5Sset_hyperslab succeeded");

    /* write data independently */
    ret = H5Dwrite(dataset1, H5T_NATIVE_INT, mem_dataspace, file_dataspace,
	    H5P_DEFAULT, data_array1);					    
    VRFY((ret != FAIL), "H5Dwrite succeeded");

    /* release resource */
    H5Sclose(file_dataspace);
    H5Sclose(mem_dataspace);


    /* -------------------------
     * Test writing to dataset2
     * -------------------------*/
    /* set up dimensions of the slab this process accesses */
    slab_set(mpi_rank, mpi_size, start, count, stride, BYCOL);

    /* put some trivial data in the data_array */
    dataset_fill(start, count, stride, &data_array1[0][0]);
    MESG("data_array initialized");
    if (verbose){
	MESG("data_array created");
	dataset_print(start, count, stride, &data_array1[0][0]);
    }

    /* create a memory dataspace independently */
    mem_dataspace = H5Screate_simple (RANK, count, NULL);
    VRFY((mem_dataspace != FAIL), "");

    /* Try write to dataset2 beyond its current dim sizes.  Should fail. */
    /* Temporary turn off auto error reporting */
    H5Eget_auto(&old_func, &old_client_data);
    H5Eset_auto(NULL, NULL);

    /* create a file dataspace independently */
    file_dataspace = H5Dget_space (dataset2);				    
    VRFY((file_dataspace != FAIL), "H5Dget_space succeeded");
    ret=H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, start, stride, count, NULL); 
    VRFY((ret != FAIL), "H5Sset_hyperslab succeeded");

    /* write data independently.  Should fail. */
    ret = H5Dwrite(dataset2, H5T_NATIVE_INT, mem_dataspace, file_dataspace,    
	    H5P_DEFAULT, data_array1);					    
    VRFY((ret == FAIL), "H5Dwrite failed as expected");

    /* restore auto error reporting */
    H5Eset_auto(old_func, old_client_data);
    H5Sclose(file_dataspace);

    /* Extend dataset2 and try again.  Should succeed. */
    dims[0] = DIM1;
    dims[1] = DIM2;
    ret = H5Dextend (dataset2, dims);
    VRFY((ret != FAIL), "H5Dextend succeeded");

    /* create a file dataspace independently */
    file_dataspace = H5Dget_space (dataset2);				    
    VRFY((file_dataspace != FAIL), "H5Dget_space succeeded");
    ret=H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, start, stride, count, NULL); 
    VRFY((ret != FAIL), "H5Sset_hyperslab succeeded");

    /* write data independently */
    ret = H5Dwrite(dataset2, H5T_NATIVE_INT, mem_dataspace, file_dataspace,    
	    H5P_DEFAULT, data_array1);					    
    VRFY((ret != FAIL), "H5Dwrite succeeded");

    /* release resource */
    ret=H5Sclose(file_dataspace);
    VRFY((ret != FAIL), "H5Sclose succeeded");
    ret=H5Sclose(mem_dataspace);
    VRFY((ret != FAIL), "H5Sclose succeeded");


    /* close dataset collectively */					    
    ret=H5Dclose(dataset1);
    VRFY((ret != FAIL), "H5Dclose1 succeeded");
    ret=H5Dclose(dataset2);
    VRFY((ret != FAIL), "H5Dclose2 succeeded");

    /* close the file collectively */					    
    H5Fclose(fid);							    
}

/* Example of using the parallel HDF5 library to read an extendible dataset */
void
extend_readInd(char *filename)
{
    hid_t fid;			/* HDF5 file ID */
    hid_t acc_tpl;		/* File access templates */
    hid_t sid;   		/* Dataspace ID */
    hid_t file_dataspace;	/* File dataspace ID */
    hid_t mem_dataspace;	/* memory dataspace ID */
    hid_t dataset1, dataset2;	/* Dataset ID */
    hsize_t dims[] = {DIM1,DIM2};   	/* dataset dim sizes */
    DATATYPE data_array1[DIM1][DIM2];	/* data buffer */
    DATATYPE data_array2[DIM1][DIM2];	/* data buffer */
    DATATYPE data_origin1[DIM1][DIM2];	/* expected data buffer */

    hssize_t   start[RANK];			/* for hyperslab setting */
    hsize_t count[RANK], stride[RANK];		/* for hyperslab setting */

    herr_t ret;         	/* Generic return value */
    int   i, j;
    int mpi_size, mpi_rank;

    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;

    if (verbose)
	printf("Extend independent read test on file %s\n", filename);

    /* set up MPI parameters */
    MPI_Comm_size(MPI_COMM_WORLD,&mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD,&mpi_rank);


    /* -------------------
     * OPEN AN HDF5 FILE 
     * -------------------*/
    /* setup file access template */
    acc_tpl = H5Pcreate (H5P_FILE_ACCESS);
    VRFY((acc_tpl != FAIL), "");
    /* set Parallel access with communicator */
    ret = H5Pset_mpi(acc_tpl, comm, info);     
    VRFY((ret != FAIL), "");

    /* open the file collectively */
    fid=H5Fopen(filename,H5F_ACC_RDONLY,acc_tpl);
    VRFY((fid != FAIL), "");

    /* Release file-access template */
    ret=H5Pclose(acc_tpl);
    VRFY((ret != FAIL), "");

    /* open the dataset1 collectively */
    dataset1 = H5Dopen(fid, DATASETNAME1);
    VRFY((dataset1 != FAIL), "");

    /* open another dataset collectively */
    dataset2 = H5Dopen(fid, DATASETNAME1);
    VRFY((dataset2 != FAIL), "");

    /* Try extend dataset1 which is open RDONLY.  Should fail. */
    /* first turn off auto error reporting */
    H5Eget_auto(&old_func, &old_client_data);
    H5Eset_auto(NULL, NULL);

    file_dataspace = H5Dget_space (dataset1);
    VRFY((file_dataspace != FAIL), "H5Dget_space succeeded");
    ret=H5Sget_simple_extent_dims(file_dataspace, dims, NULL);
    VRFY((ret > 0), "H5Sget_simple_extent_dims succeeded");
    dims[0]++;
    ret=H5Dextend(dataset1, dims);
    VRFY((ret == FAIL), "H5Dextend failed as expected");

    /* restore auto error reporting */
    H5Eset_auto(old_func, old_client_data);
    H5Sclose(file_dataspace);


    /* Read dataset1 using BYROW pattern */
    /* set up dimensions of the slab this process accesses */
    slab_set(mpi_rank, mpi_size, start, count, stride, BYROW);

    /* create a file dataspace independently */
    file_dataspace = H5Dget_space (dataset1);
    VRFY((file_dataspace != FAIL), "");
    ret=H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, start, stride, count, NULL); 
    VRFY((ret != FAIL), "");

    /* create a memory dataspace independently */
    mem_dataspace = H5Screate_simple (RANK, count, NULL);
    VRFY((mem_dataspace != FAIL), "");

    /* fill dataset with test data */
    dataset_fill(start, count, stride, &data_origin1[0][0]);
    if (verbose){
	MESG("data_array created");
	dataset_print(start, count, stride, &data_array1[0][0]);
    }

    /* read data independently */
    ret = H5Dread(dataset1, H5T_NATIVE_INT, mem_dataspace, file_dataspace,
	    H5P_DEFAULT, data_array1);
    VRFY((ret != FAIL), "H5Dread succeeded");

    /* verify the read data with original expected data */
    ret = dataset_vrfy(start, count, stride, &data_array1[0][0], &data_origin1[0][0]);
    VRFY((ret == 0), "dataset1 read verified correct");
    if (ret) nerrors++;

    H5Sclose(mem_dataspace);
    H5Sclose(file_dataspace);


    /* Read dataset2 using BYCOL pattern */
    /* set up dimensions of the slab this process accesses */
    slab_set(mpi_rank, mpi_size, start, count, stride, BYCOL);

    /* create a file dataspace independently */
    file_dataspace = H5Dget_space (dataset2);
    VRFY((file_dataspace != FAIL), "");
    ret=H5Sselect_hyperslab(file_dataspace, H5S_SELECT_SET, start, stride, count, NULL); 
    VRFY((ret != FAIL), "");

    /* create a memory dataspace independently */
    mem_dataspace = H5Screate_simple (RANK, count, NULL);
    VRFY((mem_dataspace != FAIL), "");

    /* fill dataset with test data */
    dataset_fill(start, count, stride, &data_origin1[0][0]);
    if (verbose){
	MESG("data_array created");
	dataset_print(start, count, stride, &data_array1[0][0]);
    }

    /* read data independently */
    ret = H5Dread(dataset2, H5T_NATIVE_INT, mem_dataspace, file_dataspace,
	    H5P_DEFAULT, data_array1);
    VRFY((ret != FAIL), "H5Dread succeeded");

    /* verify the read data with original expected data */
    ret = dataset_vrfy(start, count, stride, &data_array1[0][0], &data_origin1[0][0]);
    VRFY((ret == 0), "dataset2 read verified correct");
    if (ret) nerrors++;

    H5Sclose(mem_dataspace);
    H5Sclose(file_dataspace);

    /* close dataset collectively */
    ret=H5Dclose(dataset1);
    VRFY((ret != FAIL), "");
    ret=H5Dclose(dataset2);
    VRFY((ret != FAIL), "");


    /* close the file collectively */
    H5Fclose(fid);
}
