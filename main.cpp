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
	unsigned char seq[8];
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
		
		// kmer t;
		// memcpy(&t, l, sizeof(kmer));
		// memcpy(l, r, sizeof(kmer));
		// memcpy(r, &t, sizeof(kmer));
		// l++;
		// r--;
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

uint32_t update_hash(int x, char s0, char sn, int k) {
	int32_t a = x - (int)s0 * modpow(radix, k-1, modulus);
	a = wrap_neg(a, modulus) * radix % modulus;
	return ( a + (int)sn ) % modulus;
}

uint32_t rabin_std(const char* s, int k) {
	uint32_t h = 0;
	for (int i = 0; i < k; ++i) {
		h = h + (((int)s[i]) * (uint32_t)(pow(radix, (k-1)-i)));
	}
	return h;
}

uint32_t rabin_std(uint32_t x, char s0, char sn, int k) {
	int32_t a = x - (int)s0 * (uint32_t)pow(radix, k-1);
	a = a * radix;
	return ( a + (int)sn );
}

unsigned char encode_base(char b) {
	switch (b) {
		case 'A':
		return 0;
		break;
		case 'T':
		return 1;
		break;
		case 'G':
		return 2;
		break;
		case 'C':
		return 3;
		break;
		default:
		return -1;
	}
}

char decode_base(unsigned char b) {
	switch (b) {
		case 0:
		return 'A';
		break;
		case 1:
		return 'T';
		break;
		case 2:
		return 'G';
		break;
		case 3:
		return 'C';
		break;
		default:
		return -1;
	}
}

int encode_seq(const char* seq, int n, unsigned char* enc) {
	unsigned char base;
	for (int i = 0; i < n; ++i) {
		if ((base = encode_base(seq[i])) < 0) { return 1; }; // return error if base is not A/T/G/C
		base <<= (6-i%4*2); // shift base to place in encoding
		enc[i/4] |= base; // add base to encoding
	}
	return 0;
}

int decode_seq(unsigned char* enc, int n, char* seq) {
	for (int i = 0; i < n/4; ++i) {
		*seq++ = decode_base((enc[i] >> 6) & 0x03); // get base, add to sequence, inc pointer
		*seq++ = decode_base((enc[i] >> 4) & 0x03);
		*seq++ = decode_base((enc[i] >> 2) & 0x03);
		*seq++ = decode_base((enc[i] >> 0) & 0x03);
	}
	for (int i = 0; i < n%4; ++i) {
		*seq++ = decode_base((enc[n/4] >> (6-i*2)) & 3); // same as above for final byte, may not use all 8 bits
	}
	return 0;
}

int update_enc(unsigned char* enc, int n, char b) {
	unsigned char b0 = encode_base(b);
	unsigned char b1;
	int byte = n/4; // last byte used in encoding
	int offset = ((4-n%4)*2)%8; // 'leftwise' offset of last base in encoding
	b1 = enc[byte] & 0xC0; // store bits that will be shifted to next byte
	enc[byte] <<= 2; // shift byte left
	enc[byte] &= ~(0x03 << offset); // zero bits at offset just incase
	enc[byte] |= b0 << offset; // add new base
	for (int i = n/4-1; i >= 0; --i) {
	    b0 = enc[i] & 0xC0; // store bits that will be shifted to next byte
	    enc[i] <<= 2; // shift byte left
	    enc[i] |= b1 >> 6; // add stored bits from previous byte
	    b1 = b0;
	}
	return 0;
}

