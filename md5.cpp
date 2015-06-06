#include "md5.hpp"
#include <unistd.h>

constexpr unsigned int md5::S[];
constexpr unsigned int md5::K[];
constexpr unsigned char md5::padding[64];

md5::md5() {
	init();
}

md5::md5(ifstream& infile) {
	init();
	readFile(infile);
}

md5::md5(const string& s) {
	init();
	readString(s);
}

void md5::init() {
	//Initialize variables:
	a0 = 0x67452301;
	b0 = 0xefcdab89;
	c0 = 0x98badcfe;
	d0 = 0x10325476;

	totalBits = 0;
}

void md5::hash(ifstream& infile) {
	md5::init();
	md5::readFile(infile);
}

void md5::hash(int infile) {
	md5::init();
	md5::readFile(infile);
}

void md5::hash(string s) {
	md5::init();
	md5::readString(s);
}

void md5::encodeBits(unsigned char * chunk, size_t length) {
	unsigned char bits[BITS]= {0};
	for(unsigned int i = 0; i < BITS; ++i) {
//		chunk[length - BITS] = (totalBits >> 8*i) & 0xff;
		bits[i] = (totalBits >> 8*i) & 0xff;
	}
	memcpy(&chunk[length-BITS], bits, BITS);
//	for(int i = 0; i < 8; ++i) {
//		for(int j = 0; j < 8; ++j) {
//			std::cout << ((chunk[length-BITS+i] >> j) & 1);
//		}
//	}
//	std::cout << "\n";
}

void md5::readString(string s) {
	unsigned int buffersize;
	unsigned int length = s.length();
	unsigned int index = length % BLOCK;

	totalBits = length << 3;


	unsigned int padLen;
	if(index < 56) {
		padLen = 56 - index;
		buffersize = BLOCK * (length/BLOCK + 1);
	} else {
		padLen = 120 - index;
		buffersize = BLOCK * (length/BLOCK + 2);
	}
		
	unsigned char chunk[buffersize] = {0};

//	cout << "buffersize: " <<  buffersize << "\n";


	memcpy(chunk, s.c_str(), length);
	memcpy(&chunk[length], padding, padLen);
	encodeBits(chunk, buffersize);

	stateUpdate(chunk, buffersize);

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
			totalBits = (totalBytes << 3); //file size in bits

//			//calculates the total number of bits of the file
//			unsigned char bits[BITS];
////			unsigned long long totalBits = (totalBytes << 3); //file size in bits
//			totalBits = (totalBytes << 3); //file size in bits
//			for(unsigned int i = 0; i < BITS; ++i) {
//				bits[i] = (totalBits >> 8*i) & 0xff;
//			}
//
//	for(int i = 0; i < 8; ++i) {
//		for(int j = 0; j < 8; ++j) {
//			std::cout << ((bits[i] >> j) & 1);
//		}
//	}
//	std::cout << "\n";

			unsigned int index = bytesRead % BLOCK;
			unsigned int padLen;
			char overflow[BLOCK] = {0};

			//handles padding
			if(bytesRead < BUFFER - 8){ //if padding will not overfill the BUFFER
				padLen = (index < 56) ? (56 - index) : (120 - index);

				//appends 1 and pads withs 0s, then
				//inserts file size in bits as last 64 bits of message
				memcpy(&chunk[bytesRead], padding, padLen);
//				memcpy(&chunk[bytesRead + padLen], bits, BITS);
				encodeBits((unsigned char *) chunk, bytesRead+padLen+BITS);
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
//				memcpy(&overflow[BLOCK - 8], bits, BITS);
				encodeBits((unsigned char *) chunk, bytesRead+padLen+BITS);

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
//	cout << "totalBytes = " << totalBytes << "\n";
}

void md5::readFile(int infile) {
	
	char chunk[BUFFER] = {0};
	
	unsigned long long totalBytes = 0;
	unsigned int bytesRead = 0;

//	while (infile) {
//		infile.read(chunk, BUFFER);
//	while( !feof(infile) ) {
//		bytesRead = fread(chunk, 1, BUFFER, infile);
	while((bytesRead = read(infile, chunk, BUFFER)) > 0) {

//		bytesRead = infile.gcount();
		totalBytes += bytesRead;

		if(bytesRead == BUFFER) { //if there are still things to be read
			stateUpdate(chunk, bytesRead);
		} else { //handles the last chunk of a file
			totalBits = (totalBytes << 3); //file size in bits

			unsigned int index = bytesRead % BLOCK;
			unsigned int padLen;
			char overflow[BLOCK] = {0};

			//handles padding
			if(bytesRead < BUFFER - 8){ //if padding will not overfill the BUFFER
				padLen = (index < 56) ? (56 - index) : (120 - index);

				//appends 1 and pads withs 0s, then
				//inserts file size in bits as last 64 bits of message
				memcpy(&chunk[bytesRead], padding, padLen);
				encodeBits((unsigned char *) chunk, bytesRead+padLen+BITS);
				stateUpdate(chunk, bytesRead + padLen + BITS);


			} else { //if padding will overfill the BUFFER, overflow buffer is necessary

				padLen = (index < 64) ? (64 - index) : (120 - index);
				if ( padLen < 8 ) {

					//if there is space in the chunks BUFFER, put padding there.
					memcpy(&chunk[bytesRead], padding, padLen);
				} else {
					memcpy(overflow, padding, padLen);
				}
				
				//sets last 64 bits / 8 bytes of overflow
				encodeBits((unsigned char *) chunk, bytesRead+padLen+BITS);

				stateUpdate(chunk, BUFFER);
				stateUpdate(overflow, BLOCK);

			}
		}
	}
	
}

void md5::stateUpdate(const unsigned char * chunk, size_t length) {
	if((length % BLOCK) != 0) {
		cerr << "stateUpdate: buffer length not multiple of 512 bits\n";
		cerr << length;
		exit(1);
	} 

	size_t iterations = length/BLOCK;

	//for each 512 bit chunk of the message, loop
	for(size_t i = 0; i < iterations; ++i) {
		uint32_t M[16];
		uint32_t z = 0;
		for(uint32_t index = 0; index < 16; ++index) {
			M[index] = ((uint32_t) chunk[i*BLOCK + z]) | (((uint32_t)chunk[i*BLOCK+z+1]) << 8) |
				 (((uint32_t)chunk[i*BLOCK+z+2]) << 16) | (((uint32_t)chunk[i*BLOCK+z+3]) << 24);
			z += 4;
		}
//		cout << "\n";

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
				g = j;

			} else if ( j < 32) {
				F = (D & B) | ((~D) & C);
				g = (5*j + 1) % 16;

			} else if ( j < 48 ) {
				F = B ^ C ^ D;
				g = (3*j + 5) % 16;
				
			} else if (j < 64) {
				F = C ^ (B | (~D));
				g = (7*j) % 16;
			}
//				cout << M[g] << endl;
			dTemp = D;
			D = C;
			C = B;
//			B = B + leftRotate( (A + F + K[j] + chunk[i*BLOCK + g]), S[j]);
			B = B + leftRotate( (A + F + K[j] + M[g]), S[j]);
			A = dTemp;
		}
		a0 += A;
		b0 += B;
		c0 += C;
		d0 += D;

//		cout << a0 << " " << b0 << " " << c0 << " " << d0 << "\n";
		
	}
