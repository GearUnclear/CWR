"""Read-only scan of OFP pbos (with LZSS decompression) and loose files for needles.
Usage: python -I scan.py <needles-comma-separated> <path> [<path>...]
Paths may be pbo files or directories (recursed)."""
import os, struct, sys

def lzss(src, outlen):
    out = bytearray(); i = 0; n = len(src)
    while len(out) < outlen and i < n:
        flags = src[i]; i += 1
        for bit in range(8):
            if len(out) >= outlen or i >= n: break
            if flags & (1 << bit):
                out.append(src[i]); i += 1
            else:
                if i + 1 >= n: i = n; break
                b1 = src[i]; b2 = src[i+1]; i += 2
                rpos = b1 | ((b2 & 0xF0) << 4)
                rlen = (b2 & 0x0F) + 3
                start = len(out) - rpos
                for k in range(rlen):
                    p = start + k
                    out.append(out[p] if p >= 0 else 0x20)
                    if len(out) >= outlen: break
    return bytes(out), len(out) == outlen

def pbo_members(data):
    p = 0; ents = []; first = True
    while True:
        z = data.find(b'\0', p)
        if z < 0 or z + 21 > len(data): raise ValueError('trunc')
        name = data[p:z]; p = z + 1
        f = struct.unpack_from('<5I', data, p); p += 20
        if not name:
            if f[0] == 0x56657273 and first:
                while True:
                    z = data.find(b'\0', p); empty = z == p; p = z + 1
                    if empty: break
                    z = data.find(b'\0', p); p = z + 1
                first = False; continue
            break
        first = False
        ents.append((name.decode('latin1'), f))
    out = []
    for name, f in ents:
        raw = data[p:p + f[4]]; p += f[4]
        if f[0] == 0 or (f[1] == 0) or f[1] == f[4]:
            out.append((name, raw, 'stored'))
        else:
            dec, ok = lzss(raw, f[1])
            out.append((name, dec, 'lzss-ok' if ok else 'lzss-SHORT'))
    return out

def main():
    needles = [s.lower().encode('latin1') for s in sys.argv[1].split(',') if s]
    hits = {}; stats = {'pbos': 0, 'members': 0, 'lzss': 0, 'lzss_bad': 0, 'loose': 0, 'pbo_err': []}
    def check(label, blob):
        low = blob.lower()
        for nd in needles:
            j = low.find(nd)
            while j >= 0:
                ctx = blob[max(0, j - 60): j + len(nd) + 60]
                hits.setdefault(nd.decode(), []).append((label, ''.join(chr(c) if 32<=c<127 else '.' for c in ctx).replace('\n', ' ').replace('\r', ' ')))
                if len(hits[nd.decode()]) > 400: break
                j = low.find(nd, j + 1)
    def do_file(fp):
        if fp.lower().endswith('.pbo'):
            stats['pbos'] += 1
            data = open(fp, 'rb').read()
            try:
                mem = pbo_members(data)
            except Exception as e:
                stats['pbo_err'].append(fp + ': ' + str(e)); check(fp + '|RAW', data); return
            names = '\n'.join(m[0] for m in mem).encode('latin1')
            check(fp + '|<member names>', names)
            for name, blob, how in mem:
                stats['members'] += 1
                if how != 'stored': stats['lzss'] += 1
                if how == 'lzss-SHORT': stats['lzss_bad'] += 1
                check(fp + '|' + name, blob)
        else:
            stats['loose'] += 1
            check(fp, open(fp, 'rb').read())
    for root in sys.argv[2:]:
        if os.path.isfile(root): do_file(root); continue
        for dp, dn, fn in os.walk(root):
            for f in fn: do_file(os.path.join(dp, f))
    print('STATS', stats)
    for nd, lst in hits.items():
        print('==', nd, len(lst))
        for label, ctx in lst[:60]:
            print('  ', label, '::', ctx)

main()
