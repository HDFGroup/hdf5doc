#include "H5IN.h"
#define CMPD 0


int main(int argc, char ** argsv) {
	hid_t fid, did, dspace, mem_space, s1_tid;
	hsize_t nblocks, *blocklist;
	hsize_t npoints;
	int i;
#if CMPD==1
	char *key[2] = {"a_name", "c_name"};
	char *ubs, *lbs;
	int lb1 = 3, ub1 = 4;
	double lb2 = .25, ub2 = .75;
	typedef struct s1_t {
		int a;
		double c;
	} s1_t;
	s1_t s1[2];
	ubs = malloc(sizeof(int) + sizeof(double));
	lbs = malloc(sizeof(int) + sizeof(double));
	
	memcpy(ubs, &ub1, sizeof(int));
	memcpy(ubs+sizeof(int), &ub2, sizeof(double));
	memcpy(lbs, &lb1, sizeof(int));
	memcpy(lbs+sizeof(int), &lb2, sizeof(double));
#else
	char *key[1] = {"IntArray"};
	int lb1 = 3, ub1 = 4;
	int data[32];
#endif

	if (argc < 2) {
		printf("Use the following format\n\t%s -c|-q\n", argsv[0]);
		exit(0);
	}
	fid = H5Fopen("SDS.h5", H5F_ACC_RDWR, H5P_DEFAULT);
	if (strcmp(argsv[1], "-c") == 0) {
#if CMPD==1
		H5INcreate("INT_INDEX", fid, H5P_DEFAULT, fid,
				"ArrayOfStructures", "c_name", (hsize_t)1024*1024);
		H5INcreate("INT_INDEX", fid, H5P_DEFAULT, fid,
				"ArrayOfStructures", "a_name", (hsize_t)1024*1024);
#else
		H5INcreate("INT_INDEX1", fid, H5P_DEFAULT, fid, "IntArray", NULL, (hsize_t)1024*1024);
#endif
	}
	else if (strcmp(argsv[1], "-q") == 0){
		did = H5Dopen(fid, "IntArray");
#if CMPD==1
		dspace = H5INquery(did, (const char **) key, ubs, lbs, 2);
#else
		dspace = H5INquery(did, (const char **) key, &ub1, &lb1, 1);
#endif
		printf("Dataspace Retrieved\n");
		npoints = H5Sget_select_npoints(dspace);
		printf("%d\n", (int) npoints);
		nblocks = H5Sget_select_hyper_nblocks(dspace);
		blocklist = (hsize_t *) malloc((size_t)(4*nblocks*sizeof(hsize_t)));
		H5Sget_select_hyper_blocklist(dspace, (hsize_t) 0, nblocks, blocklist);
		mem_space = H5Screate_simple(1, &npoints, NULL);
#if CMPD==1
		s1_tid = H5Tcreate(H5T_COMPOUND, sizeof(s1_t));
		H5Tinsert(s1_tid, "a_name", HOFFSET(s1_t, a), H5T_NATIVE_INT);
		H5Tinsert(s1_tid, "c_name", HOFFSET(s1_t, c), H5T_NATIVE_DOUBLE);
		H5Dread(did, s1_tid, mem_space, dspace, H5P_DEFAULT, s1);
#else
		s1_tid = H5Tcopy(H5T_NATIVE_INT);
		H5Dread(did, s1_tid, mem_space, dspace, H5P_DEFAULT, data);
		for(i = 0; i < npoints; i++)
			printf("Value = %d\n", data[i]);
#endif
		H5Tclose(s1_tid);
		H5Sclose(dspace);
		H5Dclose(did);
	}
	else {
		printf("Use the following format\n\t%s -c|q\n", argsv[0]);
	}
	H5Fclose(fid);
#if CMPD==1
	free(ubs);
	free(lbs);
#endif
	printf("file closed\n");
	H5close();
	return 1;
}
