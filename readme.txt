This program tallies the kmers of a given length in a given file and outputs the frequency of the most common.
It uses a Rabin-Karp, 'rolling-hash' algorithm to search the file in O(n) time, where n is the length of the file.

Requirements: cmake, clang

Usage: 
make
./run <filename> <k>

<filename> should be a text file containing no whitespace, newlines, etc.
<k> should be the length of kmer to search for.

Program will print the frequency of the most commong kmer of length <k> in <filename>.

