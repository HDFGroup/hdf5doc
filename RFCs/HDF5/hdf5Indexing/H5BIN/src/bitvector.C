#include "bitvector.h"

// Intialization methods.
BitVector::BitVector(size_t size) {
	initialize(false, size);
}

BitVector::BitVector(std::string fname)
{
	file_name = fname;
	file_created = false;
	mem_buf_size = NO_PAGES * getpagesize()/sizeof(unsigned);

	initialize(true, mem_buf_size);
}

BitVector::BitVector(unsigned *buffer, size_t size) {
	is_creation = false;
	if (size != 0) {
		mem_vector_size = size - 1;
		mem_res_vector = new unsigned[mem_vector_size];
		memcpy(mem_res_vector, buffer, mem_vector_size * sizeof(unsigned));
		active_word_size = buffer[mem_vector_size];
	}
	else {
		mem_vector_size = 1;
		mem_res_vector = new unsigned[mem_vector_size];
		mem_res_vector[0] = 0;
		active_word_size = 0;
	}
}

void BitVector::initialize(bool creation, size_t size) {
	active_word = 0;
	active_word_size = 0;
	last_active_bit = 0;
	prev_word = 0;
	prev_word_initialized = false;
	mem_vector_size = 0;
	is_creation = creation;
	mem_res_vector = new unsigned[size];
}

BitVector::~BitVector() {
	if (is_creation)
		close_vector();
	delete[](mem_res_vector);
}

// Operations for disk based arrays.
int BitVector::close_vector() {
	int fd;
	if (mem_vector_size == 0 && active_word == 0)
		return 1;
	if (!file_created)
		fd = open(file_name.c_str(), O_WRONLY | O_CREAT | S_IRWXU);
	else
		fd = open(file_name.c_str(), O_WRONLY | O_APPEND);

	write(fd, mem_res_vector, mem_vector_size*sizeof(unsigned));

	if (prev_word_initialized) 
		write(fd, &prev_word, sizeof(unsigned));

	write(fd, &active_word, sizeof(unsigned));
	write(fd, &active_word_size, sizeof(unsigned));
	close(fd);
	chmod(file_name.c_str(), S_IRWXU);
	return 1;
}

void BitVector::flushToFile() {
	int fd;
	if (!file_created) {
		if ((fd = open(file_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC)) < 0) 
			throw BitVectorErr(BitVectorErr::open_error);
		file_created = true;
		chmod(file_name.c_str(), S_IRWXU);
	}
	else
		if ((fd = open(file_name.c_str(), O_WRONLY | O_APPEND)) < 0)
			throw BitVectorErr(BitVectorErr::open_error);

	if (write(fd, mem_res_vector, mem_vector_size*sizeof(unsigned)) < 0)
		 throw BitVectorErr(BitVectorErr::read_error);
	if (close(fd) < 0)
		 throw BitVectorErr(BitVectorErr::close_error);
}

//Creation Operations.

void BitVector::insertWord(unsigned word) {
	// Push the word onto the stack.
	mem_res_vector[mem_vector_size++] = word;
	// If it is the index creation phase a lot of indexes are being created in
	// parallel so the amount of memory available to one index is limited. Thus we
	// need to flush the memory for this into a file.
	if (mem_vector_size == mem_buf_size && is_creation) {
		flushToFile();
		// Reusing the same memory location.
		mem_vector_size = 0;
	}
}

