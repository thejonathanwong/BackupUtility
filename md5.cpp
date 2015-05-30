#include "md5.hpp"
#include <cstring>
#include <iostream>


md5::md5() {
	init();
}

md5::md5(ifstream& infile) {
	init();
	readFile(infile);
}

void md5::init() {
	//Initialize variables:
	a0 = 0x67452301;
	b0 = 0xefcdab89;
	c0 = 0x98badcfe;
	d0 = 0x10325476;
}

//vector<Block> md5::readFile(ifstream& infile) {
void md5::readFile(ifstream& infile) {
	vector<Block> v;

	Block b;
	while (infile) {
		memset(b.block, 0, sizeof(b.block));
		infile.read(b.block, BLOCK);
		v.push_back(b);
	}

//	for( Block bb : v ) {
//		cout << bb.block << "\n";
//	}

	//modify last 64 byte chunk
	b = v.back();



	for(Block chunk : v) {
		
	}


//	return v;

	
}


