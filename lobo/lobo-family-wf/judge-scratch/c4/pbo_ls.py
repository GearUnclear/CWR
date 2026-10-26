import sys,struct,re
def read_pbo(path):
    data=open(path,'rb').read()
    pos=0; entries=[]
    def rdstr():
        nonlocal pos
        e=data.index(b'\0',pos); s=data[pos:e]; pos=e+1; return s
    while True:
        name=rdstr()
        mime,orig,res,ts,size=struct.unpack_from('<5I',data,pos); pos+=20
        if name==b'' and mime==0x56657273:
            while rdstr()!=b'': pass
            continue
        if name==b'': break
        entries.append((name.decode('latin1'),mime,orig,size))
    off=pos; out=[]
    for n,m,o,s in entries:
        out.append((n,m,o,s,data[off:off+s])); off+=s
    return out
for p in sys.argv[1:]:
    print('==',p)
    for n,m,o,s,b in read_pbo(p):
        print(f'{n}\tmime={m:x}\torig={o}\tsize={s}')
