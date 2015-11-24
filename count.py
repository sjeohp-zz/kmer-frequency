import sys

def count(file, sub):
    line = 0
    count = 0
    for string in file:
        if line % 4 == 1:
            for i in xrange(len(string)):
                if string[i:].startswith(sub):
                    count += 1
        line += 1
    return count
    
if __name__ == "__main__":
    file = open(sys.argv[1])
    kmer = sys.argv[2]
    print(count(file, kmer))