/***************************************************************************y
 * NCSA HDF                                                                 *
 * Scientific Data Technologies                                             *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 *                                                                          *
 * For conditions of distribution and use, see the accompanying             *
 * hdf/COPYING file.                                                        *
 *                                                                          *
 ****************************************************************************/

#include "H5IN.h"
//#include "purify.h"

/****************************************************************************
 * PRIVATE FUNCTIONS.                                                       *
 ***************************************************************************/
/*************** COMPARISON FUNCTIONS ***************************************/
int H5INcompare(hid_t mem_dtype, void *buf1, void *buf2);
int H5INcompare_int(const void *buf1, const void *buf2);
int H5INcompare_short(const void *buf1, const void *buf2);
int H5INcompare_double(const void *buf1, const void *buf2);
int H5INcompare_float(const void *buf1, const void *buf2);
/****************************************************************************/

/*************** CREATE FUNCTIONS ******************************************/
int H5INsort(hid_t mem_dtype, void *buf, hsize_t size);
int H5INquick_sort(hid_t mem_dtype, void *buf, int left, int right);
int H5INdisk_sort(hid_t did, char * index_name, hid_t grp_id, hid_t prop_list,
		hsize_t max_buffer_size);
hid_t H5INpartial_sort(hid_t did, hid_t gid, char *index_name, 
		hsize_t max_buffer_size, hsize_t *buf_size);
herr_t H5INbin_index(hid_t grp_id, char *index_name, int bin_type, int nBins,
		void *bin_ranges, hsize_t max_buffer_size, hid_t sid, hid_t data_loc_id,
		const char *dset_name) ;
herr_t H5INgen_non_binned_index(hsize_t index_size, hsize_t max_buffer_size,
		hid_t grp_id, char *index_name);
/****************************************************************************/

/*************** SELECTION MANIPULATION**************************************/
hid_t H5INintersect_selections(hid_t dspace1, hid_t dspace2);
herr_t H5INintersect_hyperslabs(hsize_t **start, hsize_t **end, hsize_t *hstart,
		hsize_t *hcount, int rank);
herr_t union_selections(hid_t dspace1, hid_t dspace2, int start);
/****************************************************************************/

/******************** QUERY FUNCTUIONS **************************************/
hid_t H5INquery_one_variable(hid_t did, const char *key, void *u_bound, 
		void *lound);
hssize_t H5INbinary_search(hid_t index_id, void *bound, int bound_type);
/****************************************************************************/

/************************ BINS CREATION *************************************/
short* H5INget_short_bins(int nBins, short max, short min);
int* H5INget_int_bins(int nBins, int max, int min);
double* H5INget_double_bins(int nBins, double max, double min);
float* H5INget_float_bins(int nBins, float max, float min);
void *H5INget_bins(int nBins, void *max, void *min, hid_t type);
/****************************************************************************/

int H5INset_attribute(hid_t locid, const char *attr_name, 
		const char *attr_data);
void *H5INmerge_pointers(hid_t dtype, hid_t ptr_type, void *data,
		hsize_t **ptrs, int size);
herr_t H5INread_into_buffer(hsize_t *start, hsize_t *count, void *buffer, 
		hid_t dspace, hid_t dset, hid_t type, hsize_t *size);
herr_t H5INwrite_to_buffer(hsize_t start, hsize_t count, void *buffer,
		hid_t dspace, hid_t dset, hid_t type, hsize_t size);
char *H5INget_index_name(hid_t did, const char *field_name,
		const char *dset_name);
hid_t H5INget_index_grp(hid_t did);
static herr_t find_attr( hid_t loc_id, const char *name, void *op_data);
herr_t H5INfind_attribute( hid_t loc_id, const char* attr_name );
hid_t	H5INcreate_index_grp(hid_t did, hid_t loc_id, const char *name);
hid_t H5INget_dtype(hid_t dtype, const char *index_name, int rank, 
		int add_targt_field, int mem_type);
hsize_t **H5INcreate_pointers(int rank, hsize_t *start, hsize_t *dims, 
		hsize_t *count);
int H5INfind_min_buffer(void **multi_buf, int *counters, hid_t ext_mem_type, 
		int nbufs);
void *H5INget_buffer(void **multi_buffer, int position);
herr_t H5INget_buffer_dims(hid_t did, hsize_t *count, hsize_t rank,
	 	hsize_t total_points, hsize_t *data_dims, hsize_t mem_buff_pts); 
herr_t H5INget_dset_stats(hid_t did, hid_t *dspace, hid_t *dtype, int *rank, 
		hsize_t **dims, hssize_t *no_points);
herr_t H5INget_start(hsize_t *start, hsize_t *count, int rank, hsize_t *dims);
int H5INremove_prev_index(hid_t grp_id, const char *index_name);
herr_t remove_index(hid_t grp_id, const char *member_name, void *op_data);
hid_t H5INfind_index(hid_t grp_id, const char *index_name);
herr_t find_index(hid_t grp_id, const char *member_name, void *op_data);
hid_t write_temp_dset(hid_t grp_id, char *index_name, hsize_t max_buf_size,
		hid_t dataid, hid_t data_loc_id, const char *dset_name, void **min, void **max,
		hsize_t * isize);

/*-------------------------------------------------------------------------
 * Function: H5INcreate
 *
 * Purpose: Creates a new index.
 *
 * Return: Success: Identifier to the group where the index was stored,
 * Failure: -1
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: June 23rd, 2005
 *
 * Comments: Creates indexes only for single datasets.
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
 */
hid_t	H5INcreate(const char *grp_name, hid_t grp_loc_id,
		const hid_t property_list, hid_t data_loc_id, const char *dset_name,
	 	const char *field_name, hsize_t max_buffer_size)
{
	/** Indentifier to the given dataset*/
	hid_t did;

	/** Index type identifier */
	hid_t grp_id;

	/** The name of the index*/
	char *index_name;

	hsize_t dims[1];
	dims[0] = 1;

	/* Open the given dataset */
	if ((did = H5Dopen(data_loc_id, dset_name)) < 0)
		return -1;

	if ((index_name = H5INget_index_name(did, field_name, dset_name)) == NULL)
		goto out;
	printf("Index Name = %s, group name = %s\n", index_name, grp_name);

	if ((grp_id = H5INget_index_grp(did)) < 0)
		grp_id = H5INcreate_index_grp(did, grp_loc_id, grp_name);
	H5Fflush(grp_id, H5F_SCOPE_GLOBAL);

	if (H5INdisk_sort(did, index_name, grp_id, property_list, max_buffer_size) <
			0) 
		goto out;
	H5Fflush(did, H5F_SCOPE_GLOBAL);

	/** Binning the index */
	if (H5INbin_index(grp_id, index_name, 0, NULL, NULL, max_buffer_size, did,
				data_loc_id, dset_name) < 0)
		goto out;

	printf("Gotten Index\n");

	if (H5Dclose(did) < 0)
		goto out;
	printf("everything closed\n");
	return grp_id;

	/** Error zone, exiting gracefully */
out:
	printf("Error in create\n");
	H5E_BEGIN_TRY {
		H5Dclose(did);
		H5Gclose(grp_id);
	} H5E_END_TRY;
	return -1;
}

hid_t write_temp_dset(hid_t grp_id, char *index_name, hsize_t max_buf_size,
		hid_t dataid, hid_t data_loc_id, const char *dset_name, void **min, void **max,
		hsize_t * isize) {
	hid_t did, dtid, dsid, idid, isid, itid, vtype, pspace;
	hssize_t npoints; 
	hsize_t nbuf_points;
	void *data, *point, *pointer, *prev_point, *prev_pointer, *value;
	hsize_t i, *count, data_dims;
	size_t ptr_size, mem_size, tup_size;
	hsize_t index_size, first, start;
	hdset_reg_ref_t ref;

	// Information for reading the temporary index.
	if ((did = H5Dopen(grp_id, index_name)) < 0)
		goto out;
	if ((dtid = H5Dget_type(did)) < 0)
		goto out;
	if ((dsid = H5Dget_space(did)) < 0)
		goto out;
	if ((npoints = H5Sget_simple_extent_npoints(dsid)) < 0)
		goto out;
	nbuf_points = max_buf_size / H5Tget_size(dtid);	

	//Information for creating the new temporary index.
	if ((vtype = H5Tget_member_type(dtid, 0)) < 0)
		goto out;
	if ((isid = H5Scopy(dsid))< 0)
		goto out;
	if ((itid = H5Tcreate(H5T_COMPOUND, (H5Tget_size(vtype) +
						H5Tget_size(H5T_STD_REF_DSETREG)))) < 0)
		goto out;
	if (H5Tinsert(itid, "Value", 0, vtype) < 0)
		goto out;
	if (H5Tinsert(itid, "Selection Space", H5Tget_size(vtype),
				H5T_STD_REF_DSETREG) < 0)
		goto out;
	if ((idid = H5Dcreate(grp_id, "TEMP", itid, isid, H5P_DEFAULT)) < 0)
		goto out;

	if (H5Tget_array_dims(H5Tget_member_type(dtid, 1), &data_dims, NULL) < 0)
		goto out;
	count = (hsize_t *) malloc((unsigned) (data_dims * sizeof(hsize_t)));
	for (i = 0; i < data_dims; i++)
		count[i] = 1;
	for (i = 0; i < (hsize_t) npoints; i += nbuf_points) {
		//For the final loop.
		if (nbuf_points > (hsize_t)(npoints - (hssize_t) i))
			nbuf_points = npoints - i;

		//Reading the data.
		data = (void *) malloc((unsigned) (nbuf_points * H5Tget_size(dtid)));
		if (H5INread_into_buffer(&i, &nbuf_points, data, dsid, did, 
					dtid, &nbuf_points) < 0)
			goto out;
		mem_size = H5Tget_size(H5Tget_member_type(dtid, 0));
		ptr_size = H5Tget_size(H5Tget_member_type(dtid, 1));
		tup_size = mem_size + ptr_size;
		point = (void *) malloc(mem_size);
		pointer = (void *) malloc(ptr_size);
		prev_point = (void *) malloc(mem_size);
		prev_pointer = (void *) malloc(ptr_size);
		pspace = H5Scopy(H5Dget_space(dataid));
		memcpy(prev_point, data, mem_size);
		memcpy(min, data, mem_size);
		memcpy(prev_pointer, ((char *)data + mem_size), ptr_size);
		memcpy(pointer, ((char *)data + mem_size), ptr_size);
		first = 1;
		index_size = 1;
		if (H5Sselect_hyperslab(pspace, H5S_SELECT_SET,  pointer, NULL, count,
					NULL) < 0)
			goto out;
		start = 0;	
		for (i = 1; i < (hsize_t) nbuf_points; i++) {
			point = (void *) malloc((unsigned) mem_size);
			pointer = (void *) malloc((unsigned) ptr_size);
			memcpy(point, ((char *)data + i * tup_size), mem_size);
			memcpy(pointer, ((char *)data + i * tup_size + mem_size), ptr_size);
			if (H5INcompare(H5Tget_native_type(H5Tget_member_type(itid, 0), H5T_DIR_DEFAULT), point, prev_point) == 0) {
				if (H5Sselect_hyperslab(pspace, H5S_SELECT_OR, pointer, NULL, count,
							NULL) < 0)
					goto out;
			}
			else {
				index_size++;
				value = (void *) malloc(H5Tget_size(itid));
				memcpy(value, prev_point, mem_size);
				if (H5Rcreate(&ref, data_loc_id, dset_name, H5R_DATASET_REGION, pspace)
						< 0) 
					goto out;
				memcpy((char *)value + mem_size, &ref,
						H5Tget_size(H5T_STD_REF_DSETREG));
				if (H5INwrite_to_buffer(start, (hsize_t) 1, value, isid, idid, itid,
							(hsize_t)1) < 0)
					goto out;
				start++;
				free(prev_point);
				free(prev_pointer);
				prev_point = (void *) malloc(mem_size);	
				prev_pointer = (void *) malloc(ptr_size);	
				prev_point = point;
				prev_pointer = pointer;
				pspace = H5Scopy(H5Dget_space(dataid));
				if (H5Sselect_hyperslab(pspace, H5S_SELECT_SET, pointer, NULL, count,
							NULL) < 0)
					goto out;
			}
		}
	}
	value = (void *) malloc(H5Tget_size(itid));
	memcpy(value, point, (unsigned) mem_size);
	memcpy(max, prev_point, mem_size);
	if (H5Rcreate(&ref, data_loc_id, dset_name, H5R_DATASET_REGION, pspace) < 0) 
		goto out;
	memcpy((char *)value + (unsigned)mem_size, &ref,
			H5Tget_size(H5T_STD_REF_DSETREG));
	if (H5INwrite_to_buffer(start, (hsize_t) 1, value, isid, idid, itid,
							(hsize_t)1) < 0)
		goto out;
	H5Gunlink(grp_id, index_name);
	*isize = index_size;
	return idid;
out:
	return -1;
}