void BitVector::SetBit(unsigned position, unsigned value) {
	int word_position;

	// Identiying which bit from the current position needs to be set.
	// position is the position being given, last active bit is the position of
	// current last active bit, and active_word_size is the position in the 
	// current word where the last active bit is present. This formulae
	// gives us how far from the 2nd bit (1st bit is the bit to differentiate 
	// between the literal and run word) should the new active bit be placed.
	word_position = position - last_active_bit + active_word_size;

	// Invariant is active_word can never be all zero.
	// If the change has to be made in the same word.
	if (word_position < 31) {
		//printf("WORD POSITION < 32\n");
		// word position is position from the left where the bit should be placed
		// From the right the number of shifts becomes 30 - word_position
		// This is because the 1st bit is flip bit.
		unsigned temp = value << (30 - word_position);
		// We update teh active word.
		active_word |= temp;
		// The active word size is now the word position, because this is the last
		// active bit in the word.
		active_word_size = word_position;
		//printf("1. %x, %x\n", prev_word, active_word);
	}
	// If the bit is not in the current word.
	else {
		// If the current word is a literal word with all ones.
		if (active_word == (UINT_MAX >> 1)) {
			// If previous word was also a literal with all ones, encode the two
			// words into one word.
			if (prev_word == active_word) {
				//	printf("1. %x, %x\n", prev_word, active_word);
				prev_word = (3 << 30) + 2;
			}
			// Else if the previous word is an run word of 1s then increment the
			// count of the run.
			else if ((prev_word >> 30) == 3)
			{
				//printf("2. %x, %x\n", prev_word, active_word);
				prev_word += 1;
			}
			// If the previous word is not ones, then push the previous word onto the
			//  array vector, and move the current word to the previous word.
			else {
				//printf("3. %x, %x\n", prev_word, active_word);
				// Insert into the array only if the previous word is initialized else a
				// false entry would be made into the vector.
				if (prev_word_initialized)
					insertWord(prev_word);
				prev_word = active_word;
				prev_word_initialized = true;
				active_word = 0;
				active_word_size = 0;
			}
		}
		// If the current word is neither all 1s nor is it a run of 1s
		// We start with pushing the previous word onto the vector 
		// Then move the current word to the previous word.
		else {
			if (prev_word_initialized)
				insertWord(prev_word);
			prev_word = active_word;
			prev_word_initialized = true;
			active_word = 0;
			active_word_size = 0;
		}

		// If the bit is in the next word then create a new current word and make
		// the corresponding bit 1.
		if (word_position > 30 && word_position < 62) {
			word_position-= 31;
			active_word = 1 << (30 - word_position);
			active_word_size = word_position;
		}
		// If the bit is in the 2nd word then 
		else if (word_position> 61 && word_position< 93) {
			word_position-= 62;
			// If prev_word is not equal to zero, push the previous word onto the
			// vector and
			if (prev_word_initialized && prev_word != 0)
				insertWord(prev_word);
			// If previous word is zero encode the previous word to denote a run.
			if (prev_word == 0)
				prev_word = (2 << 30) + 2;
			// If previous word is not 0 then make previous word 0
			else
				prev_word = 0;
			prev_word_initialized = true;
			//Update the active word and the active word position.
			active_word = 1 << (30 - word_position);
			active_word_size = word_position;
		}
		else {
			//printf("WORD POSITION NOT IN NEXT 2 WORDS\n");
			// We calcuate the number of words that are going to be encoded.
			int no_words = word_position / 31 - 1;
			// This is the position of the bit in the new active word.
			word_position = word_position % 31;

			/*if (word_position == 0) {
				word_position = 31;
				no_words--;
				}*/

			// If previous word is initialized and is not zero push it up into the 
			// vector.
			if (prev_word_initialized && prev_word != 0)
				insertWord(prev_word);
			// If previous word is zero then encode it to represent the run of
			// no_words + 1 words.
			if (prev_word == 0)
				prev_word = (2 << 30) + no_words + 1;
			// If the previous word is not zero, just overwrite it to represent the
			// run of no_words words. The previous value has already been pushed up
			// the stack so there is no problem
			else
				prev_word = (2 << 30) + no_words;
			prev_word_initialized = true;
			//Update the active word and the active word position.
			active_word = 1 << (30 - word_position);
			active_word_size = word_position;
		}
	}
	last_active_bit = position;
}

//Iterator Operations.
unsigned BitVector::openIterator() {
	current_index_pos = 0;
	current_word_pos = 0;
	current_bit_pos = 0;
	return 1;
}

