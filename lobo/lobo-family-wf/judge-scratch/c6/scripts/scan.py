import sys, struct, os, glob, re
def lzss(data, outlen):
    out = bytearray(); i = 0
    while len(out) < outlen and i < len(data):
        flags = data[i]; i += 1
        for b in range(8):
            if len(out) >= outlen or i >= len(data): break
            if flags & (1 << b):
                out.append(data[i]); i += 1
            else:
                if i + 1 >= len(data): break
                b1, b2 = data[i], data[i+1]; i += 2
                rpos = len(out) - (b1 | ((b2 & 0xF0) << 4))
                rlen = (b2 & 0x0F) + 3
                for k in range(rlen):
                    p = rpos + k
                    out.append(out[p] if p >= 0 else 0x20)
    return bytes(out[:outlen])
def entries(path):
    d = open(path,'rb').read(); p = 0; ents = []
    while True:
        z = d.find(b'\0', p); name = d[p:z]; p = z+1
        f = struct.unpack_from('<5I', d, p); p += 20
        if not name:
            if f[0] == 0x56657273 and not ents:
                while True:
                    z = d.find(b'\0', p); emp = z == p; p = z+1
                    if emp: break
                    z = d.find(b'\0', p); p = z+1
                continue
            break
        ents.append((name.decode('latin1'), f))
    out = []
    for name, f in ents:
        raw = d[p:p+f[4]]; p += f[4]
        comp = f[0] == 0x43707273 or (f[1] and f[1] != f[4])
        data = lzss(raw, f[1]) if comp else raw
        out.append((name, comp, data))
    return out
if __name__ == '__main__':
  pats = [s.lower().encode() for s in sys.argv[1].split(',')]
  for root in sys.argv[2:]:
      for path in sorted(glob.glob(root)):
          if path.lower().endswith('.pbo'):
              try: ents = entries(path)
              except Exception as e: print('ERR', path, e); continue
              for name, comp, data in ents:
                  low = data.lower(); nl = name.lower().encode('latin1')
                  for pat in pats:
                      for m in re.finditer(re.escape(pat), low):
                          s = max(0, m.start()-40); ctx = data[s:m.end()+40]
                          print(f'{os.path.basename(path)} :: {name} {"[C]" if comp else ""} :: {ctx!r}')
                      if pat in nl: print(f'NAME {os.path.basename(path)} :: {name}')
          else:
              data = open(path,'rb').read(); low = data.lower()
              for pat in pats:
                  for m in re.finditer(re.escape(pat), low):
                      s = max(0, m.start()-40); print(f'{path} :: {data[s:m.end()+40]!r}')
