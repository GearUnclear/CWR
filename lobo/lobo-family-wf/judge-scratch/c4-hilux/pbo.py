import sys, struct, os, glob, re
def read_cstr(f):
    b=bytearray()
    while True:
        c=f.read(1)
        if not c or c==b'\0': return bytes(b)
        b+=c
def entries(path):
    out=[]
    with open(path,'rb') as f:
        first=True
        while True:
            name=read_cstr(f)
            hdr=f.read(20)
            if len(hdr)<20: break
            packing,orig,res,ts,size=struct.unpack('<5I',hdr)
            if first and name==b'' and packing==0x56657273:
                # product entry: read key/value pairs
                while True:
                    k=read_cstr(f)
                    if k==b'': break
                    read_cstr(f)
                first=False
                continue
            first=False
            if name==b'': break
            out.append((name.decode('latin1'),packing,orig,size))
        base=f.tell()
    return out,base
if __name__=='__main__':
    for p in sys.argv[1:]:
        es,base=entries(p)
        print(p,len(es))
