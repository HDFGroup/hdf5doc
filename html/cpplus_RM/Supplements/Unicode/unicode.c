/* Unicode test */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "testhdf5.h"

#define NUM_CHARS 16
#define MAX_STRING_LENGTH ((NUM_CHARS * 4) + 1) /* Max length in bytes */
#define MAX_PATH_LENGTH (MAX_STRING_LENGTH + 20) /* Max length in bytes */
#define MAX_CODE_POINT 0x200000
#define FILENAME "test_utf8.h5"

#define DSET1_NAME "dataset1"
#define DSET2_NAME "dataset2"
#define DSET3_NAME "dataset3"
#define DSET4_NAME "dataset4"
#define GROUP1_NAME "group1"
#define GROUP2_NAME "group2"
#define GROUP3_NAME "group3"
#define GROUP4_NAME "group4"
#define SLINK_NAME "soft_link"

#define RANK 1
#define COMP_INT_VAL 7
#define COMP_FLOAT_VAL -42.0
#define COMP_DOUBLE_VAL 42.0


/* Test function prototypes */
herr_t test_data(hid_t fid, const char *string);
herr_t test_objnames(hid_t fid, const char *string);
herr_t test_attrname(hid_t fid, const char *string);
herr_t test_compound(hid_t fid, const char *string);
herr_t test_enum(hid_t fid, const char *string);
herr_t test_opaque(hid_t fid, const char *string);

/* Utility function prototypes */
void dump_string(const char * string);
unsigned int write_char(unsigned int c, char * test_string, unsigned int cur_pos);

/*
 * test_data
 * Tests that UTF-8 can be used for user data.
 * Writes the string to a dataset and reads it back again.
 */
herr_t test_data(hid_t fid, const char *string)
{
  hid_t dtype_id, space_id, dset_id;
  hsize_t dims=1;
  char read_buf[MAX_STRING_LENGTH];
  herr_t ret;
  
  dtype_id = H5Tcopy(H5T_C_S1);
  CHECK(dtype_id, FAIL, "H5Tcopy");
  ret=H5Tset_size(dtype_id, MAX_STRING_LENGTH);
  CHECK(ret, FAIL, "H5Tset_size");

  /* Create dataspace for a dataset */
  space_id = H5Screate_simple(RANK, &dims, NULL);
  CHECK(space_id, FAIL, "H5Screate_simple");
  /* Create a dataset */
  dset_id=H5Dcreate(fid, DSET1_NAME, dtype_id, space_id, H5P_DEFAULT);
  CHECK(dset_id, FAIL, "H5Dcreate");

  /* Write UTF-8 string to dataset */
  ret = H5Dwrite(dset_id, dtype_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, string);
  CHECK(ret, FAIL, "H5Dwrite");

  /* Read string back and make sure it is unchanged */
  ret = H5Dread(dset_id, dtype_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, read_buf);
  CHECK(ret, FAIL, "H5Dread");

  VERIFY(strcmp(string, read_buf), 0, "strcmp");

  /* Close all */
  ret=H5Dclose(dset_id);
  CHECK(ret, FAIL, "H5Dclose");

  ret=H5Tclose(dtype_id);
  CHECK(ret, FAIL, "H5Tclose");
  ret=H5Sclose(space_id);
  CHECK(ret, FAIL, "H5Sclose");

  return 0;
}

/*
 * test_objnames
 * Tests that UTF-8 can be used for object names in the file.
 * Tests groups, datasets, named datatypes, and soft links.
 */