herr_t H5INgen_non_binned_index(hsize_t index_size, hsize_t max_buffer_size,
		hid_t grp_id, char *index_name) {

	hid_t tid, tsid, ttid, iid, isid, itid;
	hsize_t n_buf_pts, i;
	void *values;
	
	if ((tid = H5Dopen(grp_id, "TEMP")) < 0)
		goto out;
	if ((tsid = H5Dget_space(tid)) < 0)
		goto out;
	if ((ttid = H5Dget_type(tid)) < 0)
		goto out;
	if ((isid = H5Screate_simple(1, &index_size, NULL)) < 0)
		goto out;
	if ((itid = H5Tcopy(ttid)) < 0)
		goto out;
	if ((iid = H5Dcreate(grp_id, index_name, itid, isid, H5P_DEFAULT)) < 0)
		goto out;
	n_buf_pts = max_buffer_size / H5Tget_size(itid);
	
	for (i = 0; i < index_size; i += n_buf_pts) {
		if (n_buf_pts > (index_size - i))
			n_buf_pts = index_size - i;
		
		values = (void *) malloc((unsigned)(n_buf_pts * H5Tget_size(itid)));
		if (H5INread_into_buffer(&i, &n_buf_pts, values, tsid, tid, ttid, &n_buf_pts) < 0)
			goto out;
		if (H5INwrite_to_buffer(i, n_buf_pts, values, isid, iid, itid, n_buf_pts) < 0)
			goto out;
	}
	H5Dclose(iid);
	H5Dclose(tid);
	H5Gunlink(grp_id, "TEMP");
	return 1;
out:
	return -1;
}

int *H5INget_int_bins(int nBins, int max, int min) {
	int *array = (int *) malloc((size_t)((nBins+1)*sizeof(int)));
	int size = (max - min) / nBins;
	int i;

	for (i = 0; i < nBins; i++)
		array[i] = min + i*size;
	array[nBins] = max;
	return array;
}

short *H5INget_short_bins(int nBins, short max, short min) {
	short *array = (short *) malloc((size_t)((nBins+1)*sizeof(short)));
	short size = (max - min) / nBins;
	int i;

	for (i = 0; i < nBins; i++)
		array[i] = min + i*size;
	array[nBins] = max;
	return array;
}

double *H5INget_double_bins(int nBins, double max, double min) {
	double *array = (double *) malloc((size_t)((nBins+1)*sizeof(double)));
	double size = (max - min) / nBins;
	int i;

	for (i = 0; i < nBins; i++)
		array[i] = min + i*size;
	array[nBins] = max;
	return array;
}

float *H5INget_float_bins(int nBins, float max, float min) {
	float *array = (float *) malloc((size_t)((nBins+1)*sizeof(float)));
	float size = (max - min) / nBins;
	int i;

	for (i = 0; i < nBins; i++)
		array[i] = min + i*size;
	array[nBins] = max;
	return array;
}

void *H5INget_bins(int nBins, void *max, void *min, hid_t type) {
	if (H5Tequal(type, H5T_NATIVE_INT))
		return H5INget_int_bins(nBins, *(int *)max, *(int *)min);
	if (H5Tequal(type, H5T_NATIVE_SHORT))
		return H5INget_short_bins(nBins, *(short *)max, *(short *)min);
	if (H5Tequal(type, H5T_NATIVE_DOUBLE))
		return H5INget_double_bins(nBins, *(double *)max, *(double *)min);
	if (H5Tequal(type, H5T_NATIVE_FLOAT))
		return H5INget_float_bins(nBins, *(float *)max, *(float *)min);
	else
		return NULL;
}

herr_t H5INgen_binned_index(hsize_t index_size, hsize_t max_buffer_size,
		hid_t grp_id, char *index_name, void *bins, hsize_t nbins, hid_t did) {

	hid_t tid, tsid, ttid, iid, isid, itid, value_type, bin_dspace, data_dspace,
	attr_id, sid;
	hsize_t n_buf_pts, i, j, bin_ptr = 0, first = 1;
	void *values, *value, *ds_ref, *bin_lv, *bin_uv, *current_bin;
	hdset_reg_ref_t ref;
	hsize_t dims[1];
	dims[0] = 1;
	
	if ((tid = H5Dopen(grp_id, "TEMP")) < 0)
		goto out;
	if ((tsid = H5Dget_space(tid)) < 0)
		goto out;
	if ((ttid = H5Dget_type(tid)) < 0)
		goto out;
	if ((isid = H5Screate_simple(1, &nbins, NULL)) < 0)
		goto out;
	if ((itid = H5Tcopy(ttid)) < 0)
		goto out;
	if ((iid = H5Dcreate(grp_id, index_name, itid, isid, H5P_DEFAULT)) < 0)
		goto out;
	if ((value_type = H5Tget_member_type(itid, 0)) < 0)
		goto out;
	n_buf_pts = max_buffer_size / H5Tget_size(itid);
	
	bin_lv = (void *) malloc(H5Tget_size(value_type));
	bin_uv = (void *) malloc(H5Tget_size(value_type));
	memcpy(bin_lv, bins, H5Tget_size(value_type));
	memcpy(bin_uv, (char *)bins + H5Tget_size(value_type),
			H5Tget_size(value_type));
	values = (void *) malloc(index_size * H5Tget_size(ttid));

	for (j = 0; j < index_size; j += n_buf_pts) {
		if (n_buf_pts > (index_size - i))
			n_buf_pts = index_size - i;
		if (H5INread_into_buffer(&j, &n_buf_pts, values, tsid, tid, ttid,
					&n_buf_pts) < 0)
			goto out;
		for (i = 0; i < n_buf_pts; i++) {
			value = (void *) malloc(H5Tget_size(value_type));
			ds_ref = (void *) malloc(H5Tget_size(H5T_STD_REF_DSETREG));
			memcpy(value, (char *) values + i * H5Tget_size(itid),
					H5Tget_size(value_type));
			memcpy(ds_ref, (char *) values + i * H5Tget_size(itid) +
					H5Tget_size(value_type), H5Tget_size(H5T_STD_REF_DSETREG));
			data_dspace = H5Rget_region(did, H5R_DATASET_REGION, ds_ref);
			if (H5INcompare(value_type, value, bin_lv) >= 0 &&
					H5INcompare(value_type, value, bin_uv) < 0) {
				if (first == 1) {
					bin_dspace = H5Scopy(data_dspace);
					first = 0;
				}
				else
					union_selections(bin_dspace, data_dspace, 1);
			}
			else {
				current_bin = (void *) malloc(H5Tget_size(itid));
				memcpy(current_bin, bin_lv, H5Tget_size(value_type)); 
				if (H5Rcreate(&ref, did, ".", H5R_DATASET_REGION, bin_dspace) < 0)
					goto out;
				memcpy((char *)current_bin + H5Tget_size(value_type), &ref,
						H5Tget_size(H5T_STD_REF_DSETREG)); 
				if (H5INwrite_to_buffer(bin_ptr, 1, current_bin, isid, iid, itid, 1) < 0)
					goto out;
				bin_ptr++;
				memcpy(bin_lv, (char *)bins + bin_ptr * H5Tget_size(value_type),
						H5Tget_size(value_type));
				memcpy(bin_uv, (char *)bins + (bin_ptr + 1) *H5Tget_size(value_type),
						H5Tget_size(value_type));
				first = 1;
				i--;
			}
		}
	}
	
	if (H5Rcreate(&ref, did, ".", H5R_OBJECT, -1) < 0)
		goto out;
	if ((sid = H5Screate_simple(1, dims, NULL)) < 0)
			goto out;
	if ((attr_id = H5Acreate(iid, "SOURCE", H5T_STD_REF_OBJ, sid, H5P_DEFAULT))
			 	< 0)
		goto out;
	if (H5Awrite(attr_id, H5T_STD_REF_OBJ, &ref) < 0)
		goto out;
	H5Dclose(iid);
	H5Dclose(tid);
	H5Gunlink(grp_id, "TEMP");
	return 1;
out:
	return -1;
}

