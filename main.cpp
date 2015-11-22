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

int cmp_seq(unsigned char* a, unsigned char* b) {
	return (a[0] == b[0] &&
		a[1] == b[1] &&
		a[2] == b[2] &&
		a[3] == b[3] &&
		a[4] == b[4] &&
		a[5] == b[5] &&
		a[6] == b[6] &&
		a[7] == b[7]);
}

uint32_t* kmer_count;
unsigned char* kmer_seq;

int rolling_hash(const char* fn, int k, kmer* table, kmer* table1, int* collisions) {
	
	FILE* fp_lead = fopen(fn, "r");
	FILE* fp_trail = fopen(fn, "r");
	char* ignore = (char*)calloc(1024, sizeof(char));
	char* buf = (char*)calloc(k, sizeof(char));
	unsigned char* encoding = (unsigned char*)calloc(8, sizeof(unsigned char));
	int c_lead; // leading char (int incase negative value returned, e.g. EOF)
	int c_trail; // trailing char
	uint32_t x; // hash value
	fpos_t p;
	
	kmer_count = (uint32_t*)calloc(modulus, sizeof(uint32_t));
	kmer_seq = (unsigned char*)calloc(modulus * 8, sizeof(unsigned char));
	
	char* seq = (char*)calloc(k, sizeof(char));
	// for (int i = 0; i < 25; ++i) {
	// 	memset(seq, 0x00, k);
	// 	decode_seq(&kmer_seq[i*8], k, seq);
	// 	printf("%d : %s\n", kmer_count[i], seq);
	// }

	if (fp_lead && fp_trail) {
		while (fgets(ignore, 1024, fp_lead)) { // ignore line 1 of each chunk, stop at EOF
			/* start of line 2 */
			fgetpos(fp_lead, &p);
			fsetpos(fp_trail, &p);
			
			printf("ignore: %s\n", ignore);
		
			// assume FASTQ chunk size is at least as large as k
			if (fread(buf, sizeof(char), k, fp_lead) == k) { // advance leading char
				x = hash_seq(buf, k); // hash kmer
				
				memset(encoding, 0x00, 8);
				if (encode_seq(buf, k, encoding)) { return 1; }
				
				// memset(seq, 0x00, k);
// 				decode_seq(encoding, k, seq);
				printf("%d : %s\n", kmer_count[x], buf);

				c_trail = getc(fp_trail); // advance trailing char
				while ((c_lead = getc(fp_lead)) && c_lead != (int)'\n') { // advance leading char
					
					if (kmer_count[x] == 0) {

						kmer_count[x]++;
						// memset(table[x].seq, 0x00, 8);
						
						kmer_seq[x*8] = encoding[0];
						kmer_seq[x*8+1] = encoding[1];
						kmer_seq[x*8+2] = encoding[2];
						kmer_seq[x*8+3] = encoding[3];
						kmer_seq[x*8+4] = encoding[4];
						kmer_seq[x*8+5] = encoding[5];
						kmer_seq[x*8+6] = encoding[6];
						kmer_seq[x*8+7] = encoding[7];
						
						// memcpy(kmer_seq[x], encoding, 8);

					} else if (!cmp_seq(&kmer_seq[x*8], encoding)) {
						++(*collisions);
					} else if (cmp_seq(&kmer_seq[x*8], encoding)) {
						kmer_count[x]++;
					} else {
						printf("something wierd\n");
					}
					
					// memset(seq, 0x00, k);
// 					decode_seq(encoding, k, seq);
// 					printf("%d : %s\n", kmer_count[x], seq);
					
					x = update_hash(x, c_trail, c_lead, k); // update hash
					if (update_enc(encoding, k, c_lead)) { return 1; }

					c_trail = getc(fp_trail); // advance trailing char
				}
				/* end of line 2 */
				if (!fgets(ignore, 1024, fp_lead)) { break; }; // ignore line 3
				if (!fgets(ignore, 1024, fp_lead)) { break; }; // ignore line 4
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

	// if (argc > 3) {
	// 	if (strcmp(argv[3], "-b") == 0) {
	// 		benchmark(fn, k, table);
	// 		return 0;
	// 	}
	// }

	timing::start();
	if (rolling_hash(fn, k, table, table1, &collisions)) {
		printf("Error parsing sequence. Make sure sequences are at least k in length.");
		return 0;
	}
	// quicksort_kmer_dsc(table, modulus);
	// quicksort_kmer_dsc(table1, modulus);
	timing::stop();
	timing::print_milli();
	// printf("Subsequences of length %d, highest frequency: %d\n", k, freq);

	char* seq = (char*)calloc(k, sizeof(char));
	printf("Table 0:\n");
	for (int i = 0; i < 25; ++i) {
		memset(seq, 0x00, k);
		decode_seq(&kmer_seq[i*8], k, seq);
		printf("%d : %s\n", kmer_count[i], seq);
	}
	// printf("Table 1:\n");
	// for (int i = 0; i < 25; ++i) {
	// 	char seq[k];
	// 	memset(seq, 0x00, k);
	// 	decode_seq(table1[i].seq, k, seq);
	// 	printf("%d : %s\n", table1[i].count, seq);
	// }
	printf("Collisions: %d\n", collisions);
#endif
	
    return 0;
}