herr_t test_objnames(hid_t fid, const char* string)
{
  hid_t grp_id, grp1_id, grp2_id, grp3_id;
  hid_t type_id, dset_id, space_id;
  char read_buf[MAX_STRING_LENGTH];
  char path_buf[MAX_PATH_LENGTH];
  hsize_t dims=1;
  hobj_ref_t obj_ref;
  herr_t ret;

  /* Create a group with a UTF-8 name */
  grp_id = H5Gcreate(fid, string, 0);
  CHECK(grp_id, FAIL, "H5Gcreate");

  /* Set a comment on the group to test that we can access the group
   * Also test that UTF-8 comments can be read.
   */
  ret = H5Gset_comment(fid, string, string);
  CHECK(ret, FAIL, "H5Gset_comment");
  ret = H5Gget_comment(fid, string, MAX_STRING_LENGTH, read_buf);
  CHECK(ret, FAIL, "H5Gget_comment");

  ret = H5Gclose(grp_id);
  CHECK(ret, FAIL, "H5Gclose");

  VERIFY(strcmp(string, read_buf), 0, "strcmp");

  /* Create a new dataset with a UTF-8 name */
  grp1_id = H5Gcreate(fid, GROUP1_NAME, 0);
  CHECK(grp1_id, FAIL, "H5Gcreate");

  space_id = H5Screate_simple(RANK, &dims, NULL);
  CHECK(space_id, FAIL, "H5Screate_simple");
  dset_id=H5Dcreate(grp1_id, string, H5T_NATIVE_INT, space_id, H5P_DEFAULT);
  CHECK(dset_id, FAIL, "H5Dcreate");

  /* Make sure that dataset can be opened again */
  ret=H5Dclose(dset_id);
  CHECK(ret, FAIL, "H5Dclose");
  ret=H5Sclose(space_id);
  CHECK(ret, FAIL, "H5Sclose");

  dset_id=H5Dopen(grp1_id, string);
  CHECK(ret, FAIL, "H5Dopen");
  ret=H5Dclose(dset_id);
  CHECK(ret, FAIL, "H5Dclose");
  ret = H5Gclose(grp1_id);
  CHECK(ret, FAIL, "H5Gclose");

  /* Do the same for a named datatype */
  grp2_id = H5Gcreate(fid, GROUP2_NAME, 0);
  CHECK(grp2_id, FAIL, "H5Gcreate");

  type_id = H5Tcreate(H5T_OPAQUE, 1);
  CHECK(type_id, FAIL, "H5Tcreate");
  ret = H5Tcommit(grp2_id, string, type_id);
  CHECK(type_id, FAIL, "H5Tcommit");
  ret = H5Tclose(type_id);
  CHECK(type_id, FAIL, "H5Tclose");

  type_id = H5Topen(grp2_id, string);
  CHECK(type_id, FAIL, "H5Topen");
  ret = H5Tclose(type_id);
  CHECK(type_id, FAIL, "H5Tclose");

  /* Don't close the group -- use it to test that object references
   * can refer to objects named in UTF-8 */

  space_id = H5Screate_simple(RANK, &dims, NULL);
  CHECK(space_id, FAIL, "H5Screate_simple");
  dset_id=H5Dcreate(grp2_id, DSET3_NAME, H5T_STD_REF_OBJ, space_id, H5P_DEFAULT);
  CHECK(ret, FAIL, "H5Dcreate");

  /* Create reference to named datatype */
  ret = H5Rcreate(&obj_ref, grp2_id, string, H5R_OBJECT, -1);
  CHECK(ret, FAIL, "H5Rcreate");
  /* Write selection and read it back*/
  ret = H5Dwrite(dset_id, H5T_STD_REF_OBJ, H5S_ALL, H5S_ALL, H5P_DEFAULT, &obj_ref);
  CHECK(ret, FAIL, "H5Dwrite");
  ret = H5Dread(dset_id, H5T_STD_REF_OBJ, H5S_ALL, H5S_ALL, H5P_DEFAULT, &obj_ref);
  CHECK(ret, FAIL, "H5Dread");

  /* Ensure that we can open named datatype using object reference */
  type_id = H5Rdereference(dset_id, H5R_OBJECT, &obj_ref);
  CHECK(type_id, FAIL, "H5Rdereference");
  ret = H5Tcommitted(type_id);
  VERIFY(ret, 1, "H5Tcommitted");

  ret = H5Tclose(type_id);
  CHECK(type_id, FAIL, "H5Tclose");
  ret = H5Dclose(dset_id);
  CHECK(ret, FAIL, "H5Dclose");
  ret = H5Sclose(space_id);
  CHECK(ret, FAIL, "H5Sclose");
  ret = H5Gclose(grp2_id);
  CHECK(ret, FAIL, "H5Gclose");

  /* Create "group3".  Build a hard link from group3 to group2, which has
   * a datatype with the UTF-8 name.  Create a soft link in group3
   * pointing through the hard link to the datatype.  Give the soft
   * link a name in UTF-8.  Ensure that the soft link works. */

  grp3_id = H5Gcreate(fid, GROUP3_NAME, 0);
  CHECK(grp3_id, FAIL, "H5Gcreate");

  ret = H5Glink2(fid, GROUP2_NAME, H5G_LINK_HARD, grp3_id, GROUP2_NAME);
  CHECK(ret, FAIL, "H5Glink2");
  strcpy(path_buf, GROUP2_NAME);
  strcat(path_buf, "/");
  strcat(path_buf, string);
  ret = H5Glink(grp3_id, H5G_LINK_SOFT, path_buf, string);
  CHECK(ret, FAIL, "H5Glink");

  /* Open named datatype using soft link */
  type_id = H5Topen(grp3_id, string);
  CHECK(type_id, FAIL, "H5Topen");

  ret = H5Tclose(type_id);
  CHECK(type_id, FAIL, "H5Tclose");
  ret = H5Gclose(grp3_id);
  CHECK(ret, FAIL, "H5Gclose");

  return 0;
}

