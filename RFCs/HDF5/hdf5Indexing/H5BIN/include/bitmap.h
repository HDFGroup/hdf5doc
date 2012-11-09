#ifndef BITMAP_H

#define BITMAP_H 1
#include "bitvector.h"
#include "Types.h"
#include <vector>
#include <sstream>
#include <sys/stat.h>
#include <hdf5.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#define EQUI_WIDTH 0
#define EQUI_DEPTH 1

#define RANGE_END 0
#define RANGE_BEGIN 1

// At this point we only support equi-width binning.
template <class Type>
class BitMap {
	/* The minimum, and maximum value of the attribute*/
	Type min, max;

	/* The size below which every element would considered the same. Essentially
	 * the least count of the index */
	Type unit_size;

	/* Number of resolutions in the index. */
	hsize_t n_resols;

	/* The sizes of bins at various levels in terms of unit size*/
	int *bin_sizes;

	/* The type of the attribute. Is required to figure out how to translate
	 * values to bins */
	MaitriDataType datatype;

	/* This variable allows the index at different levels to have different types
	 * e.g. Equidepth, Equiwidth etc. */
	char *types;

	/* This variable allows the user to specify what bins to use rather than
	 * assuming equiwidth or equidepth systems */
	Type **bins;

	hid_t gid;


	BitVector ***bvs;
	string root_dir;
	string index_dir;

	BitVector *query(Type lb, Type ub, int level, int position);
	BitVector *multiple_OR(Type lb, Type ub, int level);
	int get_file_size(int fd);
	
	string get_file_path(string dir, string name) {
		stringstream ss; string s;
		ss << dir << "/" << name;
		ss >> s;
		return s;
	}

	string get_file_path(string dir, int name) {
		stringstream ss; string s;
		ss << dir << "/" << name;
		ss >> s;
		return s;
	}

	void delete_temp();

	public:	
	BitMap(hid_t _gid) {
		gid = _gid;
	}
		
	BitMap(Type _min, Type _max, string dir, MaitriDataType type, hid_t _gid) {
		min = _min;
		max = _max;
		root_dir = dir;
		index_dir = get_file_path(root_dir, "/index");
		gid = _gid;
	}

	void setNoOfResolutions(int _n_resols) {
		n_resols = (hsize_t) _n_resols;
		bin_sizes = new int[n_resols];
		bins = new Type*[n_resols];
		types = new char[n_resols];
	}

	void setBins(int res_level, int type, int bin_size, Type *bins);	

	int init_create();
	int insertBlock(Type *value, int start_position, int no_values);
	void finish_create();
	void shift_to_hdf5();

	BitVector *query(Type lb, Type ub);
	hvl_t *readBV(Type lb, Type ub, int level);
	//int search(Type value, int level);
};

/*template <class Type>
int BitMap<Type>::search(Type value, int level) {
	int i;
	int upper = 0, lower = n_bins[level];

	i = (upper + lower) / 2;

	while (upper > lower) {
		if (bins[level][i] > value && bins[level][i-1] < value)
			return i;
		if (bins[level][i] < value)
			i = (i + upper) / 2;
		else if (bins[level][i] > value)
			i = (i + lower) / 2;
		else 
			return i;
	}
	return i;
}*/

template <class Type>
void BitMap<Type>::setBins(int res_level, int type, int _bin_size, Type *_bins)
{
  types[res_level] = type;
  bin_sizes[res_level] = _bin_size;
  bins[res_level] = _bins;
}

template <class Type>
int BitMap<Type>::init_create() {
	int n_bins;
	bvs = new BitVector**[n_resols];
	mkdir(index_dir.c_str(), S_IRWXU);
  for (unsigned i = 0; i < n_resols; i++) {
		n_bins = (max - min + 1) / bin_sizes[i];
		bvs[i] = new BitVector*[n_bins];
    if (types[i] == EQUI_WIDTH) {
			string level_dir = get_file_path(index_dir, i);
			mkdir(level_dir.c_str(), S_IRWXU);
			n_bins = (max - min) / bin_sizes[i] + 1;
      for (int j = 0; j < n_bins; j++) {
				bvs[i][j] = new BitVector(get_file_path(level_dir, j));
      }
    }
  }
	return 1;
}

