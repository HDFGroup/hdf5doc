vpath %.C src
vpath %.h include
vpath %.o obj
CC=g++
HDF=/home/rsinha/bin/h5
LIB_DIR=./lib
HDF_LIB=$(HDF)/lib/libhdf5.a -ljpeg -lz
IND_LIB=$(LIB_DIR)/libindex.a
BUF_LIB=-lpthread
HDF_INC=$(HDF)/include 
INC=./include
CFLAGS= -g -Wall
OBJ_DIR=obj/
IND_OBJ=$(OBJ_DIR)bitvector.o 
OBJ= $(OBJ_DIR)H5BIN.o
BIT_SRC=bitvector.C 
BIT_OBJ= $(BIT_SRC:.C=.o)
BIT_INC=$(INC)/bitvector.h $(INC)/exceptions.h $(INC)/Types.h $(INC)/bitmap.h
AR=ar -r
RANLIB=ranlib

################################################################################
H5BIN: $(OBJ) $(IND_OBJ) libindex
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(HDF_LIB) $(IND_LIB) $(BUF_LIB)

$(OBJ): $(BIT_INC)

$(OBJ_DIR)%.o: %.C 
	$(CC) $(CFLAGS) -o $@ -I $(HDF_INC) -I $(INC) -c $<
################################################################################
libindex: $(IND_OBJ)
	$(AR) $@.a $(IND_OBJ)
	$(RANLIB) $@.a
	mv libindex.a lib
################################################################################
	
clean:
	rm -rf H5BIN
	rm -rf core*
	rm -rf $(OBJ)

#################################################################################
iclean:
	rm -rf $(LIB_DIR)/libindex.a
	rm -rf obj/bitvector.o 
