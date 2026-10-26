import os, sys
seen = {}
for fn in sys.argv[1:]:
    for line in open(fn, encoding='utf-8'):
        p, pre, *_ = line.rstrip('\n').split('\t')
        base = os.path.splitext(os.path.basename(p.replace('\\', '/')))[0]
        if base.lower() != pre.lower():
            seen[p] = pre
for k, v in seen.items():
    print(k, '->', v)
print(len(seen), 'pbos with explicit differing prefix')