/*
 * test_attrname
 * Test that attributes can deal with UTF-8 strings
 */
herr_t test_attrname(hid_t fid, const char * string)
{
  hid_t group_id, attr_id;
  hid_t dtype_id, space_id;
  hsize_t dims=1;
  char read_buf[MAX_STRING_LENGTH];
  herr_t ret;

 /* Create a new group and give it an attribute whose
  * name and value are UTF-8 strings.
  */
  group_id = H5Gcreate(fid, GROUP4_NAME, 0);
  CHECK(group_id, FAIL, "H5Gcreate");

  space_id = H5Screate_simple(RANK, &dims, NULL);
  CHECK(space_id, FAIL, "H5Screate_simple");
  dtype_id = H5Tcopy(H5T_C_S1);
  CHECK(dtype_id, FAIL, "H5Tcopy");
  ret=H5Tset_size(dtype_id, MAX_STRING_LENGTH);
  CHECK(ret, FAIL, "H5Tset_size");

  /* Create the attribute and check that its name is correct */
  attr_id = H5Acreate(group_id, string, dtype_id, space_id, H5P_DEFAULT);
  CHECK(attr_id, FAIL, "H5Acreate");
  ret = H5Aget_name(attr_id, MAX_STRING_LENGTH, read_buf);
  CHECK(ret, FAIL, "H5Aget_name");
  ret = strcmp(read_buf, string);
  VERIFY(ret, 0, "strcmp");
  read_buf[0] = '\0';

  /* Try writing and reading from the attribute */
  ret = H5Awrite(attr_id, dtype_id, string);
  CHECK(ret, FAIL, "H5Awrite");
  ret = H5Aread(attr_id, dtype_id, read_buf);
  CHECK(ret, FAIL, "H5Aread");
  ret = strcmp(read_buf, string);
  VERIFY(ret, 0, "strcmp");

  /* Clean up */
  ret = H5Aclose(attr_id);
  CHECK(ret, FAIL, "H5Aclose");
  ret = H5Tclose(dtype_id);
  CHECK(ret, FAIL, "H5Tclose");
  ret = H5Sclose(space_id);
  CHECK(ret, FAIL, "H5Sclose");
  ret = H5Gclose(group_id);
  CHECK(ret, FAIL, "H5Gclose");

  return 0;
}

/*
 * test_attrname
 * Test that compound datatypes can have UTF-8 field names.
 */
