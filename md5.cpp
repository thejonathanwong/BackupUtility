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

void md5::readFile(ifstream& infile) {

	char chunk[BUFFER] = {0};
	
	unsigned long long totalBytes = 0;
	unsigned int bytesRead = 0;

	while (infile) {
		infile.read(chunk, BUFFER);

		if(infile) { //if there are still things to be read
			stateUpdate(chunk);
		} else { //handles the last chunk of a file

			//calculates the total number of bits of the file
			unsigned char bits[8];
			unsigned long long totalBits = (totalBytes << 3); //file size in bits
			for(unsigned int i = 0; i < sizeof(bits); ++i) {
				bits[i] = (totalBits >> 8*i) & 0xff;
			}
			
		}
	}
}

//vector<Block> md5::readFile(ifstream& infile) {
void md5::readFile2(ifstream& infile) {
//	vector<Block> v;

//	Block b;
	char chunk[BUFFER] = {0};
	char overflow[BLOCK] = {0};
	bool overfilled = false;
	unsigned long long totalBytes = 0;
	unsigned int bRead = 0;
	while (infile) {
//		memset(b.block, 0, sizeof(b.block));
//		infile.read(b.block, BLOCK);
//		v.push_back(b);
		memset(chunk, 0, sizeof(chunk));
		infile.read(chunk, BUFFER);

		bRead = infile.gcount();
		totalBytes += bRead;

		//handles the last chunk
//		if(bRead != BUFFER) {
		if(!infile){
//			unsigned int padLen = (length < 56) ? 56 - length : 120 - length; 

			//calculates the total number of bits of the file
			unsigned char bits[8];
			unsigned long long totalBits = totalBytes << 3;
			for(unsigned int i = 0; i < sizeof(bits); ++i) {
				bits[i] = (totalBits >> 8*i) & 0xff;
				//					for(int j = 0; j < 8; ++j) {
				//						cout << ((bits[i] >> j) &1);
				//					}
			}
			//				cout << "\n";


			//handles padding
			unsigned int padLen;
			unsigned int length = (bRead % BLOCK);
			if( (bRead != 0) && (bRead < (BUFFER - BLOCK - 8)) ) { //if padding will not cause buffer to overflow

				padLen = (length < 56) ? (56 - length) : (120 - length);

				memcpy(&chunk[bRead], padding, padLen);
				memcpy(&chunk[bRead + padLen], bits, sizeof(bits));

			} else { //if padding will go beyond the chunk's buffer size of BLOCK originally 1024
				cout << "hello\n";
				if (length < 56) {

					//appends 1 and pads with 0s
					padLen = 56 - length;
					memcpy(&chunk[bRead], padding, padLen);

					//sets last 8 bytes / 64 bits to file size 
					memcpy(&chunk[bRead + padLen], bits, sizeof(bits));

				} else { //overflow buffer is needed 


					if ( length < 64 ) { // fills in padding for chunk if needed

						padLen = 64 - length;
						memcpy(&chunk[bRead], padding, padLen);

					} else { // sets first bit of overflow to 1 since chunk is filled by the file
						memcpy(overflow, padding, sizeof(padding));
					}

					//sets last 8 bytes to file size
					memcpy(&overflow[BLOCK - sizeof(bits)], bits, sizeof(bits));
					overfilled = true;
				}
			}
		}

		stateUpdate(chunk);
	}

	if (overfilled) {
		cout << "overfilled\n";
		stateUpdate(overflow);
	}


	cout << totalBytes << "\n";
	cout << sizeof(unsigned long long) << "\n";

	//	for( Block bb : v ) {
	//		cout << bb.block << "\n";
	//	}

	//modify last 64 byte chunk
	//	b = v.back();
	//
	//
	//
	//	for(Block chunk : v) {
	//		
	//	}


	//	return v;


}

void md5::stateUpdate(const unsigned char * chunk) {

}

void md5::stateUpdate(const char chunk[]) {
	md5::stateUpdate((const unsigned char *) chunk);	
}

