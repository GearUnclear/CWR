import struct
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

