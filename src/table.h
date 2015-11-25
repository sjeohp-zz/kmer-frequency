#ifndef TABLE_H
#define TABLE_H

/* 	@struct				Store information about k-mers.
**	@field 	count		Number of times the k-mer appears.
**	@field 	seq			Encoded k-mer sequence.
*/
typedef struct kmer {
	int count;
	unsigned char seq[16];
} kmer;

/* 	@function			Initialize memory for ndivs * divsize kmers.
**
**	@param	table		Pointer to the address table should start at.
** 	@param 	divsize		Size of divisions in table.
**						Increasing this will reduce number of collisions.
**	@param	ndivs		Number of divisions in table.
**						Increasing this will reduce proportion of collisions going unresolved.
*/
void table_init(kmer** table, int divsize, int ndivs);

/* 	@function			Insert encoded k-mer sequence and incrememnt its count.
**
**	@param	table		Memory for storing k-mers.
** 	@param 	divsize		Size of divisions in table.
**	@param	ndivs		Number of divisions in table.
** 	@param 	seq			Encoded k-mer sequence.
**	@param	x			Index to insert at (should be hash of sequence modulo divsize).
**	@param 	ncollisions	Pointer for storing number of collisions found while inserting.
** 	@param 	nunresolved	Pointer for recording an unresolved collision. Means sequence was not inserted.
**
**	@return 			0 - no error
**						1 - insertion error. x exceeded divsize.
*/
int table_insert(kmer* table, int divsize, int ndivs, unsigned char* seq, int x, int* ncollisions, int* nunresolved);

/* 	@function			Sort k-mer table from most common k-mer to least. (implemented as a quicksort)
**	@param	a			Table to be sorted.
**	@param	n			Size of table. Should be divsize * ndivs for the entire table.
*/
void table_sort_dsc(kmer* a, int n);

#endif