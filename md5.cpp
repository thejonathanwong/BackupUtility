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

		bytesRead = infile.gcount();
		totalBytes += bytesRead;

		if(infile) { //if there are still things to be read
			stateUpdate(chunk, bytesRead);
//			cout << bytesRead << "\n";
		} else { //handles the last chunk of a file

			//calculates the total number of bits of the file
			unsigned char bits[BITS];
			unsigned long long totalBits = (totalBytes << 3); //file size in bits
			for(unsigned int i = 0; i < BITS; ++i) {
				bits[i] = (totalBits >> 8*i) & 0xff;
			}

			unsigned int index = bytesRead % BLOCK;
			unsigned int padLen;
			char overflow[BLOCK] = {0};

			//handles padding
			if(bytesRead < BUFFER - 8){ //if padding will not overfill the BUFFER
				padLen = (index < 56) ? (56 - index) : (120 - index);

				//appends 1 and pads withs 0s, then
				//inserts file size in bits as last 64 bits of message
				memcpy(&chunk[bytesRead], padding, padLen);
				memcpy(&chunk[bytesRead + padLen], bits, BITS);
				stateUpdate(chunk, bytesRead + padLen + BITS);

//				cout << bytesRead + padLen + BITS << "\n";

			} else { //if padding will overfill the BUFFER, overflow buffer is necessary

				padLen = (index < 64) ? (64 - index) : (120 - index);
				if ( padLen < 8 ) {

					//if there is space in the chunks BUFFER, put padding there.
					memcpy(&chunk[bytesRead], padding, padLen);
				} else {
					memcpy(overflow, padding, padLen);
				}
				
				//sets last 64 bits / 8 bytes of overflow
				memcpy(&overflow[BLOCK - 8], bits, BITS);

				stateUpdate(chunk, BUFFER);
				stateUpdate(overflow, BLOCK);


				//debugging
//				cout << "overflow!\n";
//				cout << bytesRead + padLen << "\t" << BUFFER << "\n";
//				cout << chunk << "\n";
//				for(int i = 0; i < 8; ++i) {
//					for(int j = 0; j < 8; ++j) {
//						cout << ((unsigned char)(overflow[56+i] >> j) & 1);
//					}
//				}
//				cout << "\n";

			}
		}
	}
	cout << "totalBytes = " << totalBytes << "\n";
}

void md5::stateUpdate(const unsigned char * chunk, size_t length) {
	if((length % BLOCK) != 0) {
		cerr << "stateUpdate: buffer length not multiple of 512 bits\n";
		exit(1);
	} 

	size_t iterations = length/BLOCK;

	//for each 512 bit chunk of the message, loop
	for(size_t i = 0; i < iterations; ++i) {
		uint32_t A = a0;
		uint32_t B = b0;
		uint32_t C = c0;
		uint32_t D = d0;

		uint32_t F = 0;
		uint32_t g = 0;
		uint32_t dTemp = 0;

		//main loop
		for(int j = 0; j < BLOCK; ++j) {
			
			if ( j < 16  ) {
				F = (B & C) | ((~B) & D);
				g = i;

			} else if ( j < 32) {
				F = (D & B) | ((~D) & C);
				g = (5*i + 1) % 16;

			} else if ( j < 48 ) {
				F = B ^ C ^ D;
				g = (3*i + 5) % 16;
				
			} else if (j < 64) {
				F = C ^ (B | (~D));
				g = (7*i) % 16;
			}
			dTemp = D;
			D = C;
			C = B;
			B = B + leftRotate( (A + F + K[j] + chunk[i*BLOCK + g]), S[j]);
			A = dTemp;
		}
		a0 += A;
		b0 += B;
		c0 += C;
		d0 += D;
		
	}
	hash[0] = (a0 >> 24) & 0xff;
	hash[1] = (a0 >> 16) & 0xff;
	hash[2] = (a0 >> 8) & 0xff;
	hash[3] = (a0) & 0xff;

	hash[4] = (b0 >> 24) & 0xff;
	hash[5] = (b0 >> 16) & 0xff;
	hash[6] = (b0 >> 8) & 0xff;
	hash[7] = (b0) & 0xff;

	hash[8]  = (c0 >> 24) & 0xff;
	hash[9]  = (c0 >> 16) & 0xff;
	hash[10] = (c0 >> 8) & 0xff;
	hash[11] = (c0) & 0xff;

	hash[12] = (d0 >> 24) & 0xff;
	hash[13] = (d0 >> 16) & 0xff;
	hash[14] = (d0 >> 8) & 0xff;
	hash[15] = (d0) & 0xff;

	/*
    for i from 0 to 63
        if 0 ≤ i ≤ 15 then
            F := (B and C) or ((not B) and D)
            g := i
        else if 16 ≤ i ≤ 31
            F := (D and B) or ((not D) and C)
            g := (5×i + 1) mod 16
        else if 32 ≤ i ≤ 47
            F := B xor C xor D
            g := (3×i + 5) mod 16
        else if 48 ≤ i ≤ 63
            F := C xor (B or (not D))
            g := (7×i) mod 16
        dTemp := D
        D := C
        C := B
        B := B + leftrotate((A + F + K[i] + M[g]), s[i])
        A := dTemp
    end for
//Add this chunk's hash to result so far:
    a0 := a0 + A
    b0 := b0 + B
    c0 := c0 + C
    d0 := d0 + D
	*/
}

string md5::toHex() {
  char buf[33];

  for (int i=0; i<16; i++) {
    sprintf(buf+i*2, "%02x", hash[i]);
  }

  buf[32]=0;
 
  return std::string(buf);	
}

void md5::stateUpdate(const char chunk[], size_t length) {
	md5::stateUpdate((const unsigned char *) chunk, length);	
}

unsigned int inline md5::leftRotate(unsigned int x, unsigned int c) {
	return ((x << c) | (x >> (32-c)));
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
			for(unsigned int i = 0; i < BITS; ++i) {
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
				memcpy(&chunk[bRead + padLen], bits, BITS);

			} else { //if padding will go beyond the chunk's buffer size of BLOCK originally 1024
				cout << "hello\n";
				if (length < 56) {

					//appends 1 and pads with 0s
					padLen = 56 - length;
					memcpy(&chunk[bRead], padding, padLen);

					//sets last 8 bytes / 64 bits to file size 
					memcpy(&chunk[bRead + padLen], bits, BITS);

				} else { //overflow buffer is needed 


					if ( length < 64 ) { // fills in padding for chunk if needed

						padLen = 64 - length;
						memcpy(&chunk[bRead], padding, padLen);

					} else { // sets first bit of overflow to 1 since chunk is filled by the file
						memcpy(overflow, padding, sizeof(padding));
					}

					//sets last 8 bytes to file size
					memcpy(&overflow[BLOCK - BITS], bits, BITS);
					overfilled = true;
				}
			}
		}

		stateUpdate(chunk, bRead);
	}

	if (overfilled) {
		cout << "overfilled\n";
		stateUpdate(overflow, BLOCK);
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



