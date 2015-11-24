#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>

#include "lib/fasthash/fasthash.h"

#include "src/timing.h"
#include "src/encoding.h"
#include "src/table.h"
#include "src/search.h"

#define TABLESIZE 1572869
#define NTABLES 16

int main(int argc, char** argv) {

	if (argc < 3) {
		printf("usage: ./run <filename> <k>\n");
		return 0;
	}

	const char* fn = argv[1];
	const int k = atoi(argv[2]);
	
	if (k > 32) {
		printf("k cannot be greater than 32 for now, or k-mers won't fit in their configured 16byte encoding.");
		return 1;
	}
	
	FILE* fp = fopen(fn, "r");
	if (!fp) {
		printf("Could not open file.\n");
		return 0;
	}
	fseek(fp, 0L, SEEK_END);
	int filesize = ftell(fp);
	printf("File size:\t%d kb\n", filesize/1000);
	printf("Table size:\t%d kb\n", NTABLES*TABLESIZE/1000);
	fseek(fp, 0, SEEK_SET);
	
	time_start();

	kmer* table;
	table_init(&table, TABLESIZE, NTABLES);
	
	int ncollisions = 0;
	int nunresolved = 0;

	if (search(fp, k, table, TABLESIZE, NTABLES, &ncollisions, &nunresolved)) {
		printf("Error parsing sequence");
		return 0;
	}
	table_sort_dsc(table, NTABLES*TABLESIZE);
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
	
    return 0;
}
