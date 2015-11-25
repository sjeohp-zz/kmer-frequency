#include <stdio.h>
#include <stdlib.h>
#include "search.h"
#include "encoding.h"
#include "../lib/fasthash/fasthash.h"

int search(FILE* fp, int k, kmer* table, int divsize, int ndivs, int* ncollisions, int* nunresolved, parse_error* err) {
	
	char* buf = (char*)calloc(1024, sizeof(char));
	unsigned char* enc = (unsigned char*)calloc(16, sizeof(unsigned char));
	int c_lead; // leading char
	int c_trail; // trailing char
	uint64_t x; // hash value
	int linec = 0;
	int charc = 0;
	
	int error = 0;
	if (fp) {
		while (fgets(buf, 1024, fp)) { // read file line by line, stop at EOF
			if (linec++ % 4 != 1) { // skip lines 1, 3, 4
				continue;
			}
			
			int err_c;
			int err_i;
			if (encode_seq(enc, buf, k, &err_c, &err_i)) { 
				error = 1;
				if (err) {
					err->c = err_c;
					err->i = err_i;
					err->l = linec;
				}
				break; 
			}
			x = fasthash64(enc, 16, 101) % divsize;
			
			c_lead = buf[k];
			c_trail = buf[0];
			charc = 0;
			while (c_lead) {
				
				table_insert(table, divsize, ndivs, enc, x, ncollisions, nunresolved);

				if (buf[k+charc+1]) {
					if (encode_update(enc, c_lead, k)) { 
						error = 1;
						if (err) {
							err->c = c_lead;
							err->i = k+charc;
							err->l = linec;
						}
						break;
					}
					x = fasthash64(enc, 16, 101) % divsize;
				}
				charc++;
				c_lead = buf[k+charc];
				c_trail = buf[charc];
			}
		}
	} else {
		error = 2;
	}
	
	free(buf);
	free(enc);
	
	return error;
}