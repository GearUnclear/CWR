import sys, struct, glob, os, re
import sys; sys.stdout.reconfigure(encoding="utf-8", errors="replace")
def read_cstr(b, i):
    j = b.index(b'\0', i)
    return b[i:j].decode('latin-1'), j+1
def lzss(data, outlen):
    out = bytearray(); i=0
    try:
        while len(out) < outlen:
            flags = data[i]; i+=1
            for bit in range(8):
                if len(out)>=outlen: break
                if flags & (1<<bit):
                    out.append(data[i]); i+=1
                else:
                    b1=data[i]; b2=data[i+1]; i+=2
                    rpos = len(out) - (b1 | ((b2 & 0xF0)<<4))
                    rlen = (b2 & 0x0F) + 3
                    for k in range(rlen):
                        out.append(0x20 if rpos+k<0 else out[rpos+k])
    except IndexError:
        return bytes(out), False
    return bytes(out), True
def entries(b):
    i=0; out=[]; first=True
    while True:
        name, i = read_cstr(b, i)
        method, orig, res, ts, size = struct.unpack_from('<5I', b, i); i += 20
        if first and name=='' and method==0x56657273:
            while True:
                k, i = read_cstr(b, i)
                if not k: break
                v, i = read_cstr(b, i)
            first=False; continue
        first=False
        if name=='': break
        out.append([name, method, orig, size])
    off=i
    for e in out:
        e.append(off); off+=e[3]
    return out
def contents(path):
    b=open(path,'rb').read()
    try:
        es=entries(b)
    except Exception as ex:
        yield ('<raw>', b.lower(), 'unparsed:'+str(ex)); return
    for n,m,o,s,off in es:
        d=b[off:off+s]
        if m==0x43707273 or (o and o!=s):
            dd, ok = lzss(d, o)
            yield (n, dd.lower(), 'decomp-ok' if ok else 'decomp-FAIL')
        else:
            yield (n, d.lower(), 'plain')
pats=[p.lower().encode() for p in sys.argv[1].split(',')]
files=[]
for g in sys.argv[2:]:
    files+=glob.glob(g, recursive=True)
stats={'plain':0,'decomp-ok':0,'decomp-FAIL':0}
files=[f for f in files if __import__("os").path.isfile(f)]
for f in files:
    if f.lower().endswith('.pbo'):
        it=contents(f)
    else:
        it=[('<file>', open(f,'rb').read().lower(), 'plain')]
    for n,d,st in it:
        k=st if st in stats else 'plain'
        stats[k]=stats.get(k,0)+1
        if st=='decomp-FAIL': print('DECOMP FAIL', f, n)
        for p in pats:
            for mm in re.finditer(re.escape(p), d):
                s=max(0,mm.start()-80); e=min(len(d), mm.end()+80)
                ctx=d[s:e].replace(b'\r',b' ').replace(b'\n',b' | ')
                print(f'{os.path.basename(f)} :: {n} [{st}] :: {p.decode()} :: {ctx.decode("latin-1")}')
print('STATS', stats, 'files', len(files))
