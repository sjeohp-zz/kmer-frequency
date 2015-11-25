#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>

#include "../lib/fasthash/fasthash.h"

#include "timing.h"
#include "encoding.h"
#include "table.h"
#include "search.h"

/* DIVSIZE should be a prime number suitable for a hash table size */
// #define DIVSIZE 6291469
#define NDIVS 4

uint32_t hash_size[] = {
	6291469,
	12582917,
	25165843,
	50331653,
	100663319,
	201326611
};

int main(int argc, char** argv) {

	if (argc < 3) {
		printf("\nUsage: ./run <filename> <k>\n\n");
		return 0;
	}

	const char* fn = argv[1];
	const int k = atoi(argv[2]);
	
	if (k <= 0 || k > 32) {
		printf("\nPlease ensure: 0 < k < 33\n\n");
		return 1;
	}
	
	FILE* fp = fopen(fn, "r");
	if (!fp) {
		printf("Could not open file.\n");
		return 0;
	}
	fseek(fp, 0L, SEEK_END);
	uint64_t filesize = ftell(fp) / 1000000L;
	printf("\nFile size:\t%llu mb\n", filesize);
	fseek(fp, 0, SEEK_SET);
	
	int divsize;
#ifdef DIVSIZE
	divsize = DIVSIZE;
#else
	divsize = hash_size[5];
	for (int i = 5; i >= 0; --i) {
		if (hash_size[i]*NDIVS/1000000L > filesize/4) {
			divsize = hash_size[i];
		}
	}
#endif
	
	printf("Table size:\t%lu mb\n", NDIVS*divsize*sizeof(kmer)/1000000L);
	
	printf("\nTallying k-mers...\n");
	
	time_start();

	kmer* table;
	table_init(&table, divsize, NDIVS);
	
	uint64_t ncollisions = 0;
	uint64_t nunresolved = 0;
	
	parse_error err;
	int err_n;
	if (!(err_n = search(fp, k, table, divsize, NDIVS, &ncollisions, &nunresolved, &err))) {
		
		table_sort_dsc(table, NDIVS*divsize);
		time_stop();
	
		fclose(fp);
	
		printf("\nCollisions:\t%llu\n", ncollisions);
		printf("Unresolved:\t%llu\n\n", nunresolved);
	
		printf("Elapsed time:\t%.2Lf ms\n", time_elapsed() / 1000.0L);

		char* seq = (char*)calloc(k, sizeof(char));
		printf("\nFrequency\tSequence\n");
		for (int i = 0; i < 25; ++i) {
			decode_seq(seq, table[i].seq, k);
			printf("%d\t\t%s\n", table[i].count, seq);
		}
		printf("\n");
	
	} else {
		
		if (err_n == 1) {
			printf("\nError parsing sequence at %d, line %d\nUnparsable character: '%c' (%d)\n\n", err.i, err.l, err.c, err.c);
		} else{
			printf("\nInvalid file pointer.\n\n");
		}
	}
	
	free(table);
	
    return 0;
}
