
/****************************************************************************
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

#ifndef _H5IN_H
#define _H5IN_H

#include "hdf5.h"
#include <stdlib.h>
#include <string.h>

#define ADD_TARGET_FIELD 1
#define NO_TARGET_FIELD 0
#define MAX_MEM_SIZE 1024*1024
#define RANK 1
#define MEM_TYPE 1
#define NOT_MEM_TYPE 0
#define UBOUND 0
#define LBOUND 1

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
 * Create Functions.
 **************************************************************************/

/**
 * This method is used to create a new index. Under the current implementation
 * the index is static and indexes only one dataset.
 *
 * @param grp_name This parameter determines the name of the group where
 * the index would be stored.
 * @param grp_loc_id This is the location where the group, which contains the
 * indexes is to be stored.
 * @param property_list This specifies the various storage properties of the
 * dataset that is storing the index.
 * @param dset_loc_id This is the location where the dataset to be indexed is
 * stored.
 * @param data_loc_name This is the name of the dataset to be indexed.
 * @param field_name This is the field in a compound data type.
 * @param max_mem_size This the the max sime of a single buffer to bw allocated.
 * This should be half the size of memory that can be efficiently mapped. This
 * half size restriction is due to some memory copies that are required by the
 * algorithm.
 *
 * @return Success: Returns the identifier to the group where the index was 
 * stored; Failure: -ve value.
 */	
hid_t	H5INcreate(const char *grp_name, hid_t grp_loc_id,
		const hid_t property_list, hid_t data_loc_id, const char *data_loc_name,
	 	const char *field_name, hsize_t max_mem_size);

/**************************************************************************
 * Query Functions
 *************************************************************************/

/**
 * This is the method used to query from an index. The assumption is that with
 * an index the query would be much faster. Also a query at this time can only
 * be performed on a single dataset and not across datasets.
 *
 * @param dset_id This is the dataset on which the query is being done.
 * @param keys These are the keys on which the query is being done. The queries
 * being supported are simple range queries with a conjunction of various key
 * predicates. The reason for conjunction only is that disjunction can as 
 * easily be created by the user issuing 2 queries.
 * @param ubounds Since the queries being supported are range queries, we need
 * specify the upper and lower bounds. The upper bound for the various keys is
 * specified in this array.
 * @param lbounds This variable specifies the lower bounds of the keys.
 * @param nkeys Sepcifies the number of keys that the query is run on.
 *
 * @return a dataspace identifier if the query is succesful or a -ve number
 * if the query is not succesful.
 */
hid_t H5INquery(hid_t dset_id,
		const char **keys,
		void *ubounds,
		void *lbounds,
		int nkeys);

#ifdef __cplusplus
}
#endif

#endif
