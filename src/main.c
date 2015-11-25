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

#define DIVSIZE 1572869
#define NDIVS 16

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
	int filesize = ftell(fp);
	printf("\nFile size:\t%d kb\n", filesize/1000);
	printf("Table size:\t%d kb\n", NDIVS*DIVSIZE/1000);
	fseek(fp, 0, SEEK_SET);
	
	time_start();

	kmer* table;
	table_init(&table, DIVSIZE, NDIVS);
	
	int ncollisions = 0;
	int nunresolved = 0;

	parse_error err;
	int err_n;
	if (!(err_n = search(fp, k, table, DIVSIZE, NDIVS, &ncollisions, &nunresolved, &err))) {
		
		table_sort_dsc(table, NDIVS*DIVSIZE);
		time_stop();
	
		fclose(fp);
	
		printf("\nCollisions:\t%d\n", ncollisions);
		printf("Unresolved:\t%d\n\n", nunresolved);
	
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