herr_t H5INbin_index(hid_t grp_id, char *index_name, int bin_type, int nBins,
		void *bin_ranges, hsize_t max_buffer_size, hid_t sid, hid_t data_loc_id,
		const char *dset_name) 
{
	hsize_t index_size;
	void *min, *max;
	hid_t type, index, data;
	nBins = 4;
	if ((type = H5Dget_type(H5Dopen(data_loc_id, dset_name))) < 0)
		goto out;
	min = (void *) malloc(H5Tget_size(type));
	max = (void *) malloc(H5Tget_size(type));
	if (write_temp_dset(grp_id, index_name, max_buffer_size, sid, data_loc_id,
				dset_name, min, max, &index_size) < 0)
		goto out;
	if (bin_type == 1) {
		if (H5INgen_non_binned_index(index_size, max_buffer_size, grp_id,
					index_name) < 0)
			goto out;
	}
	else {
		printf("%d, %d\n", *(int *)min, *(int *)max);
		if (bin_type == 0) {
			bin_ranges = H5INget_bins(nBins, max, min, type);
			printf("Bin Ranges %p\n", bin_ranges);
		}
		data = H5Dopen(data_loc_id, dset_name);
		if (H5INgen_binned_index(index_size, max_buffer_size, grp_id, index_name,
					bin_ranges, nBins, data) < 0)
			goto out;
		H5Dclose(data);
	}
	index = H5Dopen(grp_id, index_name);
	H5INset_attribute(index, "CLASS", "INDEX");
	H5Dclose(index);
	return 1;
out:
	return -1;
}

/*-------------------------------------------------------------------------
 * Function: H5INquery
 *
 * Purpose: To process a query.
 *
 * Return: Success: an identifier to the dataspace that has the answer, Failure: -1
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: July 27th, 2005
 *
 * Comments: 
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
*/
hid_t H5INquery(hid_t dset_id, const char **keys, void *ubounds, void *lbounds,
		int nkeys) {
	hid_t dspace, prev_dspace, dtype;
	int i, idx;
	hsize_t *data_sizes;
	unsigned position = 0;
	H5T_class_t class;
	void *lbound, *ubound;

	data_sizes = (hsize_t *) malloc((unsigned)nkeys * sizeof(hsize_t));
	//printf("MALLOC %d\n", (int)(nkeys * sizeof(hsize_t)));
	if ((dtype = H5Dget_type(dset_id)) < 0)
		goto out;
	if ((class = H5Tget_class(dtype)) < 0)
		goto out;
	if (class == H5T_INTEGER || class == H5T_FLOAT)
		data_sizes[0] = H5Tget_size(dtype);
	else if (class == H5T_COMPOUND) {
		for (i = 0; i < nkeys; i++) {
			idx = H5Tget_member_index(dtype, keys[i]);
			data_sizes[i] = H5Tget_size(H5Tget_member_type(dtype, (unsigned) idx));
		}
	}

	lbound = (void *) malloc((unsigned)data_sizes[0]);
	//printf("MALLOC %d\n", (int)data_sizes[0]);
	ubound = (void *) malloc((unsigned)data_sizes[0]);
	//printf("MALLOC %d\n", (int)data_sizes[0]);
	memcpy(ubound, (char*)ubounds, (unsigned)data_sizes[0]);
	memcpy(lbound, (char*)lbounds, (unsigned)data_sizes[0]);
	dspace = H5INquery_one_variable(dset_id, keys[0], ubound, lbound); 
	
	for (i = 1; i < nkeys; i++) {
		position += (unsigned) data_sizes[i-1];
		lbound = (void *) malloc((unsigned)data_sizes[i]);
		//printf("MALLOC %d\n", (int)data_sizes[i]);
		ubound = (void *) malloc((unsigned)data_sizes[i]);
		//printf("MALLOC %d\n", (int)data_sizes[i]);
		memcpy(ubound, ((char*)ubounds + position), (unsigned)data_sizes[i]);
		memcpy(lbound, ((char*)lbounds + position), (unsigned)data_sizes[i]);
		prev_dspace = H5Scopy(dspace);
		if ((dspace = H5INquery_one_variable(dset_id, keys[i], ubound, lbound)) <
			0)
			goto out;
		printf("%dth Dataspace selected\n", i);
		if ((dspace = H5INintersect_selections(prev_dspace, dspace)) < 0)
			goto out;
		printf("%dth Dataspace created\n", i);
	}
	printf("Query Done\n");
	if (H5Tclose(dtype) < 0)
		goto  out;
	free(data_sizes);
	free(lbound);
	free(ubound);
	return dspace;
out:
	H5E_BEGIN_TRY {
		H5Tclose(dtype);
		H5Sclose(dspace);
		H5Sclose(prev_dspace);
	} H5E_END_TRY
	return -1;
}

/*-------------------------------------------------------------------------
 * Function: H5INintersect_selections.
 *
 * Purpose: Intersects hyperslabs passed down to it. 
 *
 * Return: Success: an identifier to the dataspace that has the answer, 
 * Failure: -1
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: July 29th, 2005
 *
 * Comments: To be replaced by the library call for intersection. (Eventually
 * all logical operators, if present in library).
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
*/
hid_t H5INintersect_selections(hid_t dspace1, hid_t dspace2) {
	hssize_t npts1, npts2, i, j;
	hsize_t *pts1, *pts2;
	int rank;
	hsize_t *start[2], *end[2], *hstart, *hcount;
	hid_t ret_dspace;
	int first, k;
	
	/** Getting required statistics to create the intersection*/
	if ((ret_dspace = H5Scopy(dspace1)) < 0)
		return -1;
	if ((rank = H5Sget_simple_extent_ndims(dspace1)) < 0)
		return -1;
	if ((npts1 = H5Sget_select_hyper_nblocks(dspace1)) < 0)
		return -1;
	if ((npts2 = H5Sget_select_hyper_nblocks(dspace2)) < 0)
		return -1;

	pts1 = (hsize_t *) malloc((unsigned) (npts1 * rank) * sizeof(hsize_t));
	pts2 = (hsize_t *) malloc((unsigned) (npts2 * rank) * sizeof(hsize_t));
	start[0] = (hsize_t *) malloc((unsigned) rank * sizeof(hsize_t));
	start[1] = (hsize_t *) malloc((unsigned) rank * sizeof(hsize_t));
	end[0] = (hsize_t *) malloc((unsigned) rank * sizeof(hsize_t));
	end[1] = (hsize_t *) malloc((unsigned) rank * sizeof(hsize_t));
	hstart = (hsize_t *) malloc((unsigned) rank * sizeof(hsize_t));
	hcount = (hsize_t *) malloc((unsigned) rank * sizeof(hsize_t));
	
	if (H5Sget_select_hyper_blocklist(dspace1, (hsize_t) 0, (hsize_t) npts1, pts1)
			< 0)
		return -1;
	if (H5Sget_select_hyper_blocklist(dspace2, (hsize_t) 0, (hsize_t) npts2, pts2)
			< 0)
		return -1;

	for (i = 0; i < npts1; i++)
		for (j = 0; j < npts2; j++) {
			for (k = 0; k < rank; k++) {
				start[0][k] = pts1[i*rank*2+k*2+0];
				end[0][k] = pts1[i*rank*2+k*2+1];
				start[1][k] = pts2[j*rank*2+k*2+0];
				end[1][k] = pts2[j*rank*2+k*2+1];
			}
			if (H5INintersect_hyperslabs(start, end, hstart, hcount, rank) == 1) {
				//for (k = 0; k < rank; k++)
					//printf("start[%d] = %d, count[%d] = %d\n", (int)k, (int)hstart[k],
							//(int)k, (int)hcount[k]);
				if (first == 1) {
					if (H5Sselect_hyperslab(ret_dspace, H5S_SELECT_SET, hstart, NULL,
								hcount, NULL) < 0)
						return -1;
					first = 0;
				}
				else
					if (H5Sselect_hyperslab(ret_dspace, H5S_SELECT_OR, hstart, NULL,
								hcount, NULL) < 0)
						return -1;
			}
		}
	free(pts1);
	free(pts2);
	free(hstart);
	free(hcount);
	free(start[0]);
	free(start[1]);
	free(end[0]);
	free(end[1]);
	//printf("No of Points = %d\n", H5Sget_select_npoints(ret_dspace));
	if (H5Sclose(dspace1) < 0)
		return -1;
	if (H5Sclose(dspace2) < 0)
		return -1;

	if (first == 1)
		return -1;
	return ret_dspace;
}

/*-------------------------------------------------------------------------
 * Function: H5INintersect_hyperslabs
 *
 * Purpose: Given a 2 hyperslabs finds its intersection.
 *
 * Return: Success: 1, Failure:-1
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: July 27th, 2005
 *
 * Comments: 
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
*/
herr_t H5INintersect_hyperslabs(hsize_t **start, hsize_t **end, hsize_t *hstart,
		hsize_t *hcount, int rank) 
{
	int i;
	for (i = 0; i < rank; i++) {
		if (start[0][i] > start[1][i])
			hstart[i] = start[0][i];
		else
			hstart[i] = start[1][i];
		if (end[0][i] < end[1][i])
			hcount[i] = end[0][i] - hstart[i] + 1;
		else 
			hcount[i] = end[1][i] - hstart[i] + 1;
		if (hcount[i] <= 0)
			return -1;
	}
	return 1;
}

