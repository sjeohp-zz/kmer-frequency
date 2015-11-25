#ifndef TABLE_H
#define TABLE_H

typedef struct kmer {
	int count;
	unsigned char seq[16];
} kmer;

void table_init(kmer** table, int divsize, int ndivs);
void table_insert(kmer* table, int divsize, int ndivs, unsigned char* seq, int x, int* ncollisions, int* nunresolved);
void table_sort_dsc(kmer* a, int n);

#endif