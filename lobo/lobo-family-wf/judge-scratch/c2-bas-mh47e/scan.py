"""Read-only byte scan of pbo members (LZSS-decompressing compressed entries) and loose files.
Usage: python -I scan.py <root> [<root> ...]  -- prints hits for the tokens below."""
import os, re, struct, sys

TOKENS = [b'mh47', b'mh-47', b'mh_47', b'ch47', b'ch-47', b'ch_47', b'chinook', b'bas_mh47',
          b'bas_ch47', b'mh47e', b'bas_mh6', b'bas_ah6', b'littlebird', b'bas_mah60', b'bas_soar',
          b'bas_mh60', b'bas_']

def lzss(src, outlen):
    out = bytearray(); i = 0; n = len(src)
    while len(out) < outlen and i < n:
        flags = src[i]; i += 1
        for bit in range(8):
            if len(out) >= outlen or i >= n: break
            if flags & (1 << bit):
                out.append(src[i]); i += 1
            else:
                if i + 1 >= n: break
                b1, b2 = src[i], src[i + 1]; i += 2
                rpos = b1 | ((b2 & 0xF0) << 4); rlen = (b2 & 0x0F) + 3
                p = len(out) - rpos
                for k in range(rlen):
                    if len(out) >= outlen: break
                    out.append(out[p + k] if p + k >= 0 else 0x20)
    return bytes(out), len(out) == outlen

def members(data):
    p = 0; ents = []
    while True:
        z = data.find(b'\0', p)
        if z < 0 or z + 21 > len(data): return None
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
        ents.append((name.decode('latin1'), f))
    out = []
    for name, f in ents:
        out.append((name, f, p)); p += f[4]
    return out

def scan_bytes(label, blob, stats):
    low = blob.lower(); hits = {}
    for t in TOKENS:
        c = low.count(t)
        if c: hits[t.decode()] = c
    return hits

def ctx(blob, tok, k=3, w=70):
    low = blob.lower(); res = []; s = 0
    while len(res) < k:
        i = low.find(tok, s)
        if i < 0: break
        res.append(blob[max(0, i - w):i + len(tok) + w].replace(b'\0', b' ').decode('latin1', 'replace'))
        s = i + 1
    return res

def main():
    show = os.environ.get('SHOWCTX', '')
    stats = {'pbos': 0, 'members': 0, 'compressed': 0, 'decomp_ok': 0, 'decomp_bad': 0, 'loose': 0, 'pbo_fail': 0}
    for root in sys.argv[1:]:
        for dp, dn, fn in os.walk(root):
            for f in fn:
                path = os.path.join(dp, f)
                data = open(path, 'rb').read()
                if f.lower().endswith('.pbo'):
                    stats['pbos'] += 1
                    ms = members(data)
                    if ms is None:
                        stats['pbo_fail'] += 1
                        h = scan_bytes(path, data, stats)
                        if h: print('PBO-RAW', path, h)
                        continue
                    for name, fl, off in ms:
                        stats['members'] += 1
                        raw = data[off:off + fl[4]]
                        blob = raw
                        if fl[1] and fl[1] != fl[4]:
                            stats['compressed'] += 1
                            blob, ok = lzss(raw, fl[1])
                            stats['decomp_ok' if ok else 'decomp_bad'] += 1
                        h = scan_bytes(name, blob, stats)
                        hn = scan_bytes(name, name.encode('latin1'), stats)
                        for k, v in hn.items(): h['NAME:' + k] = v
                        if h:
                            print('HIT', path, '::', name, h)
                            if show:
                                for t in show.split(','):
                                    for c in ctx(blob, t.encode()): print('    ', repr(c))
                else:
                    stats['loose'] += 1
                    h = scan_bytes(path, data, stats)
                    if h:
                        print('LOOSE', path, h)
                        if show:
                            for t in show.split(','):
                                for c in ctx(data, t.encode()): print('    ', repr(c))
    print('STATS', stats)

main()
