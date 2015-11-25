#include <string.h>
#include "encoding.h"

unsigned char encode_base(char b) {
	switch (b) {
		case 'A':
		return 0x01;
		case 'T':
		return 0x02;
		case 'G':
		return 0x03;
		case 'C':
		return 0x04;
		case 'W':
		return 0x05;
		case 'S':
		return 0x06;
		case 'M':
		return 0x07;
		case 'K':
		return 0x08;
		case 'R':
		return 0x09;
		case 'Y':
		return 0x0A;
		case 'B':
		return 0x0B;
		case 'D':
		return 0x0C;
		case 'H':
		return 0x0D;
		case 'V':
		return 0x0E;
		case 'N':
		return 0x0F;
		default:
		return 0x00;
	}
}

char decode_base(unsigned char b) {
	switch (b) {
		case 0x01:
		return 'A';
		case 0x02:
		return 'T';
		case 0x03:
		return 'G';
		case 0x04:
		return 'C';
		case 0x05:
		return 'W';
		case 0x06:
		return 'S';
		case 0x07:
		return 'M';
		case 0x08:
		return 'K';
		case 0x09:
		return 'R';
		case 0x0A:
		return 'Y';
		case 0x0B:
		return 'B';
		case 0x0C:
		return 'D';
		case 0x0D:
		return 'H';
		case 0x0E:
		return 'V';
		case 0x0F:
		return 'N';
		default:
		return 0x00;
	}
}

int encode_seq(unsigned char* enc, const char* seq, int n, int* err_c, int* err_i) {
	memset(enc, 0x00, 16);
	unsigned char base;
	for (int i = 0; i < n; ++i) {
		if ((base = encode_base(seq[i])) == 0x00) { 
			if (err_c && err_i) {
				*err_c = seq[i];
				*err_i = i;
			}
			return 1; 
		} // return error if base is not valid
		base <<= 4-i%2*4; // shift base to place in encoding
		enc[i/2] |= base; // add base to encoding
	}
	return 0;
}

int decode_seq(char* seq, unsigned char* enc, int n) {
	for (int i = 0; i < n/2; ++i) {
		*seq++ = decode_base((enc[i] >> 4) & 0x0F); // get base, add to sequence, inc pointer
		*seq++ = decode_base((enc[i] >> 0) & 0x0F);
	}
	for (int i = 0; i < n%4; ++i) {
		*seq++ = decode_base((enc[n/2] >> 4) & 0x0F); // same as above for final byte, may not use all 8 bits
	}
	return 0;
}

int encode_update(unsigned char* enc, char b, int n) {
	unsigned char b0;
	if ((b0 = encode_base(b)) == 0x00) { return 1; }
	unsigned char b1;
	int byte = n/2-1; // last byte used in encoding
	int offset = n%2*4; // 'leftwise' offset of last base in encoding
	b1 = enc[byte] & 0xF0; // store bits that will be shifted to next byte
	enc[byte] <<= 4; // shift byte left
	enc[byte] &= ~(0x0F << offset); // zero bits at offset just incase
	enc[byte] |= b0 << offset; // add new base
	for (int i = n/2-2; i >= 0; --i) {
	    b0 = enc[i] & 0xF0; // store bits that will be shifted to next byte
	    enc[i] <<= 4; // shift byte left
	    enc[i] |= b1 >> 4; // add stored bits from previous byte
	    b1 = b0;
	}
	return 0;
}