#include "hdf5.h"
#include <assert.h>

int main()
{
    hid_t fid;      /* File ID */
    hid_t sid;      /* Dataspace ID */
    hid_t did;      /* Dataset ID */
    herr_t ret;     /* Generic return value */

    /* Create the file */
    fid=H5Fcreate("example2.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    assert(fid>=0);

    /* Create a scalar dataspace for the dataset */
    sid=H5Screate(H5S_SCALAR); 
    assert(sid>=0);

    /* Create a trivial dataset */
    did=H5Dcreate(fid, "Dataset", H5T_NATIVE_INT, sid, H5P_DEFAULT);
    assert(did>=0);

    /* Close the dataset */
    ret=H5Dclose(did);
    assert(ret>=0);

    /* Close the dataspace */
    ret=H5Sclose(sid);
    assert(ret>=0);

    /* Close the file */
    ret=H5Fclose(fid);
    assert(ret>=0);

    return(0);
}

