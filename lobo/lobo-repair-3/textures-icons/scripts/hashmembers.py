"""SHA-1 of every texture member (paa/pac/jpg) in the given pbos. Usage: hashmembers.py <scriptsdir> out.tsv pbo..."""
import hashlib, os, sys
sys.path.insert(0, sys.argv[1])
import pboscan
with open(sys.argv[2], 'w', encoding='utf-8') as o:
    for pbo in sys.argv[3:]:
        data = open(pbo, 'rb').read()
        stem = os.path.splitext(os.path.basename(pbo))[0]
        pos, entries = pboscan.header(data)
        for name, method, orig, size in entries:
            body = data[pos:pos + size]
            pos += size
            if not name.lower().endswith(('.paa', '.pac', '.jpg')):
                continue
            if method == pboscan.CPRS and orig:
                body = pboscan.lzss(body, orig)
            o.write(f'{hashlib.sha1(body).hexdigest()}\t{stem}\\{name}\t{len(body)}\n')
