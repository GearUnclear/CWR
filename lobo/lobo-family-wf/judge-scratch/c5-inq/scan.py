import sys, os, struct, re, glob
def lzss(data, outlen):
    out=bytearray(); i=0
    while len(out)<outlen and i<len(data):
        flags=data[i]; i+=1
        for b in range(8):
            if len(out)>=outlen or i>=len(data): break
            if flags & (1<<b):
                out.append(data[i]); i+=1
            else:
                if i+1>=len(data): break
                b1=data[i]; b2=data[i+1]; i+=2
                rpos=b1 | ((b2&0xF0)<<4); rlen=(b2&0x0F)+3
                start=len(out)-rpos
                for k in range(rlen):
                    p=start+k
                    out.append(out[p] if 0<=p<len(out) else 0x20)
    return bytes(out)
def parse(path):
    f=open(path,'rb').read(); pos=0; entries=[]
    while True:
        e=f.index(b'\0',pos); name=f[pos:e].decode('latin1'); pos=e+1
        method,orig,res,ts,size=struct.unpack_from('<5I',f,pos); pos+=20
        if name=='' :
            if method==0x56657273:
                while True:
                    e=f.index(b'\0',pos); k=f[pos:e]; pos=e+1
                    if k==b'': break
                    e=f.index(b'\0',pos); pos=e+1
                continue
            break
        entries.append((name,method,orig,size))
    data=pos; out=[]
    for name,method,orig,size in entries:
        raw=f[data:data+size]; data+=size
        if method==0x43707273 or (orig and orig!=size):
            try: raw=lzss(raw,orig)
            except Exception as ex: raw=raw
        out.append((name,raw))
    return out
pat=re.compile(rb'inq',re.I)
files=sys.argv[1:]
for p in files:
    try: ents=parse(p)
    except Exception as ex:
        print('PARSEFAIL',p,ex); continue
    for name,raw in ents:
        if pat.search(name.encode('latin1')): print('NAME',p,name)
        for m in pat.finditer(raw):
            ctx=raw[max(0,m.start()-40):m.end()+40]
            # filter obvious english words
            s=ctx.decode('latin1','replace')
            low=s.lower()
            if any(w in low for w in ['inquir','inqu ']): pass
            print('HIT',os.path.basename(p),name,repr(s))
