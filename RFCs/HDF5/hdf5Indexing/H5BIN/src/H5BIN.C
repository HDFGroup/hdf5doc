#include <H5BIN.h>


int main (int argc, char **argv) {
unsigned short lb, ub;
	H5BIN<unsigned short> *index = new H5BIN<unsigned short>(argv[2]);
	if (argv[1][0] == 'c') {
		index->create();
	}
	else {
		lb = atoi(argv[3]);
		ub = atoi(argv[4]);
		string attribute = "LST_Day";
		BitVector *bv = index->query(&lb, &ub, &attribute, 1);
		bv->openIterator();
		while (!bv->endIterator()) {
			printf("%d\n", bv->getNext());
		}
		bv->closeIterator();
	}
}