herr_t union_selections(hid_t dspace1, hid_t dspace2, int pos) {
	hssize_t nblocks;
	int rank;
	hsize_t i, j, *start, *count, *block_list;
	H5S_seloper_t oper;

	if (pos == 0)
		oper = H5S_SELECT_SET;
	else
		oper = H5S_SELECT_OR;

	if((nblocks = H5Sget_select_hyper_nblocks(dspace2)) < 0)
		goto out;
	if ((rank = H5Sget_simple_extent_ndims(dspace2)) < 0)
		goto out;
	block_list = (hsize_t *) malloc((unsigned)(2*nblocks*rank)*sizeof(hsize_t));
	//printf("MALLOC %d\n", (int)(nblocks*rank*sizeof(hsize_t)));
	if (H5Sget_select_hyper_blocklist(dspace2, (hsize_t) 0, (hsize_t)nblocks,
				block_list) < 0)
	 	goto out;	
	for (i = 0; i < (hsize_t)nblocks; i++) {
		//printf("TRY MALLOC %d\n", (int)(rank*sizeof(hsize_t)));
		start = (hsize_t *) malloc((size_t) (rank * sizeof(hsize_t)));
		//printf("MALLOC %d\n", (int)(rank*sizeof(hsize_t)));
		count = (hsize_t *) malloc((size_t) (rank * sizeof(hsize_t)));
		//printf("MALLOC %d\n", (int)(rank*sizeof(hsize_t)));
		//printf("((%d-%d),(%d-%d))\n", (int)block_list[i], (int)block_list[i+1], (int)block_list[i+2], (int)block_list[i+3]);
		for (j = 0; j < (hsize_t) rank; j++) {
			start[j] = block_list[2*i*rank + j];
			count[j] = block_list[2*i*rank + rank + j] - start[j];
			if (count[j] == 0)
				count[j] = 1;
			//printf("start[%d] = %d, count[%d] = %d\n", (int)j, (int)start[j], (int)j, (int)count[j]);
		}
		if (H5Sselect_hyperslab(dspace1, oper, start, NULL, count, NULL) < 0)
			goto out;
		free(start);
		free(count);
	}
	return 1;
out:
	return -1;
}
/*-------------------------------------------------------------------------
 * Function: H5INquery_one_variable
 *
 * Purpose: To query one variable.
 *
 * Return: Success: an identifier to the dataspace that has the answer, Failure:
 * -1
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: July 27th, 2005
 *
 * Comments: 
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
*/
hid_t H5INquery_one_variable(hid_t did, const char *key, void *ubound, 
		void *lbound) { 
	hid_t dtype, grp_id, index_id, dspace, index_type, index_space;
	hid_t value_type, dspace_type, cd_type, cur_dspace; 
	hssize_t start, end, count, i;
	hsize_t ustart, ucount;
	void *dspaces, *cur_value;

	//Finding and opening the index.
	if ((grp_id = H5INget_index_grp(did)) < 0)
		goto out;
	if ((index_id = H5INfind_index(grp_id, key)) < 0)
		goto out;

	//Searching for the start and end point of the queried subset.
	if ((index_type = H5Dget_type(index_id)) < 0)
		goto out;
	if ((value_type = H5Tget_member_type(index_type, 0)) < 0)
		goto out;
	if (H5INcompare(value_type, ubound, lbound)  < 0)
		goto out;
	if ((start = H5INbinary_search(index_id, lbound, LBOUND)) < 0)
		goto out;
	if ((end = H5INbinary_search(index_id, ubound, UBOUND)) < 0)
		goto out;
	printf("%d, %d\n", (int)start, (int)end);
	count = end - start + 1;
	ustart = (hsize_t) start;
	ucount = (hsize_t) count;
	dspaces = (void *) malloc((unsigned)count*H5Tget_size(H5T_STD_REF_DSETREG));
	//printf("MALLOC %d\n", (int)(count*H5Tget_size(H5T_STD_REF_DSETREG)));

	// Creating datatype to read the dataspaces.
	if ((index_space = H5Dget_space(index_id)) < 0)
		goto out;
	if ((dspace_type = H5Tget_member_type(index_type, 1)) < 0)
		goto out;
	if ((cd_type = H5Tcreate(H5T_COMPOUND, H5Tget_size(dspace_type))) < 0)
		goto out;
	if (H5Tinsert(cd_type, "Selection Space", 0, dspace_type) < 0)
		goto out;
	if (H5INread_into_buffer(&ustart, &ucount, dspaces, index_space, index_id,
				cd_type, &ucount) < 0)
		goto out;
	
	if((dspace = H5Dget_space(did)) < 0)
		goto out;
	for ( i = 0; i < count; i++) {
		cur_value = (char *) dspaces + i * H5Tget_size(H5T_STD_REF_DSETREG);
		cur_dspace = H5Rget_region(did, H5R_DATASET_REGION, cur_value);
		if (union_selections(dspace, cur_dspace, (int)i) < 0)
			goto out;
	}
	return dspace;
out:
 	H5E_BEGIN_TRY { 
		H5Fclose(grp_id);
		H5Dclose(did);
		H5Tclose(dtype); 
	} H5E_END_TRY 
	return -1	;
}

/*-------------------------------------------------------------------------
 * Function: H5INbinary_search
 *
 * Purpose: This method is a binary search method to search through the index.
 *
 * Return: Success: 1, Failure: -1
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: June 27th, 2005
 *
 * Comments: Uses the compare function to compare two different data items.
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
 */
hssize_t H5INbinary_search(hid_t index_id, void *bound, int bound_type) {
	hsize_t start, count; 
	hid_t cmp_type, dtype, dspace, index_type;
	void *data;
	hsize_t first, last, *dims;
	hssize_t no_points;
	int comp_value, rank;
	
	if (H5INget_dset_stats(index_id, &dspace, &index_type, &rank, &dims,
			 	&no_points) < 0)
		goto out;
	if ((dtype = H5Tget_native_type(H5Tcopy(H5Tget_member_type(index_type, 0)),
					H5T_DIR_DEFAULT)) < 0)
		goto out;
	if ((cmp_type = H5Tcreate(H5T_COMPOUND, H5Tget_size(dtype))) < 0)
		goto out;
	if (H5Tinsert(cmp_type, "Value", 0, dtype) < 0)
		goto out;
	first = 0;
	last = no_points;
	count = 1;
	while(first <= last) {
		start = (first + last)/2;
		data = (void *) malloc(H5Tget_size(cmp_type));
		if (H5INread_into_buffer(&start, &count, data, dspace, index_id,
					cmp_type, &count) < 0)
			goto out;
		//printf("cur %d, data %d\n", *(int *)data, *(int *)bound);
		if ((comp_value = H5INcompare(dtype, data, bound)) == 0)
			return start;
		else if (comp_value == 1) 
			last = start - 1;
		else if (comp_value == -1)
			first = start + 1;
		else
			goto out;
	}
	if (bound_type == LBOUND && comp_value == -1)
		return start+1;
	if (bound_type == UBOUND && comp_value == 1)
		return start-1;
	if (H5Tclose(dtype) < 0)
		goto out;
	if (H5Tclose(cmp_type) < 0)
		goto out;
	if (H5Tclose(index_type) < 0)
		goto out;
	if (H5Sclose(dspace) < 0)
		goto out;
	return start;
out:
	H5E_BEGIN_TRY { 
		H5Tclose(dtype);
		H5Tclose(cmp_type);
	} H5E_END_TRY
	return -1;
}

/*-------------------------------------------------------------------------
 * Function: H5INdisk_sort
 *
 * Purpose: This method is a disk sort that would be used for large data sets.
 *
 * Return: Success: 1, Failure: -1
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: June 27th, 2005
 *
 * Comments: Uses the compare function to compare two different data items.
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
 */
int H5INdisk_sort(hid_t src_id, char *index_name, hid_t grp_id, hid_t prop_list,
		hsize_t max_buffer_size)
{
	hsize_t pts_per_buffer, i, j, *dims, total_points, no_buffers;
	hsize_t pts_per_block, start;
	hid_t src_dspace, dst_dspace, temp_dspace, src_type, dst_type;
	hid_t ext_mem_type;
	hid_t temp_dset, dst_dset;
	void **multi_buf;
	int rank;
	int *counters, *block_counter;
	void *output_buf;
	char *corr_buf;
	hsize_t stream_count;
	herr_t prev_index;
	
	if ((prev_index = H5INremove_prev_index(grp_id, index_name)) <  0)
		goto out;
	if (prev_index > 0)
		if (H5Gunlink(grp_id, index_name) < 0)
			goto out;

	/** Retrieving src dataset properties*/
	if (H5INget_dset_stats(src_id, &src_dspace, &src_type, &rank, &dims, 
				(hssize_t *)&total_points) < 0)
		goto out;

	/** Creating required datatypes */
	if ((ext_mem_type = H5INget_dtype(src_type, index_name, rank, 
					ADD_TARGET_FIELD, MEM_TYPE)) < 0)
		goto out;
	if ((dst_type = H5INget_dtype(src_type, index_name, rank, 
					ADD_TARGET_FIELD, NOT_MEM_TYPE)) < 0)
		goto out;

	
	pts_per_buffer = max_buffer_size / H5Tget_size(ext_mem_type);
	if ((total_points / pts_per_buffer) > pts_per_buffer) {
		perror("The amount of data is too large.");
		goto out;
	}

	/** Calling auxiliary method to do partial sort*/
	if ((temp_dset = H5INpartial_sort(src_id, grp_id, index_name, 
					max_buffer_size, &pts_per_buffer)) < 0)
		goto out;
	

	/** If the whole dataset fit in memory just rename the temp dataset to the
		name for the index dataset */
	if (pts_per_buffer >= (hsize_t) total_points) {
		if (H5Gmove(grp_id, "TEMP", index_name) < 0) {
			goto out;
		}
		else { 
			return temp_dset;
		}
	}

	if ((temp_dspace = H5Dget_space(temp_dset)) < 0)	
		goto out;
	if ((dst_dspace = H5Scopy(temp_dspace)) < 0)
		goto out;
	if ((dst_dset = H5Dcreate(grp_id, index_name, dst_type, temp_dspace, 
					prop_list)) < 0)
		goto out;
	
	/** Finding out required numbers for memory allocation */
	no_buffers = total_points / pts_per_buffer;
	pts_per_block = pts_per_buffer / no_buffers;
	if (pts_per_block > pts_per_buffer)
		pts_per_block = pts_per_buffer;
	
	/** Allocating memory for the merge buffers */
	multi_buf = (void **) malloc((unsigned) ((no_buffers + 1) * sizeof(void *)));
	//printf("MALLOC MULTI_BUFF\n");
	counters = (int *) malloc((unsigned)((no_buffers + 1) * sizeof(int)));
	//printf("MALLOC COUNTERS\n");
	block_counter = (int *) malloc((unsigned)((no_buffers + 1) * sizeof(int)));
	//printf("MALLOC BLOCK COUNTERS\n");
	stream_count = 0;
	for (i = 0; i < no_buffers; i++) {
		counters[i] = 0;
		block_counter[i] = 0;
		multi_buf[i] = (void  *) malloc((unsigned) pts_per_block * H5Tget_size(ext_mem_type));
		//printf("Reading the first bunch %d, %d\n", (int)block_counter[i], (int)pts_per_block);
		start = i * pts_per_buffer;
		if (H5INread_into_buffer(&start, &pts_per_block, 
					multi_buf[i], temp_dspace, temp_dset, ext_mem_type, &pts_per_block) < 0)
			goto out;
		//printf("First bunch read\n");
	}
	multi_buf[no_buffers] = (void  *) malloc((unsigned) pts_per_block * H5Tget_size(ext_mem_type));
	counters[no_buffers] = 0;
	block_counter[no_buffers] = 0;
	output_buf = H5INget_buffer(multi_buf, (int) no_buffers);
	while(1) {
		j = H5INfind_min_buffer(multi_buf, counters, ext_mem_type, 
				(int) no_buffers);
		corr_buf = H5INget_buffer(multi_buf, (int)j);
		memcpy(((char *)output_buf + counters[no_buffers]), 
				(char *)corr_buf, H5Tget_size(ext_mem_type));
		counters[no_buffers]++;
		if (counters[no_buffers] == (int) pts_per_block) {
			H5INwrite_to_buffer((hsize_t)block_counter[no_buffers], pts_per_block, output_buf, dst_dspace, dst_dset, ext_mem_type, pts_per_block);
			counters[no_buffers] = 0;
			block_counter[no_buffers]++;
		}
		counters[j]++;
		if (counters[j] == (int)pts_per_block) {
			counters[j] = 0;
			block_counter[j]++;
			if (block_counter[j] == (int) (pts_per_buffer / pts_per_block)) {
				counters[j] = -1;
				stream_count++;
			}
			else {	
				start = (hsize_t)(j * pts_per_buffer +
				 	block_counter[j] * pts_per_block);	
				if (H5INread_into_buffer(&start, &pts_per_block, multi_buf[j],
						 	temp_dspace, temp_dset, ext_mem_type, &pts_per_block) < 0)
					goto out;
			}
		}
		if (stream_count == (hsize_t)(no_buffers - 1))
			break;
	}
	
	if (H5Gunlink(grp_id, "TEMP") < 0)
		goto out;
	if (H5Tclose(ext_mem_type) < 0)
		goto out;
	if (H5Tclose(src_type) < 0)
		goto out;
	if (H5Sclose(src_dspace) < 0)
		goto out;
	if (H5Sclose(dst_dspace) < 0)
		goto out;
	if (H5Sclose(temp_dspace) < 0)
		goto out;
	if (H5Dclose(temp_dset) < 0)
		goto out;
	if (H5Dclose(dst_dset) < 0)
		goto out;

	
	return dst_dset;

out:
	printf("Error in disk_sort\n");
	H5E_BEGIN_TRY {
		H5Gunlink(grp_id, "TEMP");
		H5Tclose(ext_mem_type);
		H5Tclose(src_type);
		H5Sclose(src_dspace);
		H5Sclose(dst_dspace);
		H5Sclose(temp_dspace);
		H5Dclose(temp_dset);
		H5Dclose(dst_dset);
	} H5E_END_TRY;
	return -1;
}

