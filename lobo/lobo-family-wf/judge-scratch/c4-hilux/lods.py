import sys, os, struct
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pbo import entries
from scan import lzss

BS = chr(92)
pb = sys.argv[1]
model = sys.argv[2]
es, base = entries(pb)
data = open(pb, 'rb').read()
off = base
blob = None
for name, packing, orig, size in es:
    b = data[off:off + size]
    off += size
    if name == model:
        blob = lzss(b, orig) if packing else b
        break
assert blob[:4] == b'MLOD'
ver, nlods = struct.unpack_from('<II', blob, 4)
p = 12
for li in range(nlods):
    magic = blob[p:p + 4]
    hv, hs = struct.unpack_from('<II', blob, p + 4)
    npts, nnrm, nfac, flags = struct.unpack_from('<IIII', blob, p + 12)
    q = p + 28
    q += npts * 16 + nnrm * 12
    tex = {}
    for f in range(nfac):
        # SP3X face: nverts(4) + 4*(pt,nrm,u,v)=4*16 + texname[32] + flags(4)
        tn = blob[q + 4 + 64:q + 4 + 64 + 32].split(b'\0')[0].decode('latin1')
        tex[tn] = tex.get(tn, 0) + 1
        q += 4 + 64 + 32 + 4
    # TAGG section
    assert blob[q:q + 4] == b'TAGG', (li, blob[q:q + 8])
    q += 4
    sels = []
    while True:
        tag = blob[q:q + 64].split(b'\0')[0].decode('latin1')
        q += 64
        ln, = struct.unpack_from('<I', blob, q)
        q += 4 + ln
        if not tag.startswith('#'):
            sels.append(tag)
        if tag == '#EndOfFile#':
            break
    res, = struct.unpack_from('<f', blob, q)
    q += 4
    ext = {k: v for k, v in tex.items() if not k.lower().startswith(('lobotoy', 'loboter'))}
    print('LOD', li, magic, 'res', res, 'faces', nfac, 'external', ext)
    if len(sys.argv) > 3:
        print('    selections', sels[:60])
    p = q
