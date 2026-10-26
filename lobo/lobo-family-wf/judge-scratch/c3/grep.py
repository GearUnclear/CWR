import sys,re,struct
p=sys.argv[1]; pat=sys.argv[2].encode()
b=open(p,'rb').read()
i=0
def rz():
    global i
    j=b.index(b'\0',i); s=b[i:j]; i=j+1; return s
ents=[]
while True:
    name=rz()
    meth,orig,res,ts,size=struct.unpack('<5I',b[i:i+20]); i+=20
    if name==b'' and meth==0x56657273:
        while rz()!=b'': rz()
        continue
    if name==b'': break
    ents.append((name,meth,orig,size))
off=i
for n,m,o,s in ents:
    data=b[off:off+s]; off+=s
    comp = (m!=0 and o!=s)
    for mm in re.finditer(re.escape(pat),data,re.I):
        st=max(0,mm.start()-100); print(n.decode('latin1'),'compressed' if comp else '', repr(data[st:mm.end()+100]))
print('entries',len(ents),'compressed',sum(1 for e in ents if e[1]!=0 and e[2]!=e[3]))