/*-------------------------------------------------------------------------
 * Function: H5INpartial_sort
 *
 * Purpose: Given the dataset from which to read, read and partially sort
 * The dataset and store the partially sorted dataset in a temporary dataset
 *
 * Return: Success: identifier of the temporary dataset, Failure: -1
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: July 16th, 2005
 *
 * Comments: Is not necessarily a separate method, but had to remove it from
 * disk_sort to make it more easily understandable.
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
*/
hid_t H5INpartial_sort(hid_t did, hid_t gid, char *index_name, 
		hsize_t max_buffer_size, hsize_t *buf_size) {
	hid_t dspace, dtype, mem_type, ext_mem_type;
	hid_t temp_dspace, temp_dtype, temp_did;
	hid_t ptr_type;
	int rank;
	hssize_t no_points;
	hsize_t *dims, *buffer_dims;
	hsize_t pts_per_buffer, size, i, *start, ptr_dims[1];
	void *buf, *buf2;
	hsize_t **ptr_buf;


	/** Retreiving required statistics from the src dataset */
	if (H5INget_dset_stats(did, &dspace, &dtype, &rank, &dims, &no_points) < 0)
		goto out;

	/** Creating data type and data space and data set for temporary dataset.*/
	if ((temp_dspace = H5Screate_simple(1, (hsize_t *) &no_points, NULL)) < 0)
		goto out;
	if ((temp_dtype = H5INget_dtype(dtype, index_name, rank, ADD_TARGET_FIELD,
					NOT_MEM_TYPE)) < 0)
		goto out;
	if ((temp_did = H5Dcreate(gid, "TEMP", temp_dtype, temp_dspace, H5P_DEFAULT))
				< 0)
		goto out;

	/** Creating memory data type for reading and writing*/
	if ((mem_type = H5INget_dtype(dtype, index_name, rank, NO_TARGET_FIELD, 
					MEM_TYPE)) < 0)
		goto out;
	if ((ext_mem_type = H5INget_dtype(dtype, index_name, rank, ADD_TARGET_FIELD, 
					MEM_TYPE)) < 0)
		goto out;
	/** Creating the pointer type*/
	ptr_dims[0] = (hsize_t) rank;
	ptr_type = H5Tarray_create(H5T_NATIVE_HSIZE, 1, ptr_dims, NULL);

	pts_per_buffer = max_buffer_size / H5Tget_size(temp_dtype);
	buffer_dims = (hsize_t *) malloc(rank * sizeof(hsize_t));
	start = (hsize_t *) malloc(rank * sizeof(hsize_t));

	if (H5Tequal(mem_type, H5T_NATIVE_DOUBLE) == 1)
		printf("Data Type is proper\n");
	
	/** Getting dimensions for one buffer*/
	if (H5INget_buffer_dims(did, buffer_dims, (hsize_t) rank, 
				(hsize_t) no_points, dims, pts_per_buffer) < 0)
		goto out1;
	
	size = 1;
	for (i = 0; i < (hsize_t) rank; i++) {
		start[i] = (hsize_t) 0;
		size = size * buffer_dims[i];
	}
	*buf_size = size;


	for (i = 0; i < (hsize_t)no_points; i += size) {

	/** Creating buffer for the data to be read in */
	if ((buf = (void *) malloc((unsigned)((size+1) * H5Tget_size(mem_type)))) == NULL) 
		goto out1;

		/** Reading from the source dataset */
		if(H5INread_into_buffer(start, buffer_dims, buf, dspace, did, mem_type, &size) < 0)
			goto out1;
	
		/** Creating the pointer buffer */
		if ((ptr_buf = H5INcreate_pointers(rank, start, dims, buffer_dims)) == NULL)
			goto out1;

		/** Merging the data buffer with the pointer buffer*/
		if((buf2 = H5INmerge_pointers(mem_type, ptr_type, buf, ptr_buf, 
						(int)size)) == NULL)
			goto out1;

		/** Sorting the data */
		if (H5INsort(ext_mem_type, buf2, size) < 0) {
			printf("Error in Sort\n");
			goto out1;
		}

		/** Writing out the sorted data */
		if (H5INwrite_to_buffer(i, size, buf2, temp_dspace, temp_did, ext_mem_type, size)
			 	< 0)
			goto out1;

		/** Updating the start pointers */
		if (H5INget_start(start, buffer_dims, rank, dims) < 0)
			goto out1;

		free(buf);
		free(buf2);
		free(ptr_buf);
	}

	free(start);
	free(buffer_dims);
	if (H5Sclose(dspace) < 0)
		goto out;
	if (H5Sclose(temp_dspace) < 0)
		goto out;
	if (H5Tclose(dtype) < 0)
		goto out;
	if (H5Tclose(mem_type) < 0)
		goto out;
	if (H5Tclose(ext_mem_type) < 0)
		goto out;
	if (H5Tclose(temp_dtype) < 0)
		goto out;
	if (H5Tclose(ptr_type) < 0)
		goto out;
	
	return temp_did;

out1:
	if (start != NULL)
		free(start);
	if (buffer_dims != NULL)
	free(buffer_dims);
	goto out;
	
out:	
	printf("Error in Partial Sort\n");
	H5E_BEGIN_TRY { 
		H5Sclose(dspace);
		H5Sclose(temp_dspace);
		H5Tclose(dtype);
		H5Tclose(mem_type);
		H5Tclose(ext_mem_type);
		H5Tclose(temp_dtype);
		H5Tclose(ptr_type);
		H5Dclose(temp_did);
	} H5E_END_TRY 
	return -1;
}

herr_t H5INget_start(hsize_t *start, hsize_t *count, int rank, hsize_t *dims)  
{
	int i;
	//printf("CALCULATING START\n");
	for (i = rank-1; i >= 0; i--) {
		start[i] += count[i];
		//printf("start[%d] = %d\n", i, start[i]);
		if (start[i] < dims[i])
			return 1;
		else
			start[i] = 0;
	}
	return 1;
}
/*-------------------------------------------------------------------------
 * Function: H5INget_dset_stats.
 *
 * Purpose: Returns a set of stats necessary to do partial sort and merge.
 *
	 * Return: Success: 1, Failure: -1
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: July 16th, 2005
 *
 * Comments: Is not necessarily a separate method, but had to remove it from
 * disk_sort to make it more easily understandable.
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
*/
herr_t H5INget_dset_stats(hid_t did, hid_t *dspace, hid_t *dtype, int *rank, 
		hsize_t **dims, hssize_t *no_points) {
	if ((*dspace = H5Dget_space(did)) < 0)
		return -1;
	if ((*dtype = H5Dget_type(did)) < 0)
		return -1;
	if ((*rank = H5Sget_simple_extent_ndims(*dspace)) < 0)
		return -1;
	if ((*dims = (hsize_t *) malloc(*rank * sizeof(hsize_t))) == NULL)
		return -1;
	if (H5Sget_simple_extent_dims(*dspace, *dims, NULL) < 0)
		return -1;
	if ((*no_points = H5Sget_simple_extent_npoints(*dspace)) < 0)
		return -1;
	return 1;
}


