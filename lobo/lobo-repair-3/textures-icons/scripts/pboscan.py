"""Read-only decompressing PBO byte scanner.

Usage: pboscan.py <needle>[,<needle>...] dir [dir...]
Prints archive#member and hit count for every member (LZSS 'Cprs' members
decoded) whose bytes contain a needle (case-insensitive). Backup dirs skipped.
"""
import os, struct, sys

CPRS = 0x43707273


def header(data):
    pos = 0
    entries = []
    first = True
    while True:
        end = data.index(b'\0', pos)
        name = data[pos:end].decode('latin-1')
        pos = end + 1
        method, orig, res, ts, size = struct.unpack_from('<5I', data, pos)
        pos += 20
        if first and name == '' and method == 0x56657273:
            while True:
                end = data.index(b'\0', pos); k = data[pos:end]; pos = end + 1
                if not k:
                    break
                end = data.index(b'\0', pos); pos = end + 1
            first = False
            continue
        first = False
        if name == '':
            break
        entries.append((name, method, orig, size))
    return pos, entries


def lzss(src, orig):
    out = bytearray()
    i = 0
    while len(out) < orig and i < len(src):
        flags = src[i]; i += 1
        for bit in range(8):
            if len(out) >= orig or i >= len(src):
                break
            if flags & (1 << bit):
                out.append(src[i]); i += 1
            else:
                b1, b2 = src[i], src[i + 1]; i += 2
                rpos = b1 | ((b2 & 0xF0) << 4)
                rlen = (b2 & 0x0F) + 3
                start = len(out) - rpos
                for k in range(rlen):
                    p = start + k
                    out.append(out[p] if p >= 0 else 0x20)
    return bytes(out[:orig])


def main():
    needles = [n.lower().encode('latin-1') for n in sys.argv[1].split(',')]
    for d in sys.argv[2:]:
        for root, _, files in os.walk(d):
            if '.ud-backups' in root:
                continue
            for fn in files:
                if not fn.lower().endswith('.pbo'):
                    continue
                path = os.path.join(root, fn)
                data = open(path, 'rb').read()
                try:
                    pos, entries = header(data)
                except Exception as e:
                    print('ERR', path, e); continue
                for name, method, orig, size in entries:
                    body = data[pos:pos + size]
                    pos += size
                    if method == CPRS and orig:
                        try:
                            body = lzss(body, orig)
                        except Exception as e:
                            print('LZSSERR', path, name, e); continue
                    low = body.lower()
                    for n in needles:
                        c = low.count(n)
                        if c:
                            print(f'{path}#{name}\t{n.decode()}\t{c}')


if __name__ == '__main__':
    main()
