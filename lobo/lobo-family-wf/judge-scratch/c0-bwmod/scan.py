import struct, sys, re, os, glob
def lzss(src, outlen):
    out=bytearray(); i=0
    while len(out)<outlen and i<len(src):
        flags=src[i]; i+=1
        for bit in range(8):
            if len(out)>=outlen: break
            if flags & (1<<bit):
                if i>=len(src): break
                out.append(src[i]); i+=1
            else:
                if i+1>=len(src): break
                b1=src[i]; b2=src[i+1]; i+=2
                rpos=len(out)-(b1 | ((b2&0xF0)<<4))
                rlen=(b2&0x0F)+3
                for k in range(rlen):
                    if len(out)>=outlen: break
                    p=rpos+k
                    out.append(out[p] if p>=0 else 0x20)
    return bytes(out)
def entries(data):
    p=0; ents=[]
    while True:
        z=data.find(b'\0',p); name=data[p:z]; p=z+1
        f=struct.unpack_from('<5I',data,p); p+=20
        if not name:
            if f[0]==0x56657273 and not ents:
                while True:
                    z=data.find(b'\0',p); empty=(z==p); p=z+1
                    if empty: break
                    z=data.find(b'\0',p); p=z+1
                continue
            break
        ents.append((name.decode('latin1'),f))
    off=p; res=[]
    for n,f in ents:
        res.append((n,f,off)); off+=f[4]
    return res
pat=re.compile(open(sys.argv[1],"rb").read().strip(), re.I)
for path in sys.argv[2:]:
    data=open(path,'rb').read()
    try: es=entries(data)
    except Exception as e: print('ERR',path,e); continue
    for n,f,off in es:
        raw=data[off:off+f[4]]
        comp=False
        if f[0]==0x43707273 or (f[1] and f[1]!=f[4]):
            comp=True
            try: raw=lzss(raw,f[1])
            except Exception as e: print('LZERR',path,n,e); continue
        hits=set(m.group(0).decode('latin1') for m in pat.finditer(raw))
        ctx=[]
        for m in list(pat.finditer(raw))[:6]:
            ctx.append(raw[max(0,m.start()-60):m.end()+60].decode('latin1').replace('\n',' ').replace('\r',' ').replace('\0','.'))
        if hits or pat.search(n.encode('latin1')):
            print(os.path.basename(path),'|',n,'| comp' if comp else '| raw', '|', sorted(hits)[:20])
            for c in ctx: print('    ',repr(c))