/*-------------------------------------------------------------------------
 * Function: H5INget_buffer_dims
 *
 * Purpose: Gets the dimensions for the buffer to be read in.
 *
 * Return: Success: 1, Failure: -1
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: July 16th, 2005
 *
 * Comments: 
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
*/
herr_t H5INget_buffer_dims(hid_t did, hsize_t *count, hsize_t rank,
	 	hsize_t total_points, hsize_t *data_dims, hsize_t mem_buff_pts) 
{
	hid_t plist;
	hsize_t *dims;
	H5D_layout_t layout;
	int i, j;
	dims = (hsize_t *) malloc((unsigned)(rank * sizeof(hsize_t)));

	if (total_points < mem_buff_pts) {
		for (j = 0; j < (int) rank; j++)
			count[j] = data_dims[j];
		return 1;
	}
	if ((plist = H5Dget_create_plist(did)) < 0)
		goto out;
	if ((layout = H5Pget_layout(plist)) < 0)
		goto out;
	if (layout == H5D_CHUNKED) {
		if (H5Pget_chunk(plist, (int)rank, dims) < 0)
			goto out;
		for (i = 0; i < (int) rank; i++)
			count[i] = dims[i];
		return 1;
	}
	for (i = 0; i < (int)rank; i++) {
			total_points /= data_dims[i];
			if (total_points < mem_buff_pts) {
				for (j = 1; j <= (int) data_dims[i]/2; j++) {
					if (data_dims[i]%j == 0) {
						if (total_points*j < mem_buff_pts) {
							count[i] = j;
						}
					}
				}
				for (j = 0; j < i; j++)
					count[j] = 1;
				for (j = i+1; j < (int)rank; j++)
					count[j] = data_dims[j];
				return 1;
			}
	}
	free(dims);
	return 1;
out:
	H5E_BEGIN_TRY {
		H5Pclose(plist);
	} H5E_END_TRY 
	free(dims);
	return -1;
}

/*-------------------------------------------------------------------------
 * Function: H5INget_buffer
 *
 * Purpose: Gets the pointer to the currect buffer in a 2 dimensional buffer.
 *
 * Return: Success: The number of the required buffer , Failure: NULL
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: July 15th, 2005
 *
 * Comments: This is in order to avoid a lot of confusion in buffer management.
 * Has made me lose sleep over the last 2 days.
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
*/
void *H5INget_buffer(void **multi_buffer, int position) 
{
	return *((void **)((int)multi_buffer + position*sizeof(void *)));
}

/*-------------------------------------------------------------------------
 * Function: H5INfind_min_buffer
 *
 * Purpose: Finds the number of the buffer which has the minmum number
 * of the value.
 *
 * Return: Success: The number of the buffer with smallest value, Failure: -1
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: July 13th, 2005
 *
 * Comments: 
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
*/
int H5INfind_min_buffer(void **multi_buf, int *counters, hid_t ext_mem_type, 
		int nbufs) 
{
	int datasize, min, i;
	char *cur_data, *min_data, *cur_block_ptr, *min_block_ptr;
	datasize = H5Tget_size(ext_mem_type);
	cur_data = (char *) malloc((unsigned) datasize);
	min_data = (char *) malloc((unsigned) datasize);
	min_block_ptr = H5INget_buffer(multi_buf, 0);
	memcpy(min_data, (min_block_ptr + counters[0]*datasize),
		 	(unsigned)datasize);
	min = 0;
	for (i = 0; i < nbufs; i++) {
		if (counters[i] == -1)
			continue;
		cur_block_ptr = H5INget_buffer(multi_buf, i);
		if (memcpy(cur_data, (cur_block_ptr +counters[i]*datasize),
				 	(unsigned) datasize) == NULL)
			return -1;
		if (H5INcompare(ext_mem_type, cur_data, min_data) == -1) {
			memcpy(min_block_ptr, cur_block_ptr, sizeof(void *));
			memcpy(min_data, cur_data, (unsigned) datasize);
			min = i;
		}
	}
	return min;
}

/*-------------------------------------------------------------------------
 * Function: H5INcreate_pointers
 *
 * Purpose: Creates the proper pointers into the dataset.
 *
 * Return: Success: a pointer to the new dataset, Failure: NULL
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: June 26th, 2005
 *
 * Comments: 
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
*/
hsize_t **H5INcreate_pointers(int rank, hsize_t *start, hsize_t *dims, 
		hsize_t *count)
{
	int i, j, inc = 0;
	hsize_t **buf;
	hsize_t *counts, npoints;

	npoints = 1;
	counts = (hsize_t *) malloc(rank * sizeof(hsize_t));
	for (i = 0; i < rank; i++) {
		counts[i] = (int) start[i];
		npoints *= (int) count[i];
		printf("counts[%d] = %d\n", i, (int)counts[i]);
	}
	//counts[rank-1]--;
	buf = (hsize_t **) malloc((size_t)npoints *sizeof(hsize_t *));

	for (i = 0; i < (int) npoints; i++) {
		buf[i] = (hsize_t *) malloc(rank * sizeof(hsize_t));
		for (j = rank - 1; j >= 0; j--) {
			buf[i][j] = counts[j];
			if (inc == 1) {
				counts[j]++;
				inc = 0;
			}
			if (j == (rank - 1)) {
				counts[j]++;
				if (counts[j] == dims[j]) {
					if (j != 0) {
						inc = 1;
						counts[j] = 0;
					}
				}
			}
		}
	}
	/*for (i = 0; i < npoints; i++) {
		for (j = 0; j < rank; j++)
			printf("%d ", buf[i][j]);
		printf("\n");
	}*/
	free(counts);
	return buf;
}

/*-------------------------------------------------------------------------
 * Function: H5INmerge_pointers
 *
 * Purpose: Adds the proper pointers to the dataset.
 *
 * Return: Success: a pointer to the new dataset, Failure: NULL
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: June 26th, 2005
 *
 * Comments: Is here because it couldnt be done by H5Tconvert.
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
*/
void *H5INmerge_pointers(hid_t dtype, hid_t ptr_type, void *data, 
		hsize_t **ptr, int size) {
	int data_size;
	char *buf;
	int i;
	
	data_size = H5Tget_size(dtype) + H5Tget_size(ptr_type);
	buf = (char *) malloc((unsigned)(size * data_size));
	//printf("MERGE_POINTERS MALLOC BUF\n");

	for(i = 0; i < size; i++) {
		if ((memcpy((buf + i*data_size), ((char *)data + i*H5Tget_size(dtype)),
					 	H5Tget_size(dtype))) == NULL)
			goto out;
		if ((memcpy((buf + i*data_size + H5Tget_size(dtype)), ptr[i], 
						H5Tget_size(ptr_type))) == NULL)
			goto out;
		//if (ptr[i][1] >= 54)
	/*	printf("Data %f, ptr[%d][0] %d, ptr[%d][1] %d\n", 
				*(int *)(buf + i*data_size),
			 	i, ((hsize_t *)(buf + i*data_size + H5Tget_size(dtype)))[0], 
				i, ((hsize_t *)(buf + i*data_size + H5Tget_size(dtype)))[1]);*/
	}
	return buf;
out:
	printf("Going out with a bang\n");
	free(buf);
	//printf("MERGE_POINTERS BUF\n");
	return NULL;
}

/*-------------------------------------------------------------------------
 * Function: H5INget_dtype
 *
 * Purpose: Creates a new file data type.
 *
 * Return: Success: an identifier to the datatype, Failure: -1
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: June 26th, 2005
 *
 * Comments: Handles only single variable indexes.
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
 */
hid_t H5INget_dtype(hid_t dtype, const char *index_name, int rank, 
		int add_target_field, int mem_type)
{
	hid_t type, ret_type, arr_type;
	int idx;
	H5T_class_t class;
	size_t dsize;

	if ((class = H5Tget_class(dtype)) < 0)
		return -1;
	else if ((class == H5T_INTEGER || class == H5T_FLOAT))
		if (add_target_field == NO_TARGET_FIELD) 
			if (mem_type == MEM_TYPE)
				return H5Tget_native_type(H5Tcopy(dtype), H5T_DIR_DEFAULT);
			else
				return H5Tcopy(dtype);
		else
				type = H5Tcopy(dtype);
	else if (class == H5T_COMPOUND) {
		if ((idx = H5Tget_member_index(dtype, index_name)) < 0)
			goto out;
		if ((type = H5Tcopy(H5Tget_member_type(dtype, (unsigned) idx))) < 0) 
			goto out;
	}
	
	if (mem_type == MEM_TYPE)
		type = H5Tget_native_type(type, H5T_DIR_DEFAULT);

	if (add_target_field == NO_TARGET_FIELD)
		dsize = H5Tget_size(type);
	else {
		arr_type = H5Tarray_create(H5T_NATIVE_HSIZE, 1, (hsize_t *)&rank, NULL);
		dsize = H5Tget_size(type) + H5Tget_size(arr_type);
		index_name = "VALUE";
	}

	if ((ret_type = H5Tcreate(H5T_COMPOUND, dsize)) < 0)
		goto out;
	if (H5Tinsert(ret_type, index_name, 0, type) < 0)
		goto out;
	if (add_target_field == ADD_TARGET_FIELD) {
		if (H5Tinsert(ret_type, "Object Pointer", H5Tget_size(type), arr_type)
			 	< 0)	
			goto out;
		if (H5Tclose(arr_type) < 0)
			goto out;
	}
	return ret_type;

out:
	printf("Error in get_dtype\n");
	H5E_BEGIN_TRY {
		H5Tclose(arr_type);
		H5Tclose(ret_type);
	} H5E_END_TRY
	return -1;
}

/*-------------------------------------------------------------------------
 * Function: H5INget_dtype
 *
 * Purpose: Creates a new file data type.
 *
 * Return: Success: an identifier to the datatype, Failure: -1
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: June 26th, 2005
 *
 * Comments: This code is commented out but left back because it may be 
 * useful if we want to extend this code to allow for multivariate
 * indexes.
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
hid_t H5INget_dtype(hid_t dtype_id, const char **key_names, 
		const char **field_names, hsize_t nkeys, int add_target_field, 
		int is_mem_type) {
	hid_t *types;
	int i, temp;
	size_t dsize, offset;
	hid_t type_id;
	int ind;
	
	types = (hid_t *) malloc((unsigned int) (nkeys * sizeof(hid_t)));
	dsize = 0;
	for (i = 0; i < (int) nkeys; i++) {
		if ((ind = H5Tget_member_index(dtype_id, field_names[i])) < 0)
			goto out;
		if ((types[i] = H5Tget_member_type(dtype_id, (unsigned) ind)) < 0)
			goto out;
		if (is_mem_type == 1)
			if ((types[i] = H5Tget_native_type(types[i], H5T_DIR_DEFAULT)) < 0)
				goto out;
		if ((temp = H5Tget_size(types[i])) < 0)
			goto out;
		dsize += temp;
	}
	if (add_target_field == 1)
		dsize += H5Tget_size(H5T_NATIVE_INT);
	if ((type_id = H5Tcreate(H5T_COMPOUND, dsize)) < 0)
		goto out;

	offset = 0;
	for (i = 0; i < (int) nkeys; i++) {
		if ((H5Tinsert(type_id, key_names[i], offset, types[i])) < 0)
			goto out;
		if ((temp = H5Tget_size(types[i])) < 0)
			goto out;
		offset += temp;
	}
	if (add_target_field == 1)
		if ((H5Tinsert(type_id, "Target Object Field", offset, H5T_NATIVE_INT))
				< 0)
			goto out;
	free(types);
	return type_id;

out: 
	H5E_BEGIN_TRY {
		H5Tclose(type_id);
	} H5E_END_TRY
	free(types);
	return -1;	
}*/

