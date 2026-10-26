import sys,struct
p=sys.argv[1]
b=open(p,'rb').read()
i=0
ents=[]
def rz():
    global i
    j=b.index(b'\0',i); s=b[i:j]; i=j+1; return s
while True:
    name=rz()
    meth,orig,res,ts,size=struct.unpack('<5I',b[i:i+20]); i+=20
    if name==b'' and meth==0x56657273:
        while True:
            k=rz()
            if k==b'': break
            v=rz(); print('EXT',k,v)
        continue
    if name==b'': break
    ents.append((name.decode('latin1'),meth,orig,ts,size))
import datetime
off=i
for n,m,o,t,s in ents:
    print(f"{n:50s} meth={m:#x} orig={o} size={s} ts={datetime.datetime.utcfromtimestamp(t) if t else 0} off={off}")
    off+=s