//	hash[0] = (a0 >> 24) & 0xff;
//	hash[1] = (a0 >> 16) & 0xff;
//	hash[2] = (a0 >> 8) & 0xff;
//	hash[3] = (a0) & 0xff;
//
//	hash[4] = (b0 >> 24) & 0xff;
//	hash[5] = (b0 >> 16) & 0xff;
//	hash[6] = (b0 >> 8) & 0xff;
//	hash[7] = (b0) & 0xff;
//
//	hash[8]  = (c0 >> 24) & 0xff;
//	hash[9]  = (c0 >> 16) & 0xff;
//	hash[10] = (c0 >> 8) & 0xff;
//	hash[11] = (c0) & 0xff;
//
//	hash[12] = (d0 >> 24) & 0xff;
//	hash[13] = (d0 >> 16) & 0xff;
//	hash[14] = (d0 >> 8) & 0xff;
//	hash[15] = (d0) & 0xff;

	hashed[0] = (a0 ) & 0xff;
	hashed[1] = (a0 >> 8) & 0xff;
	hashed[2] = (a0 >> 16) & 0xff;
	hashed[3] = (a0 >> 24) & 0xff;

	hashed[4] = (b0) & 0xff;
	hashed[5] = (b0 >> 8) & 0xff;
	hashed[6] = (b0 >> 16) & 0xff;
	hashed[7] = (b0 >> 24) & 0xff;

	hashed[8]  = (c0) & 0xff;
	hashed[9]  = (c0 >> 8) & 0xff;
	hashed[10] = (c0 >> 16) & 0xff;
	hashed[11] = (c0 >> 24) & 0xff;

	hashed[12] = (d0) & 0xff;
	hashed[13] = (d0 >> 8) & 0xff;
	hashed[14] = (d0 >> 16) & 0xff;
	hashed[15] = (d0 >> 24) & 0xff;
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
    sprintf(buf+i*2, "%02x", hashed[i]);
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





