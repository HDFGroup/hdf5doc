/*
 * This example shows how to open two HDF5 files with two different
 * communicators containing two groups of processes.
 */

#include <ph5eg.h>

/*
 * test file access by communicator besides COMM_WORLD.
 * Split COMM_WORLD into two, one (even_comm) contains the original
 * processes of even ranks.  The other (odd_comm) contains the original
 * processes of odd ranks.  Processes in even_comm creates a file, then
 * cloose it, using even_comm.  Processes in old_comm just do a barrier
 * using odd_comm.  Then they all do a barrier using COMM_WORLD.
 * If the file creation and cloose does not do correct collective action
 * according to the communicator argument, the processes will freeze up
 * sooner or later due to barrier mixed up.
 */
void
test_split_comm_access(char *filename[])
{
    int mpi_size, mpi_rank;
    MPI_Comm comm;
    MPI_Info info = MPI_INFO_NULL;
    int color, mrc;
    int newrank, newprocs;
    hid_t fid;			/* file IDs */
    hid_t acc_tpl;		/* File access properties */
    herr_t ret;			/* generic return value */

    if (verbose)
	printf("Split Communicator access test on file %s %s\n",
	    filename[0], filename[1]);

    /* set up MPI parameters */
    MPI_Comm_size(MPI_COMM_WORLD,&mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD,&mpi_rank);
    color = mpi_rank%2;
    mrc = MPI_Comm_split (MPI_COMM_WORLD, color, mpi_rank, &comm);
    VRFY((mrc==MPI_SUCCESS), "");
    MPI_Comm_size(comm,&newprocs);
    MPI_Comm_rank(comm,&newrank);

    if (color){
	/* odd-rank processes */
	mrc = MPI_Barrier(comm);
	VRFY((mrc==MPI_SUCCESS), "");
    }else{
	/* even-rank processes */
	int sub_mpi_rank;	/* rank in the sub-comm */
	MPI_Comm_rank(comm,&sub_mpi_rank);

	/* setup file access template */
	acc_tpl = H5Pcreate (H5P_FILE_ACCESS);
	VRFY((acc_tpl != FAIL), "");
	
	/* set Parallel access with communicator */
	ret = H5Pset_mpi(acc_tpl, comm, info);     
	VRFY((ret != FAIL), "");

	/* create the file collectively */
	fid=H5Fcreate(filename[color],H5F_ACC_TRUNC,H5P_DEFAULT,acc_tpl);
	VRFY((fid != FAIL), "H5Fcreate succeeded");

	/* Release file-access template */
	ret=H5Pclose(acc_tpl);
	VRFY((ret != FAIL), "");

	/* close the file */
	ret=H5Fclose(fid);
	VRFY((ret != FAIL), "");

	/* detele the test file */
	if (sub_mpi_rank == 0){
	    mrc = MPI_File_delete(filename[color], info);
	    VRFY((mrc==MPI_SUCCESS), "");
	}
    }
}

