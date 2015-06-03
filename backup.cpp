#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "md5.hpp"


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

//	md5 m(infile);
//	cout << m.toHex() << "\n";

	md5 mm("grape");
	cout << "md5 of \"grape\": " << mm.toHex() << "\n";


	infile.close();
	

	return 0;
}
