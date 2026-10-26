import sys, struct, re, os, glob
pats = [p.encode() for p in sys.argv[1].split(',')]
roots = sys.argv[2:]
rx = re.compile(b'|'.join(re.escape(p) for p in pats), re.I)

def lzss(data, outlen):
    out = bytearray(); i = 0; n = len(data)
    while len(out) < outlen and i < n:
        flags = data[i]; i += 1
        for b in range(8):
            if len(out) >= outlen or i >= n: break
            if flags & (1 << b):
                out.append(data[i]); i += 1
            else:
                if i + 1 >= n: return bytes(out), False
                lo = data[i]; hi = data[i+1]; i += 2
                rpos = lo | ((hi & 0xF0) << 4); rlen = (hi & 0x0F) + 3
                start = len(out) - rpos
                for k in range(rlen):
                    p = start + k
                    out.append(out[p] if p >= 0 else 0x20)
    return bytes(out[:outlen]), True

def entries(data):
    p = 0; ents = []
    while True:
        z = data.find(b'\0', p)
        if z < 0 or z + 21 > len(data): return ents, None
        name = data[p:z]; p = z + 1
        f = struct.unpack_from('<5I', data, p); p += 20
        if not name:
            if f[0] == 0x56657273 and not ents:
                while True:
                    z = data.find(b'\0', p); empty = z == p; p = z + 1
                    if empty: break
                    z = data.find(b'\0', p); p = z + 1
                continue
            break
        ents.append((name, f))
    off = p; out = []
    for name, f in ents:
        out.append((name, f, off)); off += f[4]
    return out, None

def scan_pbo(path):
    data = open(path, 'rb').read()
    hits = set(); bad = 0
    try:
        ents, _ = entries(data)
    except Exception as e:
        print('ERR', path, e); return
    for name, f, off in ents:
        blob = data[off:off+f[4]]
        if f[0] == 0x43707273 and f[1] and f[1] != f[4]:
            dec, ok = lzss(blob, f[1])
            if not ok or len(dec) != f[1]: bad += 1
            blob = dec
        for m in rx.finditer(name + b' ' + blob):
            s = max(0, m.start()-60); ctx = (name + b' ' + blob)[s:m.end()+60]
            hits.add((name.decode('latin1'), ctx.decode('latin1').replace('\n',' ').replace('\r',' ')))
    for h in sorted(hits)[:20]:
        print('HIT', os.path.basename(path), '|', h[0], '|', h[1])
    if bad: print('WARN', path, bad, 'members failed decompress')

for r in roots:
    for path in glob.glob(os.path.join(r, '**', '*'), recursive=True):
        if os.path.isdir(path): continue
        low = path.lower()
        if low.endswith('.pbo'):
            scan_pbo(path)
        elif low.endswith(('.sqm','.sqs','.sqf','.cpp','.hpp','.txt','.ext','.csv','.html','.htm','.bin')):
            try: b = open(path,'rb').read()
            except Exception: continue
            for m in rx.finditer(b):
                s=max(0,m.start()-60); print('HIT', path, '|', b[s:m.end()+60].decode('latin1').replace('\n',' '))
print('DONE')
