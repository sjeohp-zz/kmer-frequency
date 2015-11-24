typedef struct kmer {
	uint32_t count;
	unsigned char seq[16];
} kmer;

void table_init(kmer** table, int divsize, int ndivs) {
	*table = (kmer*)calloc(divsize*ndivs, sizeof(kmer));
}

int table_insert_helper(kmer* tdiv, unsigned char* seq, uint64_t x) {
	if (tdiv[x].count == 0) {
		tdiv[x].count++;
		memcpy(tdiv[x].seq, seq, 16);
	} else if (memcmp(tdiv[x].seq, seq, 16) == 0) {
		tdiv[x].count++;
	} else {
		return 1;
	}
	return 0;
}

void table_insert(kmer* table, int divsize, int ndivs, unsigned char* seq, uint64_t x, int* ncollisions, int* nunresolved) {
	int col = 0;
	for (int i = 0; i < ndivs; ++i) {
		if ((col = table_insert_helper(table+i*divsize, seq, x))) {
			(*ncollisions)++;
		} else {
			break;
		}
	}
	*nunresolved += col;
}

void table_sort_dsc(kmer* a, uint32_t n) {
    if (n < 2) { return; }
    uint32_t p = a[n/2].count;
    kmer* l = a;
    kmer* r = a + n - 1;
    while (l <= r) {
        if ((*l).count > p) {
            l++;
            continue;
        }
        if ((*r).count < p) {
            r--;
            continue;
        }
        kmer t = *l;
        *l++ = *r;
        *r-- = t;
    }
    table_sort_dsc(a, r - a + 1);
    table_sort_dsc(l, a + n - l);
}