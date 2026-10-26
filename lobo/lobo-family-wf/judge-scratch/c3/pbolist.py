import sys, struct
def read_cstr(b, i):
    j = b.index(b'\0', i)
    return b[i:j].decode('latin-1'), j+1
def entries(path):
    b = open(path,'rb').read()
    i = 0; out=[]; first=True; props={}
    while True:
        name, i = read_cstr(b, i)
        method, orig, res, ts, size = struct.unpack_from('<5I', b, i); i += 20
        if first and name=='' and method==0x56657273:
            # properties
            while True:
                k, i = read_cstr(b, i)
                if not k: break
                v, i = read_cstr(b, i); props[k]=v
            first=False; continue
        first=False
        if name=='' : break
        out.append((name, method, orig, size))
    data_start = i
    off = data_start; res=[]
    for (n,m,o,s) in out:
        res.append((n,m,o,s,off)); off += s
    return props, res, b
if __name__=='__main__':
    p, es, b = entries(sys.argv[1])
    print('props', p)
    for n,m,o,s,off in es:
        print(f'{n:60s} method={m:#x} orig={o} size={s} {"COMPRESSED" if (m!=0 or (o and o!=s)) else ""}')
