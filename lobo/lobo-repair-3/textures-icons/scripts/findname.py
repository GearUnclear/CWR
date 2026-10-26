"""Find PBO entries by leaf name (without extension) across index files.

Usage: findname.py idx1.tsv[,idx2...] leaf [leaf...]
"""
import os, sys
idx = sys.argv[1].split(',')
rows = []
for fn in idx:
    for line in open(fn, encoding='utf-8'):
        p, pre, name, method, orig, size = line.rstrip('\n').split('\t')
        rows.append((p, pre, name, method, int(orig), int(size)))
for leaf in sys.argv[2:]:
    print('==', leaf)
    for p, pre, name, method, orig, size in rows:
        base = name.replace('\\', '/').rsplit('/', 1)[-1]
        stem = os.path.splitext(base)[0]
        if stem.lower() == leaf.lower():
            short = p.replace('\\', '/').split('/addons/')[-1].split('/Addons/')[-1].split('/AddOns/')[-1]
            print(f'   {short:<40} {pre}\\{name}  method={method} orig={orig} size={size}')
