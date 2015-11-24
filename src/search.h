

int search(FILE* fp, int k, kmer* table, int divsize, int ndivs, int* ncollisions, int* nunresolved) {
	
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
			
			if (encode_seq(enc, buf, k)) { error = 1; break; }
			x = fasthash64(enc, 16, 101) % divsize;
			
			c_lead = buf[k];
			c_trail = buf[0];
			charc = 0;
			while (c_lead) {
				charc++;
				
				table_insert(table, divsize, ndivs, enc, x, ncollisions, nunresolved);

				if (buf[k+charc]) {
					if (encode_update(enc, c_lead, k)) { error = 1; break; }
					x = fasthash64(enc, 16, 101) % divsize;
				}
				c_lead = buf[k+charc];
				c_trail = buf[charc];
			}
		}
	} else {
		error = 1;
	}
	
	free(buf);
	free(enc);
	
	return error;
}