Requirements:
clang
make

To build:
make

To run:
./run <filename> <k>

<filename> 	is a file of DNA sequence reads in FASTQ format.
<k> 		is the length of k-mers to be counted.

K-mer search runs in O(n) time, where n is the size of the file.
Sorting is done in-place, in O(nlogn) time.

Tested on files up to 4.5gb in size.