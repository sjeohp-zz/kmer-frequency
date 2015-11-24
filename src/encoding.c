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
		case 'N':
		return 0x05;
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
		return 'N';
		default:
		return 0x00;
	}
}

int encode_seq(unsigned char* enc, const char* seq, int n) {
	memset(enc, 0x00, 16);
	unsigned char base;
	for (int i = 0; i < n; ++i) {
		if ((base = encode_base(seq[i])) == 0) { return 1; }; // return error if base is not valid
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

int encode_update(unsigned char* enc, char b,  int n) {
	unsigned char b0 = encode_base(b);
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