unsigned BitVector::getNext() {
	unsigned current_word = mem_res_vector[current_word_pos];
	//printf("%d, %d, %d, %d, %x\n", (int)current_word_pos, (int)mem_vector_size,
			//(int)current_bit_pos, (int)active_word_size, current_word); 
	unsigned prev_word = current_word;
	while (current_word == 0 || (current_word >> 30) == 2) {
		if (current_word != 0) {
			current_word = (current_word << 1) >> 1;
			current_index_pos += current_word * 31;	 
		}
		else 
			current_index_pos += 31;
		current_word_pos++;
		current_word = mem_res_vector[current_word_pos];
	}
	if (current_word != prev_word) {
		current_bit_pos = 0;
		return getNext();
	}
	else {
		if ((current_word >> 30) == 3) {
			current_word = (current_word << 2) >> 2;
			unsigned no_bits = current_word * 31;
			if (current_bit_pos == no_bits) {
				current_bit_pos = 0;
				current_word_pos++;
				return getNext();
			}
			else {
				current_bit_pos++;
				return current_index_pos++;
			}
		}
		else {
			while(current_bit_pos < 32){
				if ((current_word & (1 << (30 - current_bit_pos))) != 0) {
					current_bit_pos++;
					return current_index_pos++;
				}
				current_index_pos++;
				current_bit_pos++;
			}
			current_index_pos--;
			current_bit_pos = 0;
			current_word_pos++;
			return getNext();
		}
	}
}

bool BitVector::endIterator() {
	if (mem_vector_size == 0)
		return true;
	if (active_word == 0 & mem_vector_size == 1)
		return true;
	if (((unsigned)(active_word_size+1) == current_bit_pos) && (mem_vector_size ==
				current_word_pos+1))
		return true;
	else 
		return false;
}

unsigned BitVector::closeIterator() {
	return 1;
}

// BitVector Operations.

void BitVector::insertNextWord(unsigned word) {
	unsigned bits;
	//printf("AW %x, W %x\n", active_word, word);
	if (!prev_word_initialized) {
		//printf("SECTION 1\n");
		active_word = word;
		prev_word_initialized = true;
		//printf("AW %x, W %x\n", active_word, word);
		return;
	}
	else if ((word >> 31) == 0 && word != 0) {
		//printf("SECTION 2\n");
		insertWord(active_word);
		active_word = word;
		//printf("AW %x, W %x\n", active_word, word);
	}
	else {
		//printf("SECTION 3\n");
		if (word == 0 || (word >> 30) == 2)
			bits = 2 << 30;
		else if (word == UINT_MAX >> 1 || (word >> 30) == 3)
			bits = 3 << 30;
		else 
			bits = UINT_MAX;
		if (active_word == word && word == 0) {
		//	printf("SUBSECTION 1 %x\n", bits);
			active_word = bits + 2;
		//	printf("AW %x, W %x\n", active_word, word);
		}
		else if ((active_word & bits) == bits) {
		//	printf("SUBSECTION 2 %x\n", bits);
			if (word == 0)
				active_word++;
			else
				active_word += ((word << 2) >> 2);
		//	printf("AW %x, W %x\n", active_word, word);
		}
		else {
		//	printf("SUBSECTION 3 %x\n", bits);
			insertWord(active_word);
			active_word = word;
		//	printf("AW %x, W %x\n", active_word, word);
		}
	}
}

char BitVector::decideOperation(unsigned word1, unsigned word2) {
	//printf("%x, %x\n", word1, word2);
	//printf("%d, %d\n", word1 >> 30, word2 >> 30);
	if ((word1 >> 31) == 0 && (word2 >> 31) == 0)
		return LITERAL_LITERAL;
	else if ((word1 >> 31) == 0 && (word2 >> 30) == 3)
		return LITERAL_ONE;
	else if ((word1 >> 31) == 0 && (word2 >> 30) == 2)
		return LITERAL_ZERO;
	else if ((word1 >> 30) == 3 && (word2 >> 31) == 0)
		return ONE_LITERAL;
	else if ((word1 >> 30) == 2 && (word2 >> 31) == 0)
		return ZERO_LITERAL;
	else if ((word1 >> 30) == 3 && (word2 >> 30) == 3)
		return ONE_ONE;
	else if ((word1 >> 30) == 2 && (word2 >> 30) == 2)
		return ZERO_ZERO;
	else if ((word1 >> 30) == 3 && (word2 >> 30) == 2)
		return ONE_ZERO;
	else if ((word1 >> 30) == 2 && (word2 >> 30) == 3)
		return ZERO_ONE;
	else
		return -1;
}	

