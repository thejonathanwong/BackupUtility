#include <iostream>
#include <fstream>
#include <stdlib.h>

using namespace std;

int main(int argc, char * argv[]) {

	if(argc != 3) {
		cerr << "USAGE: " << argv[0] << " <Input File> <Output Directory> \n";
		exit(1);
	}


	char * filename = argv[1];
	ifstream infile { filename, ifstream::in | ifstream::binary };
	if(infile == NULL) {
		cerr << "File " << filename << " could not be opened.\n";
		exit(1);
	}

	infile.close();
	

	return 0;
}
