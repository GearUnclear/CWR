import os, sys, struct, re, glob

def lzss(data, outlen):
    out = bytearray()
    i = 0
    n = len(data)
    while len(out) < outlen and i < n:
        flags = data[i]; i += 1
        for bit in range(8):
            if len(out) >= outlen or i >= n:
                break
            if flags & 1:
                out.append(data[i]); i += 1
            else:
                if i + 1 >= n:
                    break
                b1 = data[i]; b2 = data[i + 1]; i += 2
                rpos = b1 | ((b2 & 0xF0) << 4)
                rlen = (b2 & 0x0F) + 3
                start = len(out) - rpos
                for k in range(rlen):
                    idx = start + k
                    out.append(out[idx] if 0 <= idx < len(out) else 0x20)
            flags >>= 1
    return bytes(out)

def read_cstr(f):
    b = bytearray()
    while True:
        c = f.read(1)
        if not c or c == b'\0':
            return bytes(b)
        b += c

def parse(path):
    entries = []
    with open(path, 'rb') as f:
        first = True
        while True:
            name = read_cstr(f)
            hdr = f.read(20)
            if len(hdr) < 20:
                break
            meth, orig, res, ts, size = struct.unpack('<5I', hdr)
            if first and name == b'' and meth == 0x56657273:
                # product entry; read key/values
                while True:
                    k = read_cstr(f)
                    if k == b'':
                        break
                    read_cstr(f)
                first = False
                continue
            first = False
            if name == b'':
                break
            entries.append((name.decode('latin-1'), meth, orig, size))
        base = f.tell()
        out = []
        off = base
        for name, meth, orig, size in entries:
            f.seek(off)
            raw = f.read(size)
            off += size
            comp = (meth == 0x43707273) or (orig != 0 and orig != size)
            if comp:
                try:
                    data = lzss(raw, orig)
                except Exception as e:
                    data = raw
                    comp = 'FAIL'
            else:
                data = raw
            out.append((name, comp, data))
    return out

if __name__ == '__main__':
    pat = re.compile(sys.argv[1].encode(), re.I)
    files = sys.argv[2:]
    for p in files:
        try:
            ents = parse(p)
        except Exception as e:
            print('PARSEFAIL', p, e); continue
        ncomp = sum(1 for e in ents if e[1])
        hits = []
        for name, comp, data in ents:
            if pat.search(name.encode('latin-1')):
                hits.append(('NAME', name, comp, ''))
            for m in pat.finditer(data):
                ctx = data[max(0, m.start() - 40):m.end() + 40]
                ctx = re.sub(rb'[^\x20-\x7e]', b'.', ctx).decode()
                hits.append(('DATA', name, comp, ctx))
        if hits:
            print('==', os.path.basename(p), 'entries', len(ents), 'compressed', ncomp)
            seen = set()
            for h in hits[:60]:
                key = (h[1], h[3])
                if key in seen: continue
                seen.add(key)
                print('  ', h[0], h[1], 'C' if h[2] else '-', '|', h[3])
