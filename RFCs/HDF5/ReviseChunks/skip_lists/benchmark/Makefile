# Enable support for direct I/O
# OPTIONS = -DDIRECT_IO_SUPPORT -D_GNU_SOURCE

# C compiler to use
CC = gcc
#CC = gcc4

# Base C flags
CFLAGS_ALL = $(OPTIONS) -std=c99 -pedantic -Wall -Wextra -Wundef -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wsign-compare -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Winline -Wno-long-long -Wfloat-equal -Wmissing-format-attribute -Wmissing-noreturn -Wpacked -Wdisabled-optimization -Wmultichar -Wendif-labels -Wdeclaration-after-statement -Wold-style-definition -Winvalid-pch -Wmissing-field-initializers -Wvariadic-macros -fverbose-asm
#CFLAGS_ALL = $(OPTIONS) -std=gnu99 -pedantic -Wall -Wextra -Wundef -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wsign-compare -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Winline -Wno-long-long -Wfloat-equal -Wmissing-format-attribute -Wmissing-noreturn -Wpacked -Wdisabled-optimization -Wmultichar -Wendif-labels -Wdeclaration-after-statement -Wold-style-definition -Winvalid-pch -Wmissing-field-initializers -Wvariadic-macros -fverbose-asm

# Debugging
#CFLAGS = $(CFLAGS_ALL) -g

#Production
CFLAGS = $(CFLAGS_ALL) -O2

#Profiling
#CFLAGS = $(CFLAGS_ALL) -g -pg

# define any directories containing header files other than /usr/include
#
#INCLUDES = -I../source/hdf5/src -I../build/hdf5_dbg/src
INCLUDES = -I../source/hdf5/src -I../build/hdf5_prod/src
HDRS = benchmark.h

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
#LFLAGS = -L../build/hdf5_dbg/src/.libs
LFLAGS = -L../build/hdf5_prod/src/.libs

# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname 
#   option, something like (this will link in libmylib.so and libm.so:
LIBS = -lhdf5 -lz

# define the C source files
COMMON_SRCS = common.c
BTREE_SRCS = btree.c
BTREE2_SRCS = btree2.c
EARRAY_SRCS = earray.c
SRCS = $(COMMON_SRCS) $(BTREE_SRCS) $(BTREE2_SRCS) $(EARRAY_SRCS)

# define the C object files 
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
COMMON_OBJS = $(COMMON_SRCS:.c=.o)
BTREE_OBJS = $(BTREE_SRCS:.c=.o)
BTREE2_OBJS = $(BTREE2_SRCS:.c=.o)
EARRAY_OBJS = $(EARRAY_SRCS:.c=.o)
OBJS = $(SRCS:.c=.o)

# define the executable files
BTREE = btree
BTREE2 = btree2
EARRAY = earray
PROGS = $(BTREE) $(BTREE2) $(EARRAY)

# define the data files produced
BTREE_DATAFILE = btree.h5
BTREE2_DATAFILE = btree2.h5
EARRAY_DATAFILE = earray.h5
DATAFILE = $(BTREE_DATAFILE) $(BTREE2_DATAFILE) $(EARRAY_DATAFILE)

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all:    $(PROGS)
	@echo  Simple benchmarking tools built

$(BTREE): $(BTREE_OBJS) $(COMMON_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BTREE) $(BTREE_OBJS) $(COMMON_OBJS) $(LFLAGS) $(LIBS)

$(BTREE2): $(BTREE2_OBJS) $(COMMON_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BTREE2) $(BTREE2_OBJS) $(COMMON_OBJS) $(LFLAGS) $(LIBS)

$(EARRAY): $(EARRAY_OBJS) $(COMMON_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(EARRAY) $(EARRAY_OBJS) $(COMMON_OBJS) $(LFLAGS) $(LIBS)

# Currently, all the source files use the same headers...
$(OBJS): $(HDRS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	$(RM) *.o *~ $(PROGS) $(DATAFILE)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