herr_t test_compound(hid_t fid, const char * string)
{
  /* Define two compound structures, s1_t and s2_t.
   * s2_t is a subset of s1_t, with two out of three
   * fields.
   * This is stolen from the h5_compound example.
   */
  typedef struct s1_t {
      int    a;
      double c;
      float b;
  } s1_t;
  typedef struct s2_t {
      double c;
      int    a;
  } s2_t;
  /* Actual variable declarations */
  s1_t       s1;
  s2_t       s2;
  hid_t      s1_tid, s2_tid;
  hid_t      space_id, dset_id;
  hsize_t    dim = 1;
  char      *readbuf;
  herr_t     ret;

  /* Initialize compound data */
  s1.a = COMP_INT_VAL;
  s1.c = COMP_DOUBLE_VAL;
  s1.b = COMP_FLOAT_VAL;

  /* Create compound datatypes using UTF-8 field name */
  s1_tid = H5Tcreate (H5T_COMPOUND, sizeof(s1_t));
  CHECK(s1_tid, FAIL, "H5Tcreate");
  ret = H5Tinsert(s1_tid, string, HOFFSET(s1_t, a), H5T_NATIVE_INT);
  CHECK(ret, FAIL, "H5Tinsert");

  /* Check that the field name was stored correctly */
  readbuf=H5Tget_member_name(s1_tid, 0);
  ret = strcmp(readbuf, string);
  VERIFY(ret, 0, "strcmp");
  free(readbuf);

  /* Add the other fields to the datatype */
  ret = H5Tinsert(s1_tid, "c_name", HOFFSET(s1_t, c), H5T_NATIVE_DOUBLE);
  CHECK(ret, FAIL, "H5Tinsert");
  ret = H5Tinsert(s1_tid, "b_name", HOFFSET(s1_t, b), H5T_NATIVE_FLOAT);
  CHECK(ret, FAIL, "H5Tinsert");

  /* Create second datatype, with only two fields. */
  s2_tid = H5Tcreate (H5T_COMPOUND, sizeof(s2_t));
  CHECK(s2_tid, FAIL, "H5Tcreate");
  ret = H5Tinsert(s2_tid, "c_name", HOFFSET(s2_t, c), H5T_NATIVE_DOUBLE);
  CHECK(ret, FAIL, "H5Tinsert");
  ret = H5Tinsert(s2_tid, string, HOFFSET(s2_t, a), H5T_NATIVE_INT);
  CHECK(ret, FAIL, "H5Tinsert");

  /* Create the dataspace and dataset. */
  space_id = H5Screate_simple(1, &dim, NULL);
  CHECK(space_id, FAIL, "H5Screate_simple");
  dset_id = H5Dcreate(fid, DSET4_NAME, s1_tid, space_id, H5P_DEFAULT);
  CHECK(dset_id, FAIL, "H5Dcreate");

  /* Write data to the dataset. */
  ret = H5Dwrite(dset_id, s1_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, &s1);
  CHECK(ret, FAIL, "H5Dwrite");

  /* Ensure that data can be read back by field name into s2 struct */
  ret = H5Dread(dset_id, s2_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, &s2);
  CHECK(ret, FAIL, "H5Dread");

  VERIFY(s2.a, COMP_INT_VAL, "H5Dread");
  VERIFY(s2.c, COMP_DOUBLE_VAL, "H5Dread");

  /* Clean up */
  ret = H5Tclose(s1_tid);
  CHECK(ret, FAIL, "H5Tclose");
  ret = H5Tclose(s2_tid);
  CHECK(ret, FAIL, "H5Tclose");
  ret = H5Sclose(space_id);
  CHECK(ret, FAIL, "H5Sclose");
  ret = H5Dclose(dset_id);
  CHECK(ret, FAIL, "H5Dclose");

  return 0;
}

/*
 * test_enum
 * Test that enumerated datatypes can have UTF-8 member names.
 */
herr_t test_enum(hid_t UNUSED fid, const char * string)
{
  /* Define an enumerated type */
  typedef enum {
    E1_RED,
    E1_GREEN,
    E1_BLUE,
    E1_WHITE
  } c_e1;
  /* Variable declarations */
  c_e1 val;
  herr_t ret;
  hid_t type_id;
  char readbuf[MAX_STRING_LENGTH];

  /* Create an enumerated datatype in HDF5 with a UTF-8 member name*/
  type_id = H5Tcreate(H5T_ENUM, 1);
  CHECK(type_id, FAIL, "H5Tcreate");
  val = E1_RED;
  ret = H5Tenum_insert(type_id, "RED", &val);
  CHECK(ret, FAIL, "H5Tenum_insert");
  val = E1_GREEN;
  ret = H5Tenum_insert(type_id, "GREEN", &val);
  CHECK(ret, FAIL, "H5Tenum_insert");
  val = E1_BLUE;
  ret = H5Tenum_insert(type_id, "BLUE", &val);
  CHECK(ret, FAIL, "H5Tenum_insert");
  val = E1_WHITE;
  ret = H5Tenum_insert(type_id, string, &val);
  CHECK(ret, FAIL, "H5Tenum_insert");

  /* Ensure that UTF-8 member name gives the right value and vice versa. */
  ret = H5Tenum_valueof(type_id, string, &val);
  CHECK(ret, FAIL, "H5Tenum_valueof");
  VERIFY(val, E1_WHITE, "H5Tenum_valueof");
  ret = H5Tenum_nameof(type_id, &val, readbuf, MAX_STRING_LENGTH);
  CHECK(ret, FAIL, "H5Tenum_nameof");
  ret = strcmp(readbuf, string);
  VERIFY(ret, 0, "strcmp");

  /* Close the datatype */
  ret = H5Tclose(type_id);
  CHECK(ret, FAIL, "H5Tclose");

  return 0;
}

