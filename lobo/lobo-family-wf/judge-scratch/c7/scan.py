import sys, struct, re, os, glob, json

def lzss(src, outlen):
    out = bytearray(); i = 0; n = len(src)
    while len(out) < outlen and i < n:
        flags = src[i]; i += 1
        for b in range(8):
            if len(out) >= outlen or i >= n: break
            if flags & (1 << b):
                out.append(src[i]); i += 1
            else:
                if i + 1 >= n: break
                b1 = src[i]; b2 = src[i+1]; i += 2
                rpos = b1 | ((b2 & 0xF0) << 4)
                rlen = (b2 & 0x0F) + 3
                start = len(out) - rpos
                for k in range(rlen):
                    if len(out) >= outlen: break
                    p = start + k
                    out.append(out[p] if p >= 0 else 0x20)
    return bytes(out)

def read_pbo(path):
    data = open(path, 'rb').read()
    p = 0; entries = []
    while True:
        z = data.find(b'\0', p)
        name = data[p:z]; p = z + 1
        mp, osz, res, ts, dsz = struct.unpack_from('<5I', data, p); p += 20
        if not name:
            if mp == 0x56657273 and not entries:
                while True:
                    z = data.find(b'\0', p); empty = (z == p); p = z + 1
                    if empty: break
                    z = data.find(b'\0', p); p = z + 1
                continue
            break
        entries.append([name.decode('latin1'), mp, osz, dsz])
    off = p; res = []
    for name, mp, osz, dsz in entries:
        raw = data[off:off+dsz]; off += dsz
        comp = (mp == 0x43707273) or (mp == 0 and osz and osz != dsz)
        status = 'plain'
        if comp:
            try:
                body = lzss(raw, osz); status = 'lzss' if len(body) == osz else 'lzss-short'
            except Exception as e:
                body = raw; status = 'lzss-fail'
        else:
            body = raw
        res.append((name, status, body))
    return res

pats = [p.lower().encode() for p in sys.argv[1].split(',')]
paths = []
for g in sys.argv[2:]:
    paths += glob.glob(g)
tot_entries = 0; comp_entries = 0; bad = 0
for path in sorted(paths):
    if path.lower().endswith('.pbo'):
        try:
            ents = read_pbo(path)
        except Exception as e:
            print('ERR', path, e); continue
    else:
        ents = [(os.path.basename(path), 'file', open(path,'rb').read())]
    for name, status, body in ents:
        tot_entries += 1
        if status.startswith('lzss'): comp_entries += 1
        if status in ('lzss-short','lzss-fail'): bad += 1
        lname = name.lower().encode('latin1'); lb = body.lower()
        for pt in pats:
            if pt in lname:
                print('NAMEHIT', pt.decode(), path, name)
            cnt = lb.count(pt)
            if cnt:
                ctxs = []
                for m in re.finditer(re.escape(pt), lb):
                    s = body[max(0,m.start()-60):m.end()+80]
                    ctxs.append(s.decode('latin1').replace('\n',' ').replace('\r',' ').replace('\0','|'))
                    if len(ctxs) >= 4: break
                print('HIT', pt.decode(), cnt, path, name, status)
                for c in ctxs: print('    ', repr(c))
print('SCANNED files=%d entries=%d compressed=%d badcomp=%d' % (len(paths), tot_entries, comp_entries, bad))
