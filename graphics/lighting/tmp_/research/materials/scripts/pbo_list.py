import sys, struct
def entries(path):
    with open(path,'rb') as f:
        data=f.read(4*1024*1024)
    pos=0; out=[]
    while True:
        end=data.index(b'\0',pos); name=data[pos:end].decode('latin1'); pos=end+1
        method,orig,res,ts,size=struct.unpack_from('<5I',data,pos); pos+=20
        if name=='' and method==0x56657273:  # 'Vers' header
            # skip properties
            while True:
                e=data.index(b'\0',pos); k=data[pos:e]; pos=e+1
                if k==b'': break
                e=data.index(b'\0',pos); pos=e+1
            continue
        if name=='': break
        out.append((name,size))
    return out
for p in sys.argv[1:]:
    for n,s in entries(p): print(n, s)
