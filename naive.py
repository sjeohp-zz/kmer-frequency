import sys

def kmer_count(seq, k):
    f = {}
    for x in range(len(seq)+1-k):
        kmer = seq[x:x+k]
        f[kmer] = f.get(kmer, 0) + 1
    curr = 0
    for v in f.values():
        if v > curr:
            curr = v
    return curr
    
if __name__ == "__main__":
    fn = sys.argv[1]
    seq = open(fn).read()
    k = sys.argv[2]
    print(kmer_count(seq, k))