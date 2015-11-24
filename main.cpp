#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>

#include "lib/timing.h"
#include "fast-hash/fasthash.h"

// #define ALT 

#define mix(h) ({					\
			(h) ^= (h) >> 23;		\
			(h) *= 0x2127599bf4325c37ULL;	\
			(h) ^= (h) >> 47; })

const int modulus = 10000000;
const int radix = 101;

typedef struct kmer {
	uint32_t count;
	unsigned char seq[16];
} kmer;

void quicksort_kmer_asc(kmer* a, uint32_t n) {
    if (n < 2) { return; }
    uint32_t p = a[n / 2].count;
    kmer* l = a;
    kmer* r = a + n - 1;
    while (l <= r) {
        if ((*l).count < p) {
            l++;
            continue;
        }
        if ((*r).count > p) {
            r--;
            continue;
        }
        kmer t = *l;
        *l++ = *r;
        *r-- = t;
    }
    quicksort_kmer_asc(a, r - a + 1);
    quicksort_kmer_asc(l, a + n - l);
}

void quicksort_kmer_dsc(kmer* a, uint32_t n) {
    if (n < 2) { return; }
    uint32_t p = a[n / 2].count;
    kmer* l = a;
    kmer* r = a + n - 1;
    while (l <= r) {
        if ((*l).count > p) {
            l++;
            continue;
        }
        if ((*r).count < p) {
            r--;
            continue;
        }
        kmer t = *l;
        *l++ = *r;
        *r-- = t;
    }
    quicksort_kmer_dsc(a, r - a + 1);
    quicksort_kmer_dsc(l, a + n - l);
}

unsigned long modpow(int b, int e, int m) {
	unsigned long num = 1;
	while (e) {
		if (e & 1) {
			num = num * b % m;
		}
		e >>= 1;
		b = (unsigned long)b * b % m;
	}
	return num;
}

int32_t wrap_neg(int32_t i, int32_t i_max) {
   return ((i % i_max) + i_max) % i_max;
}

uint32_t hash_seq(const char* s, int k) {
	uint32_t h = 0;
	for (int i = 0; i < k; ++i) {
		h = (h + (((int)s[i] % modulus) * modpow(radix, (k-1)-i, modulus))) % modulus;
	}
	return h % modulus;
}

uint32_t hash_update(int x, char s0, char sn, int k) {
	int32_t a = x - (int)s0 * modpow(radix, k-1, modulus);
	a = wrap_neg(a, modulus) * radix % modulus;
	return ( a + (int)sn ) % modulus;
}

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
	unsigned char base;
	for (int i = 0; i < n; ++i) {
		if ((base = encode_base(seq[i])) == 0) { return 1; }; // return error if base is not A/T/G/C/N
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

int search(const char* fn, int k, kmer* table, kmer* table1, uint32_t* ncollisions) {
	
	FILE* fp = fopen(fn, "r");
	char* buf = (char*)calloc(1024, sizeof(char));
	unsigned char* enc = (unsigned char*)calloc(16, sizeof(unsigned char));
	int c_lead; // leading char (int incase negative value returned, e.g. EOF)
	int c_trail; // trailing char
	uint32_t x; // hash value
	int linec = 0;
	int charc = 0;
	
	if (fp) {
		while (fgets(buf, 1024, fp)) { // read file line by line, stop at EOF
			if (linec++ % 4 != 1) { // skip lines 1, 3, 4
				continue;
			} 
			
			x = hash_seq(buf, k); // hash kmer
			memset(enc, 0x00, 16);
			if (encode_seq(enc, buf, k)) { return 1; }
			
			c_lead = buf[k];
			c_trail = buf[0];
			charc = 0;
			while (c_lead) {
				charc++;
				if (table[x].count == 0) {
					table[x].count++;
					memcpy(table[x].seq, enc, 16);
				} else if (memcmp(table[x].seq, enc, 16) == 0) {
					table[x].count++;
				} else {
					(*ncollisions)++;
				}
				if (buf[k+charc]) {
					x = hash_update(x, c_trail, c_lead, k); // update hash
					if (encode_update(enc, c_lead, k)) { return 1; }
				}
				c_lead = buf[k+charc];
				c_trail = buf[charc];
			}
		}
		fclose(fp);			
	} else {
		return 1;
	}
	return 0;
}

void benchmark(const char* fn, int k, kmer* table) {
	// int iter = 100;
	// int collisions = 0;
	// timing::start();
	// for (int i = 0; i < iter-1; ++i) {
	// 	rolling_hash(fn, k, table, &collisions);
	// 	quicksort_kmer_dsc(table, modulus);
	// }
	// timing::stop();
	// double t = timing::elapsed_ms_stopped() / iter;
	// // printf("Subsequences of length %d, highest frequency: %d\n", k, freq);
	// printf("Mean time over %d iterations: %gms\n", iter, t);
}

int main(int argc, char** argv) {
	
#ifdef ALT
#else
	
	if (argc < 3) {
		printf("usage: ./run <filename> <k>\n");
		return 0;
	}

	const char* fn = argv[1];
	const int k = atoi(argv[2]);

	FILE* file = fopen(fn, "r");
	if (!file) {
		printf("Could not open file.\n");
		return 0;
	}
	fseek(file, 0L, SEEK_END);
	int bytes = ftell(file);
	printf("Bytes in file: %d\n", bytes);
	fclose(file);

	kmer* table = (kmer*)calloc(modulus, sizeof(kmer));
	kmer* table1 = (kmer*)calloc(modulus, sizeof(kmer));
	uint32_t ncollisions = 0;

	// if (argc > 3) {
	// 	if (strcmp(argv[3], "-b") == 0) {
	// 		benchmark(fn, k, table);
	// 		return 0;
	// 	}
	// }

	timing::start();
	if (search(fn, k, table, table1, &ncollisions)) {
		printf("Error parsing sequence. Make sure sequences are at least k in length.");
		return 0;
	}
	quicksort_kmer_dsc(table, modulus);
	// quicksort_kmer_dsc(table1, modulus);
	timing::stop();
	timing::print_milli();
	// printf("Subsequences of length %d, highest frequency: %d\n", k, freq);

	char* seq = (char*)calloc(k, sizeof(char));
	printf("Table 0:\n");
	for (int i = 0; i < 25; ++i) {
		decode_seq(seq, table[i].seq, k);
		printf("%d : %s\n", table[i].count, seq);
	}
	// printf("Table 1:\n");
	// for (int i = 0; i < 25; ++i) {
	// 	char seq[k];
	// 	memset(seq, 0x00, k);
	// 	decode_seq(table1[i].seq, k, seq);
	// 	printf("%d : %s\n", table1[i].count, seq);
	// }
	printf("Collisions: %d\n", ncollisions);
#endif
	
    return 0;
}