BitVector *operator | (BitVector &b1, BitVector &b2) {
	BitVector *b3 = new BitVector(1028*1028*100);

	unsigned count1 = 0, count2 = 0, count;
	unsigned word_count1 = 0, word_count2 = 0;
	unsigned word_size1, word_size2, word;
	//printf("MEMVS %d, %d\n", (int)b1.mem_vector_size, (int)b2.mem_vector_size);

	while (count1 < b1.mem_vector_size && 
			count2 < b2.mem_vector_size) {
		char oper = BitVector::decideOperation(b1.mem_res_vector[count1],
				b2.mem_res_vector[count2]);
		switch (oper) {
			case BitVector::LITERAL_LITERAL:
			//	printf("LITERAL_LITERAL\n");
				b3->insertNextWord(b1.mem_res_vector[count1] |
						b2.mem_res_vector[count2]);
				count1++; count2++;
				break;
			case BitVector::ONE_LITERAL: 
				//printf("LITERAL_ONE\n");
				b3->insertNextWord(UINT_MAX >> 1);
				count2++; word_count2 = 0;
				word_count1++;
				if (word_count1 == 
						((b1.mem_res_vector[count1] << 2) >> 2)) {
					count1++;
					word_count1 = 0;
				}
				break;
			case BitVector::ZERO_LITERAL:
				//printf("LITERAL_ZERO\n");
				b3->insertNextWord(b2.mem_res_vector[count2]);
				count2++; word_count2 = 0;
				word_count1++;
				if (word_count1 == 
						((b1.mem_res_vector[count1] << 2) >> 2)) {
					count1++;
					word_count1 = 0;
				}
				break;
			case BitVector::LITERAL_ONE:
				//printf("ONE_LITERAL\n");
				b3->insertNextWord(UINT_MAX >> 1);
				count1++; word_count1 = 0;
				word_count2++;
				if (word_count2 == 
						((b2.mem_res_vector[count2] << 2) >> 2)) {
					count2++;
					word_count2 = 0;
				}
				break;
			case BitVector::LITERAL_ZERO:
				//printf("ZERO_LITERAL\n");
				b3->insertNextWord(b1.mem_res_vector[count1]);
				count1++; word_count1 = 0;
				word_count2++;
				if (word_count2 == 
						((b2.mem_res_vector[count2] << 2) >> 2)) {
					count2++;
					word_count2 = 0;
				}
				break;
			case BitVector::ONE_ONE:
				//printf("ONE_ONE\n");
				word_size1 = ((b1.mem_res_vector[count1] << 2) >> 2);
				word_size2 = ((b2.mem_res_vector[count2] << 2) >> 2);
				if ((word_size1 - word_count1) < (word_size2 - word_count2)) {
					count = word_size1 - word_count1;
					count1++; word_count1 = 0;
					word_count2 += count;
					if (word_count2 == word_size2) {
						count2++; word_count2 = 0;		
					}
				}
				else {                             
					count = word_size2 - word_count2;
					count2++; word_count2 = 0;
					word_count1 += count;
					if (word_count1 == word_size1) {
						count1++; word_count1 = 0;		
					}
				}

				if (count == 1)
					if (b3->active_word == 0)
						b3->active_word = (2 << 30) + 2;
					else if ((b3->active_word >> 30) == 2)
						b3->active_word++;
					else {
						b3->insertWord(b3->active_word);
						b3->active_word = 0;
					}
				else 
					if (b3->active_word == 0)
						b3->active_word = (2 << 30) + count + 1;
					else if ((b3->active_word >> 30) == 2)
						b3->active_word += count;
					else {
						b3->insertWord(b3->active_word);
						b3->active_word = (2 << 30) + count;
					}
				break;
			case BitVector::ZERO_ZERO:
				//printf("ZERO_ZERO\n");
				word_size1 = ((b1.mem_res_vector[count1] << 2) >> 2);
				word_size2 = ((b2.mem_res_vector[count2] << 2) >> 2);
				if ((word_size1 - word_count1) < (word_size2 - word_count2)) {
					count = word_size1 - word_count1;
					count1++; word_count1 = 0;
					word_count2 += count;
					if (word_count2 == word_size2) {
						count2++; word_count2 = 0;		
					}
				}
				else {                             
					count = word_size2 - word_count2;
					count2++; word_count2 = 0;
					word_count1 += count;
					if (word_count1 == word_size1) {
						count1++; word_count1 = 0;		
					}
				}
				if (count == 1)
					word = 0;
				else 
					word = (2 << 30) + count;
				b3->insertNextWord(word);
				break;
			case BitVector::ONE_ZERO:
				//printf("ONE_ZERO\n");
				word_size1 = ((b1.mem_res_vector[count1] << 2) >> 2);
				word_size2 = ((b2.mem_res_vector[count2] << 2) >> 2);
				if ((word_size1 - word_count1) < (word_size2 - word_count2)) {
					count = word_size1 - word_count1;
					count1++; word_count1 = 0;
					word_count2 += count;
					if (word_count2 == word_size2) {
						count2++; word_count2 = 0;		
					}
				}
				else {                             
					count = word_size2 - word_count2;
					count2++; word_count2 = 0;
					word_count1 += count;
					if (word_count1 == word_size1) {
						count1++; word_count1 = 0;		
					}
				}

				if (count == 1)
					if (b3->active_word == 0)
						b3->active_word = (2 << 30) + 2;
					else if ((b3->active_word >> 30) == 2)
						b3->active_word++;
					else {
						b3->insertWord(b3->active_word);
						b3->active_word = 0;
					}
				break;
			case BitVector::ZERO_ONE:
				//printf("ZERO_ONE\n");
				word_size1 = ((b1.mem_res_vector[count1] << 2) >> 2);
				word_size2 = ((b2.mem_res_vector[count2] << 2) >> 2);
				if ((word_size1 - word_count1) < (word_size2 - word_count2)) {
					count = word_size1 - word_count1;
					count1++; word_count1 = 0;
					word_count2 += count;
					if (word_count2 == word_size2) {
						count2++; word_count2 = 0;		
					}
				}
				else {                             
					count = word_size2 - word_count2;
					count2++; word_count2 = 0;
					word_count1 += count;
					if (word_count1 == word_size1) {
						count1++; word_count1 = 0;		
					}
				}

				if (count == 1)
					if (b3->active_word == 0)
						b3->active_word = (2 << 30) + 2;
					else if ((b3->active_word >> 30) == 2)
						b3->active_word++;
					else {
						b3->insertWord(b3->active_word);
						b3->active_word = 0;
					}
				break;
			default:
				throw BitVectorErr(BitVectorErr::unknown_oper) ;
		}
		//printf("3[%d], %x\n", (int) b3->mem_vector_size, b3->active_word);
		//printf("COUNTS %d, %d\n", count1, count2);
	}
//	printf("DONE ORING\n");
	b3->insertWord(b3->active_word);
	if (count1 == b1.mem_vector_size) {
		for (unsigned i = count2; i < b2.mem_vector_size; i++) {
			if (i == count2 && (b2.mem_res_vector[i] >> 31) == 1)
			b3->insertWord(((1 << 31) + 
						(((b2.mem_res_vector[i] << 2) >> 2) - word_count2)));
			else 
				b3->insertWord(b2.mem_res_vector[i]);
			//printf("1. [%d], %x \n", i, b1.mem_res_vector[i]);
			//printf("3[%d], %x \n", (int)b3->mem_vector_size-1,
				//	b3->mem_res_vector[b3->mem_vector_size-1]);
		}
		b3->active_word_size = b2.active_word_size;
		//printf("%d\n", (int) b3->active_word_size);
	}
	else if (count2 == b2.mem_vector_size) {
		for (unsigned i = count1; i < b1.mem_vector_size; i++) {
			if (i == count1 && (b1.mem_res_vector[i] >> 31) == 1)
				b3->insertWord(((1 << 31) + 
							(((b1.mem_res_vector[i] << 2) >> 2) - word_count1)));
			else
				b3->insertWord(b1.mem_res_vector[i]);
			//printf("1[%d], %x \n", i, b1.mem_res_vector[i]);
			//printf("3[%d], %x \n", (int) b3->mem_vector_size-1,
			//		b3->mem_res_vector[b3->mem_vector_size-1]);
		}
		b3->active_word_size = b1.active_word_size;
		//printf("AWS %d\n", (int) b3->active_word_size);
	}
	//cout << "MVS " << b3->mem_vector_size << endl; 
	//cout << "AWS " << b3->active_word_size << endl; 
	delete(&b1);
	delete(&b2);
	return b3;
}