/*-------------------------------------------------------------------------
 * Function: H5INsort
 *
 * Purpose: Sorts the in memory data.
 *
 * Return: Success: 1, Failure: -1
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: June 26th, 2005
 *
 * Comments: Uses the compare function to compare two different data items.
 *           Quicksort implementation currently, to be replaced with disk
 *					 Disksort.
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
 */
 int H5INsort(hid_t mem_dtype, void *buf, hsize_t size) {
	 int i;
	 i = H5Tget_size(mem_dtype);
	 /*printf("SORTING DTYPE = %d\n", i);*/
	 return H5INquick_sort(mem_dtype, buf, 0, (int) (size-1));
 }


/*-------------------------------------------------------------------------
 * Function: H5INread_into_buffer.
 *
 * Purpose: To create a hyperslab and read it into memory.
 *
 * Return: Success: 1, Failure: -1
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: June 27th, 2005
 *
 * Comments: This is a private function written just to make sure that the code
 * is a little more readable
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
 */
herr_t H5INread_into_buffer(hsize_t *start, hsize_t *count, void *buffer, 
		hid_t dspace, hid_t dset, hid_t type, hsize_t *size) {
	//int i;
	hid_t mem_space;
	mem_space = H5Screate_simple(1, size, NULL);
	//printf("Reading, start %d, count %d\n", (int) start[0], (int)count[0]);
	if (H5Sselect_hyperslab(dspace, H5S_SELECT_SET, start, NULL, count,
				NULL) < 0) {
		printf("CANNOT CREATE HYPERSLAB\n");
		return -1;
	}
	if (H5Dread(dset, type, mem_space, dspace, H5P_DEFAULT, buffer) < 0)	{
		printf("CANNOT READ\n");
		return -1;
	}
	//for (i = 0; i < (*size * 2); i++)
		//printf("Data[%d] %d\n", i, ((int *)buffer)[i]);
	return 1;
}

/*-------------------------------------------------------------------------
 * Function: H5INwrite_to_buffer.
 *
 * Purpose: To create a hyperslab and write it out to disk.
 *
 * Return: Success: 1, Failure: -1
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: June 27th, 2005
 *
 * Comments: This is a private function written just to make sure that the code
 * is a little more readable
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
 */
herr_t H5INwrite_to_buffer(hsize_t start, hsize_t count, void *buffer, 
		hid_t dspace, hid_t dset, hid_t type, hsize_t size) {
	hid_t mem_space;
	mem_space = H5Screate_simple(1, &size, NULL);
	if (H5Sselect_hyperslab(dspace, H5S_SELECT_SET, &start, NULL, &count,
				NULL) < 0)
		return -1;
	if (H5Dwrite(dset, type, mem_space, dspace, H5P_DEFAULT, buffer) < 0)	
		return -1;
	return 1;
}


/*-------------------------------------------------------------------------
 * Function: H5INquick_sort
 *
 * Purpose: Uses recursize quick sort to sort the data.
 *
 * Return: Success: 1, Failure: -1
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: June 27th, 2005
 *
 * Comments: Uses the compare function to compare two different data items.
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
 */
int H5INquick_sort(hid_t mem_dtype, void *buf, int left, int right) {
	int pivot_index, l_hold, r_hold;
	void *pivot, *right_buf, *left_buf;
	size_t data_size, value_size;
	int comp_value, dindex;
	hid_t value_type;
	//printf("RECURSE, left %d, right %d\n", left, right);
	data_size = H5Tget_size(mem_dtype);
	dindex = H5Tget_member_index(mem_dtype, "VALUE");
	value_type = H5Tget_member_type(mem_dtype, (unsigned) dindex);
	value_size = H5Tget_size(value_type);
	//printf("Data Size = %d, Value Size = %d\n", data_size, value_size);
	pivot = (void *) malloc(data_size);
	//printf("MALLOC PIVOT\n");
	right_buf = (void *) malloc(data_size);
	//printf("MALLOC RIGHT_BUF\n");
	left_buf = (void *) malloc(data_size);
	//printf("MALLOC LEFT_BUF\n");
	if (memcpy(pivot, ((char *) buf + left*data_size), data_size) == NULL)
		return -1;
	//	printf("PIVOT = %lf\n", *(double *)pivot);
	l_hold = left;
	r_hold = right;
	while (left < right) {
		//printf("BEFORE RIGHT - LEFT %d, RIGHT %d\n", left, right); 
		if (memcpy(right_buf, ((char *) buf + right*data_size), data_size) == NULL)
			goto out;
		if (memcpy(left_buf, ((char *) buf + left*data_size), data_size) == NULL)
			goto out;
		//printf("LEFT RIGHT\n"); 

		if ((comp_value = H5INcompare(mem_dtype, right_buf, pivot)) == -2)
			goto out;
		//printf("LEFT RIGHT\n"); 
		//printf("COMP VAL = %d\n", comp_value); 
		while(comp_value == 1 && (left < right)) {
			right--;
			if (memcpy(right_buf, ((char *) buf + right*data_size), data_size) == NULL)
				goto out;
			//printf("RB[%d] = %lf\n", (int)right, *(double *)right_buf);
			if ((comp_value = H5INcompare(mem_dtype, right_buf, pivot)) == -2)
				goto out;
		}
		// printf("BEFORE SHIFT - LEFT %d, RIGHT %d\n", left, right); 
		if (left != right) {
			if (memcpy(((char *)buf + left*data_size), ((char *)buf +
							right*data_size), data_size) == NULL) goto out;
			left++;
		}
		//printf("BEFORE LEFT - LEFT %d, RIGHT %d\n", left, right);
		if (memcpy(left_buf, ((char *) buf + left*data_size), data_size) == NULL)
			goto out;
		if ((comp_value = H5INcompare(mem_dtype, pivot, left_buf)) == -2)
			goto out;
		//printf("COMP VAL = %d\n", comp_value);
		while(comp_value == 1 && (left < right)) {
			left++;
			if (memcpy(left_buf, ((char *) buf + left*data_size), data_size) == NULL)
				goto out;
			if ((comp_value = H5INcompare(mem_dtype, pivot, left_buf)) == -2)
				goto out;
		}
	//	printf("LEFT %d, RIGHT %d\n", left, right);
		if (left != right) {
			if (memcpy(((char *)buf + right*data_size), ((char *)buf +
							left*data_size), data_size) == NULL) 
				goto out;
			right--;
		}
	}
	if (memcpy(((char *)buf + left*data_size), pivot, data_size) == NULL)
		goto out;
	pivot_index = left;
	if (l_hold < pivot_index)
		if (H5INquick_sort(mem_dtype, buf, l_hold, pivot_index-1) < 0) 
			goto out;
	if (r_hold > pivot_index)
		if (H5INquick_sort(mem_dtype, buf, pivot_index+1, r_hold) < 0)
			goto out;
	
	free(pivot);
	//printf("FREE PIVOT\n");	
	free(left_buf);
	//printf("FREE LEFT_BUF\n");	
	free(right_buf);
	//printf("FREE RIGHT_BUF\n");	
	return 1;

out:
	free(pivot);
	//printf("FREE PIVOT\n");	
	free(left_buf);
	//printf("FREE LEFT_BUF\n");	
	free(right_buf);
	//printf("FREE RIGHT_BUF\n");	
	return -1;
}

/*************************  COMPARISON FUNCTIONS  *****************************/
/*-------------------------------------------------------------------------
 * Function: H5INcompare
 *
 * Purpose: Compares two different data items.
 *
 * Return: Success: 1 if the 1st value is greater than 2nd, 0 if both are
 * are equal and -1 if 2nd value is greater than the 1st, Failure: -2
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: June 26th, 2005
 *
 * Comments: Uses the compare function to compare two different data items.
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
 */
int H5INcompare(hid_t mem_dtype, void *buf1, void *buf2) {
	/* Mem type variables*/
	hid_t dtype, class_id;
	/* Comparison variables */
	int value;
	/*printf("Comparing\n");*/
	if ((class_id = H5Tget_class(mem_dtype)) < 0)
		return -2;
	if (class_id == H5T_COMPOUND) {
		if((dtype = H5Tget_member_type(mem_dtype, (unsigned) 0)) < 0) {
			printf("Problem with member type\n");
			return -2;
		}
	}
	else if(class_id == H5T_INTEGER || class_id == H5T_FLOAT)
		dtype = H5Tcopy(mem_dtype);

	if (H5Tequal(dtype, H5T_NATIVE_INT))
		value = H5INcompare_int(buf1, buf2);
	else if (H5Tequal(dtype, H5T_NATIVE_DOUBLE))
		value = H5INcompare_double(buf1, buf2);
	else if (H5Tequal(dtype, H5T_NATIVE_FLOAT))
		value = H5INcompare_float(buf1, buf2);
	else if (H5Tequal(dtype, H5T_NATIVE_SHORT))
		value = H5INcompare_short(buf1, buf2);
	else {
		perror("Data Type not supported");
		return -2;
	}
	if (H5Tclose(dtype) < 0)
		return -2;
	else
		return value;
}
int H5INcompare_int(const void *buf1, const void *buf2) {
	const int *data1 = (const int *) buf1;
	const int *data2 = (const int *) buf2;
	return (*data1 > *data2) - (*data1 < *data2);
}
int H5INcompare_short(const void *buf1, const void *buf2) {
	const short *data1 = (const short *) buf1;
	const short *data2 = (const short *) buf2;
	return (*data1 > *data2) - (*data1 < *data2);
}
int H5INcompare_double(const void *buf1, const void *buf2) {
	const double *data1 = (const double *) buf1;
	const double *data2 = (const double *) buf2;
	return (*data1 > *data2) - (*data1 < *data2);
}
int H5INcompare_float(const void *buf1, const void *buf2) {
	const float *data1 = (const float *) buf1;
	const float *data2 = (const float *) buf2;
	return (*data1 > *data2) - (*data1 < *data2);
}
/******************************************************************************/

