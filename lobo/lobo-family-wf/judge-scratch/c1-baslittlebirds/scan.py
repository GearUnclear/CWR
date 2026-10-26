import sys, os, re, struct, glob

def lzss(data, outsize):
    out = bytearray(); i = 0
    while len(out) < outsize and i < len(data):
        flags = data[i]; i += 1
        for b in range(8):
            if len(out) >= outsize or i >= len(data): break
            if flags & (1 << b):
                out.append(data[i]); i += 1
            else:
                if i + 1 >= len(data): break
                lo, hi = data[i], data[i+1]; i += 2
                rpos = lo | ((hi & 0xF0) << 4)
                rlen = (hi & 0x0F) + 3
                start = len(out) - rpos
                for k in range(rlen):
                    p = start + k
                    out.append(out[p] if 0 <= p < len(out) else 0x20)
    return bytes(out)

def read_pbo(path):
    d = open(path, 'rb').read()
    pos = 0; entries = []
    while True:
        e = d.index(b'\0', pos); name = d[pos:e].decode('latin1'); pos = e + 1
        meth, orig, res, ts, size = struct.unpack_from('<5I', d, pos); pos += 20
        if name == '' and meth == 0x56657273:
            # properties
            while True:
                e = d.index(b'\0', pos); k = d[pos:e]; pos = e + 1
                if not k: break
                e = d.index(b'\0', pos); pos = e + 1
            continue
        if name == '': break
        entries.append((name, meth, orig, size))
    out = []
    for name, meth, orig, size in entries:
        blob = d[pos:pos+size]; pos += size
        comp = False
        if meth == 0x43707273 or (orig and orig != size):
            comp = True
            try: blob = lzss(blob, orig)
            except Exception as ex: blob = b''
        out.append((name, comp, blob))
    return out

pat = re.compile(sys.argv[1].encode(), re.I)
paths = sys.argv[2:]
for p in paths:
    try:
        ents = read_pbo(p)
    except Exception as ex:
        print('ERR', p, ex); continue
    ncomp = sum(1 for e in ents if e[1])
    hits = []
    for name, comp, blob in ents:
        for m in pat.finditer(name.encode('latin1')):
            hits.append((name, 'NAME', m.group(0)))
        for m in pat.finditer(blob):
            s = max(0, m.start()-40); t = m.end()+40
            hits.append((name, comp, blob[s:t]))
    if hits or ncomp:
        print('==', os.path.basename(p), 'entries', len(ents), 'compressed', ncomp, 'hits', len(hits))
        for h in hits[:15]: print('   ', h)
