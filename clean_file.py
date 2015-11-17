import sys

if __name__ == "__main__":
    fn = sys.argv[1]
    out = sys.argv[2]
    clean = open(fn).read().replace('\n', '').replace('\r', '').replace('\t', '').replace(' ', '')
    open(out, "w").write(clean)