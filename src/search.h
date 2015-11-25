#ifndef SEARCH_H
#define SEARCH_H

#include "table.h"

/*	@struct				Store parse error information found while searching.
**	@field	c			character that couldn't be parsed
**	@field	i			index in line
**	@field	l			line number
*/
typedef struct parse_error {
	int c;
	int i; 
	int l; 
} parse_error;

/*	@function			Search for and tally k-mers in a given file.
**
**	@param	fp			File stream to be searched.
**	@param	k			Length of k-mer;
**	@param 	table		Pointer to block of memory for use as a hash table.
**	@param	divsize		Size of table divisions.
**	@param	ndivs		Number of table divisions.
**	@param 	ncollisions	Pointer for storing number of collisions found while searching.
**						Will be larger for small division sizes.
** 	@param 	nunresolved	Pointer for storing number of unresolved collisions found while searching.
**						Will be larger for small number of divisions (and to a lesser extent, division size).
** 	@param 	err			Pointer to a struct for storing error information.
**
**	@return				An error id:
						0 - no error
						1 - error parsing sequence
						2 - fp was NULL
						3 - error inserting sequence: x exceeded division size. this shouldn't happen.
*/
int search(FILE* fp, int k, kmer* table, int divsize, int ndivs, int* ncollisions, int* nunresolved, parse_error* err);

#endif