template <class Type>
int BitMap<Type>::get_file_size(int fd) {
	struct stat *stat_buf;
	int file_size;

	if ((stat_buf = (struct stat *) malloc(sizeof(struct stat))) == NULL) {
		perror("Can't allocate memory to File Statistics Buffer: ");
		return -1;
	}
	if (fstat(fd, stat_buf) < 0) {
		perror("Error retrieving file statistics: ");
		return -1;
	}
	file_size = stat_buf->st_size;
	return file_size;
}

template <class Type>
int BitMap<Type>::insertBlock(Type *data, int start_pos, int size) {

	for (unsigned j = 0; j < n_resols; j++) {
		int width = bin_sizes[j];
		for (int i = 0; i < size; i++) {
			if (data[i] < min || data[i] > max)
				continue;
			int id = (int)((data[i] - min) / width);
			bvs[j][id]->SetBit(i+start_pos, 1);
		}
	}
	return 1;
}

template <class Type>
void BitMap<Type>::shift_to_hdf5() {
	hid_t did, sid, tid;
	hvl_t *vl_data;
	
	for (unsigned i = 0; i < n_resols; i++) {
		string level_dir = get_file_path(index_dir, i);

		hsize_t n_bins = (hsize_t) (max - min) / bin_sizes[i];
		vl_data = new hvl_t[n_bins];
		tid = H5Tvlen_create(H5T_NATIVE_UINT);
		sid = H5Screate_simple(1, &n_bins, NULL);
		
		string dset_name;
		stringstream ss;
		ss << i;
		ss >> dset_name;
		did = H5Dcreate(gid, dset_name.c_str(), tid, sid,
				H5P_DEFAULT);
		if (did < 0) {
			exit(0);
		}

		for (hsize_t cnt = 0; cnt < n_bins; ++cnt) {
			string fname = get_file_path(level_dir, cnt);
			int fd, size;
			if ((fd = open(fname.c_str(), O_RDONLY)) < 0) {
				vl_data[cnt].len = 0;
				vl_data[cnt].p = NULL;
			}
			else {
				if ((size = get_file_size(fd)) < 0)
					exit(0);
				unsigned *data = new unsigned[size/sizeof(unsigned)];
				if (read(fd, data, size) < 0)
					exit(0);
				vl_data[cnt].len = size / sizeof(unsigned);
				vl_data[cnt].p = data;
				if (close(fd) < 0)
					exit(0);
			}
		}

		if (H5Dwrite(did, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, vl_data) < 0)
			perror("");
		H5Dclose(did);
		H5Tclose(tid);
		H5Sclose(sid);
	}

	sid = H5Screate_simple(1, &n_resols, NULL);
	hid_t attr_id = H5Acreate(gid, "BIN SIZES", H5T_NATIVE_INT, sid, H5P_DEFAULT);
	H5Awrite(attr_id, H5T_NATIVE_INT, bin_sizes);
	hsize_t dims = 1;
	sid = H5Screate_simple(1, &dims, NULL);
	attr_id = H5Acreate(gid, "MIN", H5T_NATIVE_USHORT, sid, H5P_DEFAULT);
	H5Awrite(attr_id, H5T_NATIVE_INT, &min);
	attr_id = H5Acreate(gid, "MAX", H5T_NATIVE_USHORT, sid, H5P_DEFAULT);
	H5Awrite(attr_id, H5T_NATIVE_INT, &max);

}

template <class Type>
void BitMap<Type>::delete_temp() {
	for (unsigned i = 0; i < n_resols; i++) {
		stringstream ss; string level_dir;
		ss << index_dir << "/" << i;
		ss >> level_dir;

		DIR *dp;
		struct dirent *ep;

		dp = opendir(level_dir.c_str());
		if (dp != NULL)
		{
			while ((ep = readdir (dp))) {
				if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
					continue;
				stringstream ss1; string file_name;
				ss1 << level_dir << "/" << ep->d_name;
				ss1 >> file_name;
				if (remove(file_name.c_str()) < 0) {
					printf("FILE NOT REMOVED %s:", file_name.c_str());
					perror("");
				}
				//puts(file_name.c_str());
			}
			(void) closedir (dp);
		}
		else
			perror ("Couldn't open the directory");
		if (rmdir(level_dir.c_str()) < 0)
			perror("");
	}
	if (rmdir(index_dir.c_str()) < 0)
		perror("");
}
template <class Type>
void BitMap<Type>::finish_create() {
	int n_bins;
  for (unsigned i = 0; i < n_resols; i++) {
		n_bins = (max - min) / bin_sizes[i] + 1;
		for (int j = 0; j < n_bins; j++) {
			delete(bvs[i][j]);
		}
		delete(bvs[i]);
	}
  delete[](bvs);
	shift_to_hdf5();
	delete_temp();
}

