import sys, struct, re


def lzss(data, outlen):
    out = bytearray(); i = 0
    while len(out) < outlen and i < len(data):
        flags = data[i]; i += 1
        for b in range(8):
            if len(out) >= outlen or i >= len(data):
                break
            if flags & (1 << b):
                out.append(data[i]); i += 1
            else:
                if i + 1 >= len(data):
                    break
                b1, b2 = data[i], data[i + 1]; i += 2
                rpos = len(out) - (b1 | ((b2 & 0xF0) << 4))
                rlen = (b2 & 0x0F) + 3
                for k in range(rlen):
                    p = rpos + k
                    out.append(out[p] if 0 <= p < len(out) else 0x20)
    return bytes(out)


def entries(path):
    d = open(path, 'rb').read(); i = 0; ents = []
    while True:
        j = d.index(b'\0', i); name = d[i:j]; i = j + 1
        mp, osz, res, ts, dsz = struct.unpack_from('<5I', d, i); i += 20
        if name == b'' and mp == 0x56657273:
            while True:
                j = d.index(b'\0', i); k = d[i:j]; i = j + 1
                if k == b'':
                    break
                j = d.index(b'\0', i); i = j + 1
            continue
        if name == b'':
            break
        ents.append((name.decode('latin1'), mp, osz, dsz))
    off = i; res = []
    for n, mp, osz, dsz in ents:
        raw = d[off:off + dsz]; off += dsz
        comp = bool(osz and osz != dsz)
        res.append((n, comp, lzss(raw, osz) if comp else raw))
    return res
