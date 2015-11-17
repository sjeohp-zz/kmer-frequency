#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "lib/timing.h"

unsigned long modpow(int b, int e, int m) {
	unsigned long number = 1;
	while (e) {
		if (e & 1) {
			number = number * b % m;
		}
		e >>= 1;
		b = (unsigned long)b * b % m;
	}
	return number;
}

int32_t wrap_neg(int32_t i, int32_t i_max) {
   return ((i % i_max) + i_max) % i_max;
}

const int modulus = 611939;
const int radix = 101;

uint32_t rabin_mod(const char* s, int k) {
	uint32_t h = 0;
	for (int i = 0; i < k; ++i) {
		h = (h + (((int)s[i] % modulus) * modpow(radix, (k-1)-i, modulus))) % modulus;
	}
	return h % modulus;
}

uint32_t rabin_mod(int x, char s0, char sn, int k) {
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

int rolling_hash(const char* fn, int k) {
	int* table = (int*)calloc(modulus, sizeof(int));	
	char buf[k];
	int c;
	FILE* file_a = fopen(fn, "r");
	FILE* file_b = fopen(fn, "r");
	if (file_a && file_b) {
	    if (fread(buf, 1, sizeof(buf), file_a) == k) {
			uint32_t x = rabin_mod(buf, k);
			char c0 = getc(file_b);
			table[x]++;			
			while ((c = getc(file_a)) != EOF) {
				x = rabin_mod(x, c0, (char)c, k);
				// printf("%d\n", x);
				c0 = getc(file_b);
				table[x]++;
			}
		} else {
			printf("Error: file length in bytes is shorter than k.");
			return 0;
		}
	    fclose(file_a);
		fclose(file_b);
	}
	int curr = 0;
	for (int i = 0; i < modulus; ++i) {
		if (table[i] > curr) {
			curr = table[i];
		}
	}
	return curr;
}

void benchmark(const char* fn, int k) {
	int iter = 100;
	timing::start();
	for (int i = 0; i < iter-1; ++i) {
		rolling_hash(fn, k);
	}
	int freq = rolling_hash(fn, k);
	timing::stop();
	double t = timing::elapsed_ms_stopped() / iter;
	printf("Subsequences of length %d, highest frequency: %d\n", k, freq);
	printf("Mean time over %d iterations: %gms\n", iter, t);
}

int main(int argc, char** argv) {
	
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
	
	if (argc > 3) {
		if (strcmp(argv[3], "-b") == 0) {
			benchmark(fn, k);
			return 0;
		}
	}
	
	timing::start();
	int freq = rolling_hash(fn, k);
	timing::stop();
	printf("Subsequences of length %d, highest frequency: %d\n", k, freq);
	timing::print_milli();

    return 0;
}
