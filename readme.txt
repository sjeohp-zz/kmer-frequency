This program tallies all kmers of a given length in a given file and outputs the frequency of the most common. 
It uses a Rabin-Karp, 'rolling-hash' algorithm. The search hashes many strings that only differ by two characters 
(the first and the last), so it uses a polynomial hash function that can be computed easily given the previous 
hash and the characters that have changed. The search completes in O(n) time, where n is the length of the file, 
demonstrated in the following simple tests:

./run test-data/test01.txt 30 -b
Bytes in file: 60000
Subsequences of length 30, highest frequency: 23
Mean time over 100 iterations: 18.7827ms

./run test-data/test02.txt 30 -b
Bytes in file: 600000
Subsequences of length 30, highest frequency: 230
Mean time over 100 iterations: 162.74ms

./run test-data/test03.txt 30 -b
Bytes in file: 6000000
Subsequences of length 30, highest frequency: 2300
Mean time over 100 iterations: 1621.49ms

The program can easily be altered to search for a specific substring.

Requirements: cmake, clang

Usage:
make
./run <filename> <k> (optional: -b)

<filename> should be a text file containing no whitespace, newlines, etc.
<k> should be the length of kmer to search for.
-b instructs the program to run 100 times and output the mean runtime.

Program will print the frequency of the most commong kmer of length <k> in <filename>.

Limitations:
Collisions are improbable but can occur, generating false positives. Currently results appear accurate up to a k value 
of at least 120, verified using a different hash function (naive.py), to generate the same results.