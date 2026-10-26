import sys, re, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pbo import entries
from scan import lzss

BS = chr(92)
pb = sys.argv[1]
model = sys.argv[2]
pat = sys.argv[3].replace('/', BS).lower().encode()
n = int(sys.argv[4]) if len(sys.argv) > 4 else 4
es, base = entries(pb)
data = open(pb, 'rb').read()
off = base
for name, packing, orig, size in es:
    blob = data[off:off + size]
    off += size
    if name != model:
        continue
    if packing:
        blob = lzss(blob, orig)
    low = blob.lower()
    i = 0
    shown = 0
    while shown < n:
        i = low.find(pat, i)
        if i < 0:
            break
        print(i, repr(blob[i - 48:i + 48]))
        i += 1
        shown += 1
    # list distinct texture names (SP3X face: 4x(point,normal,u,v)=64 bytes, then 32-byte texname, then flags)
    names = {}
    for m in re.finditer(rb'[A-Za-z0-9_][A-Za-z0-9_ ' + BS.encode() + BS.encode() + rb'.\-]{2,31}\.(?:paa|pac)', blob, re.I):
        s = m.group().decode('latin1')
        names[s] = names.get(s, 0) + 1
    for s, c in sorted(names.items(), key=lambda x: -x[1]):
        if not s.lower().startswith(('lobotoy', 'loboter')):
            print('  ', c, s)
