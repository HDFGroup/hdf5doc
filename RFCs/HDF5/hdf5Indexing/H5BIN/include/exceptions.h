#ifndef _EXCEPTION_

#define _EXCEPTION_

#include <errno.h>
#include <stdio.h>
#include <string.h>

/* Class for capturing all errors */
class BaseErr{
public:
  virtual void print_err() =0;
	virtual ~BaseErr() {
	}
};

/* Class for capturing general errors */
class GenErr: public virtual BaseErr{
public:
  int err_type;
  static const int mem_alloc_fail=0;
  GenErr(int errtype){err_type=errtype;};
  void print_err(){
    printf("General Error:");
    switch (err_type){
    case mem_alloc_fail:
      printf("Insufficient Memory\n");
      break;
    }
  }
};
/* Class for capturing File related Errors*/


class FileErr: public virtual BaseErr {

public:
  int err_type;
  int err_no;
  static const int file_open_error=0;
  static const int file_read_error=1;
  static const int file_write_error=2;
  static const int file_append_error=3;
  static const int file_stat_error=4;
  static const int file_mmap_error=5;
  static const int file_munmap_error=6;
  static const int file_delete_error=7;
  FileErr(int errtype,int sys_err_no){
    err_type=errtype;
    err_no=sys_err_no;
  }
  void print_err(){
    switch (err_type){
      // case FILE_OPEN_ERROR:
    case file_open_error:
      printf("File Open Error:%s\n",strerror(err_no));
      break;
    case file_read_error:
      printf("File Read Error:%s\n",strerror(err_no));
      break;
    case file_write_error:
      printf("File Write Error:%s\n",strerror(err_no));
      break;
    case file_append_error:
      printf("File Append Error:%s\n",strerror(err_no));
      break;
    case file_stat_error:
      printf("File GetStat Error:%s\n",strerror(err_no));
      break;
    case file_mmap_error:
      printf("File Mmap Error:%s\n",strerror(err_no));
      break;
    case file_munmap_error:
      printf("File Mmap Error:%s\n",strerror(err_no));
      break;
    case file_delete_error:
      printf("File Delete Error:%s\n",strerror(err_no));
      break;

    }
  }
}
  ;

class BitVectorErr:public virtual BaseErr{
public:
  int err_type;

  static const int invalid_arg=0;
  static const int out_of_bound=1;
  static const int empty_bit_vector=2;
  static const int mem_alloc_fail=3;
  static const int invalid_bitfile=4;
  static const int not_implemented=5;
	static const int open_error = 10;
	static const int read_error = 6;
	static const int close_error = 7;
	static const int invalid_bitfile_size = 8;
	static const int unknown_oper = 9;

  BitVectorErr(int errtype){err_type=errtype;};

  void print_err(){
    switch (err_type){
    case invalid_arg:
      printf("BitVector Invalid Argument\n");
      break;
    case out_of_bound:
      printf("BitVector Index out of Bound\n");
      break;
    case empty_bit_vector:
      printf("BitVector Empty BitVector\n");
      break;
    case mem_alloc_fail:
      printf("BitVector Memory Allocate Fail\n");
      break;
    case invalid_bitfile:
      printf("BitVector BitFile not proper\n");
      break;
    case not_implemented:
      printf("Not implemented\n");
      break;
		case invalid_bitfile_size:
			printf("BitFile size is invalid\n");
			break;
		case read_error:
			printf("Unable to read from the bitfile\n");
			break;
		case close_error:
			printf("unable to close the file\n");
			break;
		case unknown_oper:
			printf("The Type of sub operation chosen is unknown\n");
			break;
		case open_error:
			printf("Unable to open the bitfile\n");
			break;
    }
  }
}
  ;

class BitMapIndexErr: public virtual BaseErr{

public:
  int err_type;
  static const int val_outof_range =0;
  static const int dir_open_fail=1;
  static const int mem_alloc_fail=2;
  static const int val_outof_bins=3;
  static const int rsearch_error=4;
  static const int invalid_arg=5;
  BitMapIndexErr(int errtype){
    err_type=errtype;
  }
  void print_err(){
    switch (err_type){
    case val_outof_range:
      printf("BitMapIndex Value not in range\n");
      break;
    case dir_open_fail:
      printf("BitMapIndex Failed to open dir\n");
      break;
    case mem_alloc_fail:
      printf("BitMapIndex Failed to allocate mem\n");
      break;
    case val_outof_bins:
      printf("BitMapIndex: Value not in bin range\n");
      break;
    case rsearch_error:
      printf("BitMapIndex: Range Search Error");
      break;
    case invalid_arg:
      printf("BitMapIndex: Invalid Argument");
      break;


    }
  }

};
#endif
