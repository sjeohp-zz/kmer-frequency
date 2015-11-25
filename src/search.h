#ifndef SEARCH_H
#define SEARCH_H

#include "table.h"

typedef struct parse_error {
	int c; // character that couldn't be parsed
	int i; // index in line
	int l; // line number
} parse_error;

int search(FILE* fp, int k, kmer* table, int divsize, int ndivs, int* ncollisions, int* nunresolved, parse_error* err);

#endif