BitVector *operator & (BitVector &b1, BitVector &b2) {
	BitVector *b3 = new BitVector(1028*1028*100);

	unsigned count1 = 0, count2 = 0, count;
	unsigned word_count1 = 0, word_count2 = 0;
	unsigned word_size1, word_size2, word;

	while (count1 < b1.mem_vector_size && 
			count2 < b2.mem_vector_size) {
		char oper = BitVector::decideOperation(b1.mem_res_vector[count1],
				b2.mem_res_vector[count2]);
		//printf("1[%u] %x, 2[%u] %x, %x, %x, %d\n", count1, b1.mem_res_vector[count1],
			//	count2, b2.mem_res_vector[count2], word_count1, word_count2, oper);
		switch (oper) {
			case BitVector::LITERAL_LITERAL:
				//printf("LITERAL_LITERAL\n");
				b3->insertNextWord(b1.mem_res_vector[count1] &
						b2.mem_res_vector[count2]);
				count1++; word_count1 = 0;
				count2++; word_count2 = 0;
				//printf("WC1 %u, WC2 %u\n", word_count1, word_count2);
				break;
			case BitVector::LITERAL_ONE: 
				//printf("LITERAL_ONE\n");
				b3->insertNextWord(b1.mem_res_vector[count1]);
				count1++; word_count1 = 0;
				word_count2++;
				if (word_count2 == 
						((b2.mem_res_vector[count2] << 2) >> 2)) {
					count2++;
					word_count2 = 0;
				}
				//printf("WC1 %u, WC2 %u\n", word_count1, word_count2);
				break;
			case BitVector::LITERAL_ZERO:
				//printf("LITERAL_ZERO\n");
				b3->insertNextWord(0);
				count1++; word_count1 = 0;
				word_count2++;
				if (word_count2 == 
						((b2.mem_res_vector[count2] << 2) >> 2)) {
					count2++;
					word_count2 = 0;
				}
				//printf("WC1 %u, WC2 %u\n", word_count1, word_count2);
				break;
			case BitVector::ONE_LITERAL:
				//printf("ONE_LITERAL\n");
				b3->insertNextWord(b2.mem_res_vector[count2]);
				count2++; word_count2 = 0;
				word_count1++;
				if (word_count1 == 
						((b1.mem_res_vector[count1] << 2) >> 2)) {
				count1++;
				word_count1 = 0;
				}
				//printf("WC1 %u, WC2 %u\n", word_count1, word_count2);
				break;
			case BitVector::ZERO_LITERAL:
				//printf("ZERO_LITERAL\n");
				b3->insertNextWord(0);
				count2++; word_count2 = 0;
				word_count1++;
				if (word_count1 == 
						((b1.mem_res_vector[count1] << 2) >> 2)) {
					count1++;
					word_count1 = 0;
				}
				//printf("WC1 %u, WC2 %u\n", word_count1, word_count2);
				break;
			case BitVector::ONE_ONE: 
				//printf("ONE_ONE\n");
				word_size1 = ((b1.mem_res_vector[count1] << 2) >> 2);
				word_size2 = ((b2.mem_res_vector[count2] << 2) >> 2);
				if ((word_size1 - word_count1) < (word_size2 - word_count2)) {
				//	printf("1. WS1 %d, WC1 %d, WS2 %d, WC2 %d, Count %d\n", word_size1,
				//			word_size2, word_count1, word_count2, count);
					count = word_size1 - word_count1;
					count1++; word_count1 = 0;
					word_count2 += count;
				//	printf("1. WS1 %d, WC1 %d, WS2 %d, WC2 %d, Count %d\n", word_size1,
				//			word_size2, word_count1, word_count2, count);
					if (word_count2 == word_size2) {
						count2++; word_count2 = 0;		
					}
				}
				else {                             
					count = word_size2 - word_count2;
				//	printf("2. WS1 %d, WC1 %d, WS2 %d, WC2 %d, Count %d\n", word_size1,
				//			word_size2, word_count1, word_count2, count);
					count2++; word_count2 = 0;
					word_count1 += count;
				//	printf("2. WS1 %d, WC1 %d, WS2 %d, WC2 %d, Count %d\n", word_size1,
				//			word_size2, word_count1, word_count2, count);
					if (word_count1 == word_size1) {
						count1++; word_count1 = 0;		
					}
				}
				if (count == 1)
					if (b3->active_word == 0)
						b3->active_word = (2 << 30) + 2;
					else if ((b3->active_word >> 30) == 2)
						b3->active_word++;
					else {
						b3->insertWord(b3->active_word);
						b3->active_word = 0;
					}
				/*if (count == 1)
					word = UINT_MAX >> 1;
				else
					word = (3 << 30) + count;
				b3->insertNextWord(word);*/
				break;
			case BitVector::ZERO_ZERO:
				//printf("ZERO_ZERO\n");
				word_size1 = ((b1.mem_res_vector[count1] << 2) >> 2);
				word_size2 = ((b2.mem_res_vector[count2] << 2) >> 2);
				if ((word_size1 - word_count1) < (word_size2 - word_count2)) {
					count = word_size1 - word_count1;
				//	printf("1. WS1 %d, WC1 %d, WS2 %d, WC2 %d, Count %d\n", word_size1,
				//			word_size2, word_count1, word_count2, count);
					count1++; word_count1 = 0;
					word_count2 += count;
			//printf("1. WS1 %d, WC1 %d, WS2 %d, WC2 %d, Count %d\n", word_size1,
			//		word_size2, word_count1, word_count2, count);
					if (word_count2 == word_size2) {
						count2++; word_count2 = 0;		
					}
				}
				else {                             
					count = word_size2 - word_count2;
			//		printf("2. WS1 %d, WC1 %d, WS2 %d, WC2 %d, Count %d\n", word_size1,
			//				word_size2, word_count1, word_count2, count);
					count2++; word_count2 = 0;
					word_count1 += count;
			//		printf("2. WS1 %d, WC1 %d, WS2 %d, WC2 %d, Count %d\n", word_size1,
			//				word_size2, word_count1, word_count2, count);
					if (word_count1 == word_size1) {
						count1++; word_count1 = 0;		
					}
				}
				if (count == 1)
					word = 0;
				else 
					word = (2 << 30) + count;
				b3->insertNextWord(word);
				/*if (count == 1)
					if (b3->active_word == 0)
						b3->active_word = (2 << 30) + 2;
					else if ((b3->active_word >> 30) == 2)
						b3->active_word++;
					else {
						b3->insertWord(b3->active_word);
						b3->active_word = 0;
					}
				else 
					if (b3->active_word == 0)
						b3->active_word = (2 << 30) + count + 1;
					else if ((b3->active_word >> 30) == 2)
						b3->active_word += count;
					else {
						b3->insertWord(b3->active_word);
						b3->active_word = (2 << 30) + count;
					}*/
				break;
			case BitVector::ZERO_ONE:
			//	printf("ZERO_ONE\n");
				word_size1 = ((b1.mem_res_vector[count1] << 2) >> 2);
				word_size2 = ((b2.mem_res_vector[count2] << 2) >> 2);
				if ((word_size1 - word_count1) < (word_size2 - word_count2)) {
					count = word_size1 - word_count1;
			//		printf("1. WS1 %d, WC1 %d, WS2 %d, WC2 %d, Count %d\n", word_size1,
			//				word_size2, word_count1, word_count2, count);
					count1++; word_count1 = 0;
					word_count2 += count;
			//		printf("1. WS1 %d, WC1 %d, WS2 %d, WC2 %d, Count %d\n", word_size1,
			//				word_size2, word_count1, word_count2, count);
					if (word_count2 == word_size2) {
						count2++; word_count2 = 0;		
					}
				}
				else {                             
					count = word_size2 - word_count2;
			//		printf("2. WS1 %d, WC1 %d, WS2 %d, WC2 %d, Count %d\n", word_size1,
			//				word_size2, word_count1, word_count2, count);
					count2++; word_count2 = 0;
					word_count1 += count;
			//		printf("2. WS1 %d, WC1 %d, WS2 %d, WC2 %d, Count %d\n", word_size1,
			//				word_size2, word_count1, word_count2, count);
					if (word_count1 == word_size1) {
						count1++; word_count1 = 0;		
					}
				}
				if (count == 1)
					word = 0;
				else 
					word = (2 << 30) + count;
				b3->insertNextWord(word);
				break;
			case BitVector::ONE_ZERO:
			//	printf("ONE_ZERO\n");
				word_size1 = ((b1.mem_res_vector[count1] << 2) >> 2);
				word_size2 = ((b2.mem_res_vector[count2] << 2) >> 2);
				if ((word_size1 - word_count1) < (word_size2 - word_count2)) {
					count = word_size1 - word_count1;
			//		printf("1. WS1 %d, WC1 %d, WS2 %d, WC2 %d, Count %d\n", word_size1,
			//				word_size2, word_count1, word_count2, count);
					count1++; word_count1 = 0;
					word_count2 += count;
			//		printf("1. WS1 %d, WC1 %d, WS2 %d, WC2 %d, Count %d\n", word_size1,
			//				word_size2, word_count1, word_count2, count);
					if (word_count2 == word_size2) {
						count2++; word_count2 = 0;		
					}
				}
				else {                             
					count = word_size2 - word_count2;
			//		printf("2. WS1 %d, WC1 %d, WS2 %d, WC2 %d, Count %d\n", word_size1,
			//				word_size2, word_count1, word_count2, count);
					count2++; word_count2 = 0;
					word_count1 += count;
			//		printf("2. WS1 %d, WC1 %d, WS2 %d, WC2 %d, Count %d\n", word_size1,
			//				word_size2, word_count1, word_count2, count);
					if (word_count1 == word_size1) {
						count1++; word_count1 = 0;		
					}
				}
				if (count == 1)
					word = 0;
				else 
					word = (2 << 30) + count;
				b3->insertNextWord(word);
				break;
			default:
				throw BitVectorErr(BitVectorErr::unknown_oper) ;
		}
		//printf("3[%d], %x, AW %x\n", b3->mem_vector_size-1,
		//		b3->mem_res_vector[b3->mem_vector_size-1], b3->active_word);
	}
	//printf("REMOVING\n");
	bool reduced = false;
	while((b3->mem_res_vector[b3->mem_vector_size] >> 30) == 2 ||
			b3->mem_res_vector[b3->mem_vector_size] == 0) {
		//printf("3[%d], %x, AW %x\n", b3->mem_vector_size-1,
			//	b3->mem_res_vector[b3->mem_vector_size-1], b3->active_word);
		reduced = true;
			b3->mem_vector_size--;
	}
	if (reduced)
		b3->mem_vector_size++;

	unsigned aw = b3->mem_res_vector[b3->mem_vector_size-1];
	unsigned one = 1;
	for (unsigned i = 0; i < 31; i++)
		if (((one << i) & aw) == (one << i)) {
			b3->active_word_size = 30 - i;
			break;
		}
	//printf("AWS %d\n", b3->active_word_size);
	//for (size_t i = 0; i < b3->mem_vector_size; i++)
		//printf("%d, %x\n", i, b3->mem_res_vector[i]);
	//delete(b3);
	//b3 = new BitVector(file_name, true);
	return b3;
}