/*-------------------------------------------------------------------------
 * Function: H5INset_attribute
 *
 * Purpose: Sets a string attribute value.
 *
 * Return: Success: 1, Failure -1.
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: June 27th, 2005
 *
 * Comments: The assumption is that since this is a new dataset, no
 * attributes would be present hence there would be no clashes.
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
 */
int H5INset_attribute(hid_t locid, const char *attr_name, 
		const char *attr_data) {
	hid_t attr_type, attr_space_id, attr_id;
	hsize_t attr_size;

	/** Create the attribute */
	if ((attr_type = H5Tcopy(H5T_C_S1)) < 0)
		goto out;

	attr_size = strlen(attr_data) + 1; /* NULL TERMINATION*/

	if (H5Tset_size(attr_type, (size_t) attr_size) < 0)
		goto out;
	if (H5Tset_strpad(attr_type, H5T_STR_NULLTERM) < 0)
		goto out;
	if ((attr_space_id = H5Screate(H5S_SCALAR)) < 0)
		goto out;
	if ((attr_id = H5Acreate(locid, attr_name, attr_type, attr_space_id,
					H5P_DEFAULT)) < 0)
		goto out;
	if(H5Awrite(attr_id, attr_type, attr_data) < 0)
		goto out;

	if (H5Aclose(attr_id) < 0)
		goto out;
	if (H5Sclose(attr_space_id) < 0)
		goto out;
	if (H5Tclose(attr_type) < 0)
		return -1;
	return 1;

	/** Error zone, exiting gracefully */
out:
	printf("Error in set_attribute\n");
	H5E_BEGIN_TRY {
	H5Tclose(attr_type);
	H5Sclose(attr_space_id);
	H5Dclose(attr_id);
	} H5E_END_TRY;
	return -1;
}

/*-------------------------------------------------------------------------
 * Function: H5INget_index_name
 *
 * Purpose: Finding out the type of the index dataset.
 *
 * Return: Success: identifier to the proper type, Failure -1.
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: June 27th, 2005
 *
 * Comments: Finds out the type of the dataset to be index if it is a simple 
 * dataset, else finds out the type of the field to be indexed in a compound
 * datatype. The assumption is that there is no nesting in the compound type.
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
 */
char *H5INget_index_name(hid_t did, const char *field_name, 
		const char *dset_name) {

	hid_t dtype_id, class_id;

	if ((dtype_id = H5Dget_type(did)) < 0)
		goto out;
	if ((class_id = H5Tget_class(dtype_id)) < 0)
		goto out;
	if (class_id == H5T_COMPOUND) {
		if (H5Tclose(dtype_id) < 0)
			goto out;
		return (char *)field_name;
	}
	else if (class_id == H5T_INTEGER || class_id == H5T_FLOAT) {
		if (H5Tclose(dtype_id) < 0)
			goto out;
		return (char *)dset_name;
	}

	/** Error check zone, graceful exit*/
out:
	printf("Error in get_index_name\n");
	H5E_BEGIN_TRY {
		H5Tclose(dtype_id);
	} H5E_END_TRY
	return NULL;
}

/*-------------------------------------------------------------------------
 * Function: H5INget_index_grp
 *
 * Purpose: Reading the dataset attributes to see if an index is already 
 * present. If it is present then return the id of the group where indexes are
 * to be stored. Else return a negative value.
 *
 * Return: Success: identifier to the proper group, Failure -1.
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: June 27th, 2005
 *
 * Comments: This is so that the indexes of the same dataset are not stored in
 * multiple groups.
 *
 * Modifications: 
 *
 *--------------------------------------------------------------------------
 */
hid_t H5INget_index_grp(hid_t did) 
{
	hid_t attr_id, grp_id;
	hobj_ref_t grp_ref;
	
	if (H5INfind_attribute(did, "INDEX_GRP") <= 0)
		return -1;
	if ((attr_id = H5Aopen_name(did, "INDEX_GRP")) < 0)
			goto out;
	if (H5Aread(attr_id, H5T_STD_REF_OBJ, &grp_ref) < 0)
			goto out;
	if ((grp_id = H5Rdereference(did, H5R_OBJECT, &grp_ref)) < 0)
			goto out;

	if (H5Aclose(attr_id) < 0)
		goto out;

	return grp_id;
out:
	printf("Error in get_index_grp\n");
H5E_BEGIN_TRY { 
	H5Aclose(attr_id);
	H5Gclose(grp_id);
} H5E_END_TRY
	return -1;
}


/*-------------------------------------------------------------------------
 * Function: find_attr
 *
 * Purpose: operator function used by H5LT_find_attribute
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: June 21, 2001
 *
 * Comments: Copied from H5LT.c
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t find_attr( hid_t loc_id, const char *name, void *op_data)
{

 /* Define a default zero value for return. This will cause the iterator to 
	*	continue if the palette attribute is not found yet.
  */
 int ret = 0;   
 char *attr_name = (char*)op_data;
 
 /* Shut the compiler up */
 loc_id=loc_id;

 /* Define a positive value for return value if the attribute was found. This 
	*	will cause the iterator to immediately return that positive value, 
  * indicating short-circuit success 
  */

 if( strcmp( name, attr_name ) == 0 ) 
  ret = 1;

 return ret;
} 


/*-------------------------------------------------------------------------
 * Function: H5IN_find_attribute
 *
 * Purpose: Inquires if an attribute named attr_name exists attached to the 
 * object loc_id.
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: June 21, 2001
 *
 * Comments:
 *  The function uses H5Aiterate with the operator function find_attr. Copied
 * from H5LT.c
 *
 * Return:  
 *  Success: The return value of the first operator that
 *              returns non-zero, or zero if all members were
 *              processed with no operator returning non-zero.
 *
 *  Failure: Negative if something goes wrong within the
 *              library, or the negative value returned by one
 *              of the operators.
 *
 *-------------------------------------------------------------------------
 */
herr_t H5INfind_attribute(hid_t loc_id, const char* attr_name) {
 unsigned int attr_num;     
 herr_t       ret;

 attr_num = 0;
 ret = H5Aiterate(loc_id, &attr_num, find_attr, (void *)attr_name );
 return ret;
}


/*-------------------------------------------------------------------------
 * Function: H5INcreate_index_grp
 *
 * Purpose: Creates a new group of the specified group and adds a
 * reference to the group as an attribute of the spcified dataset.
 *
 * Return: Success: Identifier to the group, Failure: -1
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: July 11th, 2005
 *
 * Comments: Creates indexes only for single datasets.
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
 */
hid_t	H5INcreate_index_grp(hid_t did, hid_t loc_id, const char *name)
{
	hid_t grp_id, sid, attr_id;
	hobj_ref_t ref;
	hsize_t dims[1];
	dims[0] = 1;

	printf("Creating Index Group\n");
	if ((grp_id = H5Gcreate(loc_id, name, 4)) < 0)
		return -1;
	if (H5Rcreate(&ref, loc_id, name, H5R_OBJECT, -1) < 0)
		goto out;
	if ((sid = H5Screate_simple(1, dims, NULL)) < 0)
			goto out;
	if ((attr_id = H5Acreate(did, "INDEX_GRP", H5T_STD_REF_OBJ, sid, H5P_DEFAULT))
			 	< 0)
		goto out;
	if (H5Awrite(attr_id, H5T_STD_REF_OBJ, &ref) < 0)
		goto out;
	if (H5INset_attribute(grp_id, "CLASS", "INDEX") < 0) {
		printf("Attribute Not Set");
		goto out;
	}
	H5Sclose(sid);
	H5Aclose(attr_id);
	return grp_id;

out:
	printf("Error in create_index_grp\n");
	H5E_BEGIN_TRY {
		H5Gclose(grp_id);
		H5Sclose(sid);
		H5Aclose(attr_id);
	}H5E_END_TRY
	return -1;
}

/*-------------------------------------------------------------------------
 * Function: H5INremove_prev_index
 *
 * Purpose: Checks if the index is already made. If it is remove that dataset.
 * The assumption is that if the user is creating the index again some changes
 * have been made.
 *
 * Return: 
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: July 21st, 2005
 *
 * Comments: 
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
 */
int H5INremove_prev_index(hid_t grp_id, const char *index_name) {
	return H5Giterate(grp_id, ".", NULL, remove_index, (void *)index_name);
}

/*-------------------------------------------------------------------------
 * Function: remove_index
 *
 * Purpose: Operator to the H5Giterate call. Actually removes the index.
 *
 * Return: 
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: July 21st, 2005
 *
 * Comments: 
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
 */
herr_t remove_index(hid_t grp_id, const char *member_name, void *op_data) {
	if (strcmp(member_name, (char *)op_data) == 0) {
		//if (H5Gunlink(grp_id, member_name) <0)
			//return -1;
		//else 
			return 1;
	}
	return 0;
}

/*-------------------------------------------------------------------------
 * Function: H5INfind_index
 *
 * Purpose: Checks if the index is present. If it is returns an identifier to the
 * dataset containing the index.
 *
 * Return: Success: identifier to the dataset, Failure: -1
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: July 21st, 2005
 *
 * Comments: 
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
 */
hid_t H5INfind_index(hid_t grp_id, const char *index_name) {
	hid_t did;

	if (H5Giterate(grp_id, ".", NULL, find_index, (void *) index_name) <= 0)
		return -1;
	if ((did = H5Dopen(grp_id, index_name)) < 0)
		return -1;
	return did;
}

/*-------------------------------------------------------------------------
 * Function: find_index
 *
 * Purpose: Operator to the H5Giterate call. 
 *
 * Return: 0 if the dataset is not the required one, 1 if it is, and -1 in 
 * case of a failure.
 *
 * Programmer: Rishi Rakesh Sinha rsinha@ncsa.uiuc.edu
 *             
 *
 * Date: July 27th, 2005
 *
 * Comments: 
 *
 * Modifications: 
 *
 *-------------------------------------------------------------------------
 */
herr_t find_index(hid_t grp_id, const char *member_name, void *op_data) {
	if (strcmp(member_name, (char *)op_data) == 0) {
			return 1;
	}
	return 0;
}
