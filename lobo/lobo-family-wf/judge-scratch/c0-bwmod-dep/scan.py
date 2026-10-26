import struct, sys, os, re, glob

def lzss(data, outlen):
    out = bytearray(); i = 0; n = len(data)
    while len(out) < outlen and i < n:
        flags = data[i]; i += 1
        for bit in range(8):
            if len(out) >= outlen or i >= n: break
            if flags & (1 << bit):
                out.append(data[i]); i += 1
            else:
                if i + 1 >= n: break
                b1 = data[i]; b2 = data[i+1]; i += 2
                rpos = b1 | ((b2 & 0xF0) << 4)
                rlen = (b2 & 0x0F) + 3
                start = len(out) - rpos
                for k in range(rlen):
                    if len(out) >= outlen: break
                    p = start + k
                    out.append(out[p] if p >= 0 else 0x20)
    return bytes(out)

def entries(path):
    data = open(path, 'rb').read()
    p = 0; ents = []; first = True
    while True:
        z = data.find(b'\0', p)
        if z < 0: break
        name = data[p:z]; p = z + 1
        if p + 20 > len(data): break
        f = struct.unpack_from('<5I', data, p); p += 20
        if not name:
            if f[0] == 0x56657273 and first:
                while True:
                    z = data.find(b'\0', p); empty = z == p; p = z + 1
                    if empty: break
                    z = data.find(b'\0', p); p = z + 1
                first = False
                continue
            break
        first = False
        ents.append((name.decode('latin1'), f))
    off = p
    res = []
    for name, f in ents:
        raw = data[off:off + f[4]]; off += f[4]
        comp = f[0] != 0 or (f[1] and f[1] != f[4])
        if comp:
            try:
                body = lzss(raw, f[1]); status = 'lzss'
            except Exception as e:
                body = raw; status = 'lzss-fail'
        else:
            body = raw; status = 'plain'
        res.append((name, status, body))
    return res, data

pats = [p.encode() for p in sys.argv[1].split(',')]
targets = sys.argv[2:]
files = []
for t in targets:
    if os.path.isdir(t):
        for root, ds, fs in os.walk(t):
            for fn in fs: files.append(os.path.join(root, fn))
    else:
        files.extend(glob.glob(t))
stats = {'files': 0, 'compressed_entries': 0, 'fail': 0}
for fpath in files:
    stats['files'] += 1
    try:
        if fpath.lower().endswith('.pbo'):
            ents, raw = entries(fpath)
        else:
            raw = open(fpath, 'rb').read(); ents = [('<file>', 'plain', raw)]
    except Exception as e:
        print('ERR', fpath, e); continue
    for name, status, body in ents:
        if status != 'plain': stats['compressed_entries'] += 1
        if status == 'lzss-fail': stats['fail'] += 1
        low = body.lower()
        for pat in pats:
            for m in re.finditer(re.escape(pat), low):
                s = max(0, m.start() - 60); e = min(len(body), m.end() + 80)
                ctx = body[s:e].decode('latin1').replace('\n', ' ').replace('\r', ' ')
                ctx = ''.join(c if 32 <= ord(c) < 127 else '.' for c in ctx)
                print(f'HIT {pat.decode()} | {os.path.basename(fpath)} :: {name} [{status}] | {ctx}')
print('STATS', stats)