int rolling_hash(const char* fn, int k, kmer* table, kmer* table1, int* collisions) {
	
	FILE* fp_lead = fopen(fn, "r");
	FILE* fp_trail = fopen(fn, "r");
	char ignore[1024];
	char buf[k];
	unsigned char* encoding = (unsigned char*)calloc(8, sizeof(unsigned char));
	int c_lead; // leading char (int incase negative value returned, e.g. EOF)
	int c_trail; // trailing char
	uint32_t x; // hash value
	fpos_t p;

	if (fp_lead && fp_trail) {
		while (fgets(ignore, sizeof(ignore), fp_lead)) { // ignore line 1 of each chunk, stop at EOF
			/* start of line 2 */
			fgetpos(fp_lead, &p);
			fsetpos(fp_trail, &p);
		
			// assume FASTQ chunk size is at least as large as k
			if (fread(buf, sizeof(char), k, fp_lead) == k) { // advance leading char
				x = hash_seq(buf, k); // hash kmer
				// table[x].count++; // increment kmer count
				
				memset(encoding, 0x00, 8);
				if (encode_seq(buf, k, encoding)) { return 1; }
				
				// uint64_t enc64;
				// memcpy(&enc64, encoding, 8);
				// uint64_t mixed = fasthash64(&encoding, 8, 0);
				// uint64_t x = mixed % modulus;
				
				if (table[x].count == 0) {
				
					table[x].count++;
					memcpy(table[x].seq, encoding, 8);
			
				} else if (memcmp(table[x].seq, encoding, 8) != 0) {
				
					if (table1[x].count == 0) {
						table1[x].count++;
						memcpy(table1[x].seq, encoding, 8);
					} else if (memcmp(table1[x].seq, encoding, 8) != 0) {
						++(*collisions);
					} else {
						table1[x].count++;
					}
									
				} else if (memcmp(table[x].seq, encoding, 8) == 0) {
					table[x].count++;
				}
				
				c_trail = getc(fp_trail); // advance trailing char
				while ((c_lead = getc(fp_lead)) && c_lead != (int)'\n') { // advance leading char
					x = update_hash(x, c_trail, c_lead, k); // update hash
					// table[x].count++; // increment kmer count

					if (update_enc(encoding, k, c_lead)) { return 1; }

					// uint64_t enc64;
					// memcpy(&enc64, encoding, 8);
					// uint64_t mixed = fasthash64(&encoding, 8, 0);
					// uint64_t x = mixed % modulus;

					if (table[x].count == 0) {

						table[x].count++;
						// memset(table[x].seq, 0x00, 8);
						memcpy(table[x].seq, encoding, 8);

					} else if (memcmp(table[x].seq, encoding, 8) != 0) {

						if (table1[x].count == 0) {
							table1[x].count++;
							memcpy(table1[x].seq, encoding, 8);
						} else if (memcmp(table1[x].seq, encoding, 8) != 0) {
							++(*collisions);
						} else {
							table1[x].count++;
						}

					} else if (memcmp(table[x].seq, encoding, 8) == 0) {
						table[x].count++;
					}

					c_trail = getc(fp_trail); // advance trailing char
				}
				/* end of line 2 */
				if (!fgets(ignore, sizeof(ignore), fp_lead)) { break; }; // ignore line 3
				if (!fgets(ignore, sizeof(ignore), fp_lead)) { break; }; // ignore line 4
			}
		}
		
		fclose(fp_lead);
		fclose(fp_trail);
			
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

	// const int n = 9;
// 	char seq[n+1] = { 'A', 'T', 'G', 'C', 'A', 'T', 'G', 'C', 'A' };
// 	seq[n] = '\0';
// 	printf("%s\n", seq);
//
// 	unsigned char* enc = (unsigned char*)calloc(8, sizeof(unsigned char)); // 64 bits can hold a 30 base sequence
// 	encode_seq(seq, n, enc);
//
// 	update_enc(enc, n, 'A');
//
// 	char dec[n+1];
// 	dec[n] = '\0';
// 	decode_seq(enc, n, dec);
//
// 	printf("%s\n", dec);
	
#ifdef ALT
	const int k = 30;
	const int n = 3;
	// char seq[9] = { 'A', 'T', 'G', 'C', 'A', 'T', 'G', 'C', 'A' };
	kmer* a = (kmer*)calloc(n, sizeof(kmer));

	unsigned char* enc = (unsigned char*)calloc(8, sizeof(unsigned char));


	for (unsigned char i = 0; i < n; ++i) {
		char b = decode_base(i);
		const char* seq = "CTCCCCCCGGCTCCCGAGACCCCTCTCATC";
		printf("%s\n", seq);

		memset(enc, 0x00, 8);
		encode_seq(seq, k, enc);

		printf("%d ", (int)enc[0]);
		printf("%d ", (int)enc[1]);
		printf("%d ", (int)enc[2]);
		printf("%d ", (int)enc[3]);
		printf("%d ", (int)enc[4]);
		printf("%d ", (int)enc[5]);
		printf("%d ", (int)enc[6]);
		printf("%d\n", (int)enc[7]);

		memcpy(a[i].seq, enc, 8);
		a[i].count = i;
	}

	quicksort_kmer_dsc(a, n);

	for (int i = 0; i < n; ++i) {
		char seq[k];
		update_enc(a[i].seq, k, 'C');
		decode_seq(a[i].seq, k, seq);
		printf("%d : %s\n", a[i].count, seq);
	}
	
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
	int collisions = 0;

	if (argc > 3) {
		if (strcmp(argv[3], "-b") == 0) {
			benchmark(fn, k, table);
			return 0;
		}
	}

	timing::start();
	if (rolling_hash(fn, k, table, table1, &collisions)) {
		printf("Error parsing sequence. Make sure sequences are at least k in length.");
		return 0;
	}
	quicksort_kmer_dsc(table, modulus);
	quicksort_kmer_dsc(table1, modulus);
	timing::stop();
	timing::print_milli();
	// printf("Subsequences of length %d, highest frequency: %d\n", k, freq);

	printf("Table 0:\n");
	for (int i = 0; i < 25; ++i) {
		char seq[k];
		memset(seq, 0x00, k);
		decode_seq(table[i].seq, k, seq);
		printf("%d : %s\n", table[i].count, seq);
	}
	printf("Table 1:\n");
	for (int i = 0; i < 25; ++i) {
		char seq[k];
		memset(seq, 0x00, k);
		decode_seq(table1[i].seq, k, seq);
		printf("%d : %s\n", table1[i].count, seq);
	}
	printf("Collisions: %d\n", collisions);
#endif
	
    return 0;
}
