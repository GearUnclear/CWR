"""Dump root config.cpp of each pbo to outdir/<stem>/config.cpp. Usage: dumpcfg.py <scriptsdir> outdir pbo..."""
import os, sys
sys.path.insert(0, sys.argv[1])
import pboscan
out = sys.argv[2]
for pbo in sys.argv[3:]:
    data = open(pbo, 'rb').read()
    pos, entries = pboscan.header(data)
    for name, method, orig, size in entries:
        body = data[pos:pos + size]
        pos += size
        if name.lower() == 'config.cpp':
            if method == pboscan.CPRS and orig:
                body = pboscan.lzss(body, orig)
            d = os.path.join(out, os.path.splitext(os.path.basename(pbo))[0])
            os.makedirs(d, exist_ok=True)
            open(os.path.join(d, 'config.cpp'), 'wb').write(body)
