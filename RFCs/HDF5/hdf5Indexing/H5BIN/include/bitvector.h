#ifndef _BIT_VECTOR_
#define _BIT_VECTOR_

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "exceptions.h"
#include <string>
#include <string.h>
#include <iostream>

using namespace std;

#define NO_PAGES 16

class BitVector{
	/** The name of the file where this bit vector would be stored. */
	string file_name;

	/** The size of the memory buffer allowed by the system */
	size_t mem_buf_size;

	/** The size of data vector in memory buffer. */
	size_t mem_vector_size;

	/** Number of active bits in the active word. */
	unsigned active_word_size;

	/** This value stores the last active bit position */
	int last_active_bit;

	/** The active word */
	unsigned active_word;

	/** The actual buffer storing the values in memory. */
	unsigned *mem_res_vector;

	/** Decompressed bit vector. */
	unsigned *decompressed_vector;

	/** This boolean checks whether or not the file has been created. */
	bool file_created;

	/** This checks to see if the bitmap is being created or read */
	bool while_query;

	/** This variable is used to mark the current position during query */
	unsigned current_word_pos;

	/** This is the overall current index position */
	unsigned current_index_pos;

	/** This is the variable used to mark the current bit position in the current
		word during the query*/
	unsigned current_bit_pos;

	/** A boolean variable to check if the previous word is initialized */
	bool prev_word_initialized;

	/** This stores the previous word */
	unsigned prev_word;

	unsigned mask;

	/** This checks if it is in the creation phase.*/
	bool is_creation;

	static char decideOperation(unsigned word1, unsigned word2);

	static const char LITERAL_LITERAL = 0;
	static const char LITERAL_ONE = 1;
	static const char LITERAL_ZERO = 2;
	static const char ONE_LITERAL = 3;
	static const char ZERO_LITERAL = 4;
	static const char ONE_ONE = 5;
	static const char ZERO_ZERO = 6;
	static const char ONE_ZERO = 7;
	static const char ZERO_ONE = 8;

	/** This method flushes all the bits that are in the memory. */
	int close_vector();

	void initialize(bool creation, size_t size);

	inline void insertWord(unsigned word);

public:
	friend class SizeCompare;

	/**
		* This is the constructor for the class. It allocates the memory space for
		* the bit vector. Along with it stores the name of the file where the 
		* bit vector is to be stored on the disk.
		*
		* @param fname The name of the file where the bit vector is to be stored.
		*/
	BitVector(std::string fname);
	BitVector(unsigned *buffer, size_t size);
	BitVector(size_t size);
	
	/**
		* This is the destructor to the class. It flushes the bit vector that is in
		* the memory to the disk. It then frees the allocated memory. Eventually it
		* closes the file and exits.
		*/
	~BitVector();

	/** 
		* This is the method to set a bit in the bit vector.
		*
		* param position The position in the bitmap where the bit is to be set.
		*/
	void SetBit(unsigned position, unsigned value);

	/**
		* This is the method to get the next bit that is 1.
		*/
	unsigned openIterator();
 	unsigned getNext();
	bool endIterator();
	unsigned closeIterator();

	/**
		*	This is the method that gets the value of a particular bit.
		*
		* @param position The position in the bitvector that is asked for
		*/
	int getBit(unsigned position);

	/**
		* This method gets number of active points in the bit vector.
		*/
	int getNumActiveBits();
	void flushToFile();
	void insertNextWord(unsigned word);
	void insertNextCodeWord(unsigned word1, unsigned word2, unsigned
			&counter1, unsigned &counter2, unsigned &bit_counter1, unsigned
			&bit_counter2, char type);

		/**
	 * OR opreator overloaded for bitvectors
	 */
  friend BitVector *operator | (BitVector &b1,BitVector &b2);

	/**
	 * AND opreator overloaded for bitvectors
	 */
  friend BitVector *operator & (BitVector &b1,BitVector &b2);

  friend BitVector *operator -(BitVector &b1,BitVector &b2);

	void Decompress();
};
#endif
