HDF=/usr/hdf/
CC=$(HDF)/bin/h5cc
CC_FLAGS=-g
INC = -I$HDF/include/

index: Main.o H5IN.o
	$(CC) $(CC_FLAGS) -o index Main.o H5IN.o

Main.o: Main.c
	$(CC) $(CC_FLAGS) -c Main.c

H5IN.o: H5IN.c
	$(CC) $(CC_FLAGS) -c H5IN.c

clean:
	rm *.o index
