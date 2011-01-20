#include "hdf5.h"
#include <assert.h>

int main()
{
    hid_t fid;      /* File ID */
    herr_t ret;     /* Generic return value */

    /* Create the file */
    fid=H5Fcreate("example1.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    assert(fid>=0);

    /* Close the file */
    ret=H5Fclose(fid);
    assert(ret>=0);

    return(0);
}