/*
 * test_opaque
 * Test comments on opaque datatypes 
 */
herr_t test_opaque(hid_t UNUSED fid, const char * string)
{
  hid_t type_id;
  char * read_buf;
  herr_t ret;

  /* Create an opaque type and give it a UTF-8 tag */
  type_id = H5Tcreate(H5T_OPAQUE, 4);
  CHECK(type_id, FAIL, "H5Tcreate");
  ret = H5Tset_tag(type_id, string);
  CHECK(ret, FAIL, "H5Tset_tag");

  /* Read the tag back. */
  read_buf = H5Tget_tag(type_id);
  ret = strcmp(read_buf, string);
  VERIFY(ret, 0, "H5Tget_tag");
  free(read_buf);

  ret = H5Tclose(type_id);
  CHECK(ret, FAIL, "H5Tclose");

  return 0;
}

/* Utility functions */
/* write_char
 * Append a unicode code point c to test_string in UTF-8 encoding.
 * Return the new end of the string.
 */
unsigned int write_char(unsigned int c, char * test_string, unsigned int cur_pos)
{
  if (c < 0x80) {
    test_string[cur_pos] = c;
    cur_pos++;
  }
  else if (c < 0x800) {
    test_string[cur_pos] = (0xC0 | c>>6);
    test_string[cur_pos+1] = (0x80 | (c & 0x3F));
    cur_pos += 2;
  }
  else if (c < 0x10000) {
    test_string[cur_pos] = (0xE0 | c>>12);
    test_string[cur_pos+1] = (0x80 | (c>>6 & 0x3F));
    test_string[cur_pos+2] = (0x80 | (c & 0x3F));
    cur_pos += 3;
  }
  else if (c < 0x200000) {
    test_string[cur_pos] = (0xF0 | c>>18);
    test_string[cur_pos+1] = (0x80 | (c>>12 & 0x3F));
    test_string[cur_pos+2] = (0x80 | (c>>6 & 0x3F));
    test_string[cur_pos+3] = (0x80 | (c & 0x3F));
    cur_pos += 4;
  }

  return cur_pos;
}

/* dump_string
 * Print a string both as text (which will look like garbage) and as hex.
 * The text display is not guaranteed to be accurate--certain characters
 * could confuse printf (e.g., '\n'). */
void dump_string(const char * string)
{
  unsigned int length;
  unsigned int x;

  printf("The string was:\n");
  printf(string);
  printf("Or in hex:\n");

  length = strlen(string);

    for(x=0; x<length; x++)
    printf("%x ", string[x] & (0x000000FF));

  printf("\n");
}

/* Main test.
 * Create a string of random Unicode characters, then run each test with
 * that string.
 */
int main()
{
  char test_string[MAX_STRING_LENGTH];
  unsigned int cur_pos=0;      /* Current position in test_string */
  unsigned int unicode_point;  /* Unicode code point for a single character */
  hid_t fid;                   /* ID of file */
  herr_t num_errors =0;
  int x;                       /* Temporary variable */
  herr_t ret_val = 0;

  /* Create a random string with length NUM_CHARS */
  HDsrandom((unsigned long)HDtime(NULL));

  for(x=0; x<NUM_CHARS; x++)
  {
    /* We need to avoid unprintable characters (codes 0-31) and the
     * . and / characters, since they aren't allowed in path names.
     */
    unicode_point = HDrandom() % (MAX_CODE_POINT-32) + 32;
    if(unicode_point != 56 && unicode_point != 57)
      cur_pos = write_char(unicode_point, test_string, cur_pos);
  }
  test_string[cur_pos]='\0';

  /* Create file */
  fid = H5Fcreate(FILENAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  CHECK(fid, FAIL, "H5Fcreate");

  num_errors += test_data(fid, test_string);
  num_errors += test_objnames(fid, test_string);
  num_errors += test_attrname(fid, test_string);
  num_errors += test_compound(fid, test_string);
  num_errors += test_enum(fid, test_string);
  num_errors += test_opaque(fid, test_string);

  if(num_errors >0)
  {
    printf("UTF-8 test failed!\n");
    dump_string(test_string);
    ret_val = 1;
  }

  return ret_val;
}

