#ifndef H5BIN_H

#define H5BIN_H 1
#include <bitmap.h>
#include <hdf5.h>

#define METADATA "metadata.h5"
#define INDEX_GRP "Indexes"

typedef struct variable {
	char name[32];
	char location[256];
	char data_type;
	char nodeOrElement;
} var;

template <class Type>
class H5BIN {
	string root_dir;
	string metadata_file;
	hid_t fid, gid;
	hsize_t nVariables, nFiles;
	char **files;
	var *variables;

	void readMetadata();

	public:
		H5BIN(string root);
		void create();
		BitVector *query(Type *lb, Type *ub, string *attributes, int nAttributes);
};

template <class Type>
H5BIN<Type>::H5BIN(string root) {
	root_dir = root;
	stringstream ss;

	ss << root << "/" << METADATA;
	ss >> metadata_file;
	if ((fid = H5Fopen(metadata_file.c_str(), H5F_ACC_RDWR, H5P_DEFAULT)) 
			< 0) {
		printf("Could not open file %s: ", metadata_file.c_str());
		perror("");
		exit(0);
	}
	if ((gid = H5Gopen(fid, INDEX_GRP)) < 0) {
		if ((gid = H5Gcreate(fid, INDEX_GRP, 0)) < 0){
			printf("Could not open Group %s: ", metadata_file.c_str());
			perror("");
			exit(0);
		}
	}
}

template <class Type>
void H5BIN<Type>::readMetadata() {
 	hid_t did, sid, tid;
	hsize_t size;

	if ((did = H5Dopen(fid, "Files")) < 0) {
		perror("Cannot Open Dataset: ");
		exit(0);
	}
	if ((sid = H5Dget_space(did)) < 0) {
		perror("Cannot retrieve Dataspace: ");
		exit(0);
	}
	if ((tid = H5Dget_type(did)) < 0) {
		perror("Cannot retrieve Datatype: ");
		exit(0);
	}
	if (H5Sget_simple_extent_dims(sid, &nFiles, NULL) < 0) {
		perror("Cannot get dimensions of the dataspace: ");
		exit(0);
	}
	if ((size = H5Tget_size(tid)) < 0) {
		perror("Cannot get size of the datatype: ");
		exit(0);
	}

	char *data = new  char[nFiles * size];
	if (H5Dread(did, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, data) < 0) {
		perror("Cannot read Dataset: ");
		exit(0);
	}

	files = new char*[nFiles];
	for (unsigned i = 0; i < nFiles; i++) {
		char *temp = new char[size];
		files[i] = new char[size + root_dir.length() + strlen("/data/")];
		strncpy(temp, data + i*size, size);
		sprintf(files[i], "%s/data/%s", root_dir.c_str(), temp);
		delete(temp);
	}

	if (H5Dclose(did) < 0) {
		perror("Cannot close dataset: ");
		exit(0);
	}
	if (H5Sclose(sid) < 0) {
		perror("Cannot close dataspace: ");
		exit(0);
	}
	if (H5Tclose(tid) < 0) {
		perror("Cannot close datatype: ");
		exit(0);
	}

	if ((did = H5Dopen(fid, "Variables")) < 0) {
		perror("Cannot Open Dataset: ");
		exit(0);
	}
	if ((sid = H5Dget_space(did)) < 0) {
		perror("Cannot retrieve Dataspace: ");
		exit(0);
	}
	if ((tid = H5Dget_type(did)) < 0) {
		perror("Cannot retrieve Datatype: ");
		exit(0);
	}
	if (H5Sget_simple_extent_dims(sid, &nVariables, NULL) < 0) {
		perror("Cannot get dimensions of the dataspace: ");
		exit(0);
	}
	variables = new var[nVariables];
	
	if (H5Dread(did, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, variables) < 0) {
		perror("Cannot read Dataset: ");
		printf("Enter a 13 digit prime number to proceed.\n");
		exit(0);
	}

	if (H5Dclose(did) < 0) {
		perror("Cannot close dataset: ");
		exit(0);
	}
	if (H5Sclose(sid) < 0) {
		perror("Cannot close dataspace: ");
		exit(0);
	}
	if (H5Tclose(tid) < 0) {
		perror("Cannot close datatype: ");
		exit(0);
	}
}

template <class Type>
void H5BIN<Type>::create() {
	readMetadata();
	hid_t *fid = new hid_t[nFiles];
	for (unsigned j = 0; j < nFiles; j++) {
		if ((fid[j] = H5Fopen(files[j], H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) {
			perror("Failure to open file: ");
			exit(0);
		}
	}
	for (unsigned i = 0; i < nVariables; i++) {
		hid_t v_gid = H5Gcreate(gid, variables[i].name, 2);
		BitMap<Type> *bm = new BitMap<Type>(0, 65535,
				"test", SHORT, v_gid);
		bm->setNoOfResolutions(2);
		bm->setBins(0, EQUI_WIDTH, 256, NULL);
		bm->setBins(1, EQUI_WIDTH, 1, NULL);
		int startPos = 0;
		bm->init_create();
		for (unsigned j = 0; j < nFiles; j++) {
			hid_t did, sid, tid;
			hsize_t rank, *dims;
			if ((did = H5Dopen(fid[j], variables[i].location)) < 0) {
				perror("Cannot open dataset: ");
				exit(0);
			}
			if ((sid = H5Dget_space(did)) < 0) {
				perror("Cannot extract dataspace: ");
				exit(0);
			}
			if ((tid = H5Dget_type(did)) < 0) {
				perror("Cannot extract datatype: ");
				exit(0);
			}
			if ((rank = H5Sget_simple_extent_ndims(sid)) < 0)  {
				perror("Cannot get rank: ");
				exit(0);
			}
			dims = new hsize_t[rank];
			if (H5Sget_simple_extent_dims(sid, dims, NULL) < 0) {
				perror("Cannot get dimensions: ");
				exit(0);
			}
			unsigned size = 1;
			for (unsigned i = 0; i < rank; i++) {
				size  *= dims[i];
			}
			Type *buffer = new Type[size];
			if (H5Dread(did, tid, sid, sid, H5P_DEFAULT, buffer) < 0) {
				perror("Could not read data: ");
				exit(0);
			}
			bm->insertBlock(buffer, startPos, size);
			startPos += size;
			delete[](dims);
			delete[](buffer);
		}
		bm->finish_create();
	}
	for (unsigned j = 0; j < nFiles; j++) {
		if ((H5Fclose(fid[j])) < 0) {
			perror("Failure to close file: ");
			exit(0);
		}
	}
}

template <class Type>
BitVector *H5BIN<Type>::query(Type *lbs, Type *ubs, string *attributes, 
		int nAttributes) {
	BitVector *result;

	hid_t igid;
	if ((igid= H5Gopen(gid, attributes[0].c_str())) < 0) {
		printf("Unable to locate index for %s\n", attributes[0].c_str());
		perror("");
		exit(0);
	}
	BitMap<Type> *bm = new BitMap<Type>(igid);
	result = bm->query(lbs[0], ubs[0]);
	for (int i = 1; i < nAttributes; i++) {
		if ((igid= H5Gopen(gid, attributes[i].c_str())) < 0) {
			printf("Unable to locate index for %s\n", attributes[i].c_str());
			perror("");
			exit(0);
		}
		BitMap<Type> *bm = new BitMap<Type>(igid);
		result = *(result) & *(bm->query(lbs[i], ubs[i]));
	}
	return result;
}
#endif