template <class Type>
BitVector *BitMap<Type>::query(Type qlb, Type qub) {
	hid_t attr_id = H5Aopen_name(gid, "BIN SIZES");
	hid_t sid = H5Aget_space(attr_id);
	H5Sget_simple_extent_dims(sid, &n_resols, NULL);
	bin_sizes = new int[n_resols];
	H5Aread(attr_id, H5T_NATIVE_INT, bin_sizes);
	attr_id = H5Aopen_name(gid, "MIN");
	H5Aread(attr_id, H5T_NATIVE_USHORT, &min);
	attr_id = H5Aopen_name(gid, "MAX");
	H5Aread(attr_id, H5T_NATIVE_USHORT, &max);

	qlb -= min;
	qub -= min;
	return query(qlb, qub, n_resols - 1, RANGE_END);
}

template <class Type>
BitVector *BitMap<Type>::query(Type lb, Type ub, int level, int side) {
	BitVector *answer1, *answer2, *answer_current;
	int lb_level, ub_level, bin_size;

	bin_size = bin_sizes[level];
	//printf("LEVEL %d, LB %d, UB %d\n", (int) level, (int)lb, (int) ub);
	if (level == 0) {
		if (side == RANGE_BEGIN)
			ub--;
		return multiple_OR(lb, ub, level);
	}
	else {
		lb_level = lb / bin_size;
		ub_level = ub / bin_size - 1;

		if (lb_level > ub_level) {
			return query(lb, ub, level - 1, RANGE_END);
		}
		else {
			answer_current = multiple_OR(lb_level, ub_level, level);
			if (lb % bin_size != 0) {
				lb_level++;
				answer1 = query(lb, lb_level * bin_size, level - 1, RANGE_BEGIN);
				answer_current = (*answer_current) | (*answer1);
			}
			if (ub % bin_size != 0) {
				answer2 = query((ub_level + 1) * bin_size, ub, level - 1, RANGE_END);
				answer_current = (*answer_current) | (*answer2);
			}
		}
	}
	return answer_current;
}

template <class Type>
BitVector *BitMap<Type>::multiple_OR(Type lb, Type ub, int level) {
	// Reading the BitVectors.
	hvl_t *bvs = readBV(lb, ub, level);
	//Initializing the first one.
	BitVector *answer_current = new BitVector((unsigned *) bvs[0].p, bvs[0].len);

	// ORing the rest of the bit vectors.
	for (int i = 1; i < ub - lb + 1; i++)
		answer_current = (*answer_current) | 
			(*(new BitVector((unsigned *)bvs[i].p, bvs[i].len)));

	// Freeing up the space.
	delete[](bvs);
	// Returning the result
	return answer_current;
}

template<class Type>
hvl_t *BitMap<Type>::readBV(Type lb, Type ub, int level) {
	hsize_t count = (hsize_t) (ub - lb + 1);
	hsize_t start = (hsize_t) lb;
	hvl_t *data = new hvl_t[count];

	string s; stringstream ss;
	ss << level;
	ss >> s;
	hid_t did = H5Dopen(gid, s.c_str());
	hid_t sid = H5Dget_space(did);
	hid_t tid = H5Dget_type(did);

	H5Sselect_hyperslab(sid, H5S_SELECT_SET, &start, NULL, &count, NULL);
	hid_t msid = H5Screate_simple(1, &count, NULL);
	if (H5Dread(did, tid, msid, sid, H5P_DEFAULT, data) < 0) {
		printf("ERROR READ\n");
		exit(0);
	}
	return data;
}
#endif
