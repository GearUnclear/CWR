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

