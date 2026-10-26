import sys,struct,json,collections,hashlib
sys.path.insert(0,r'D:/Arma_CWA/.claude/worktrees/lobo-repair-4/arma_CWR/tools/lobo')
from repair.pbo import Archive
from pathlib import Path
here=Path(__file__).parent
idx=json.loads((here/'lobo-index.json').read_text());idx.update(json.loads((here/'base-index.json').read_text()))
tools=r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe'
def lods(data):
    sig,ver,count=struct.unpack_from('<4sII',data);pos=12;out=[]
    for i in range(count):
        s,header,v,points,normals,faces,flags=struct.unpack_from('<4s6i',data,pos)
        ft=pos+header+points*16+normals*12
        fields=[data[ft+j*104:ft+j*104+32] for j in range(faces)]
        pos=ft+faces*104+4
        while True:
            raw,size=struct.unpack_from('<64si',data,pos);name=raw.split(b'\0',1)[0];pos+=68+size
            if name==b'#EndOfFile#':break
        res=struct.unpack_from('<f',data,pos)[0];pos+=4
        out.append((res,fields))
    return out,data[pos:]
def resolve(n):
    s=n.decode('latin1').replace(chr(92),'/').lower().lstrip('/')
    return s in idx
path=Path(sys.argv[1]);a=Archive(path.read_bytes())
only_missing='--missing' in sys.argv
filt=[x.lower().encode() for x in sys.argv[3:] if not x.startswith('--')]
for k in sys.argv[2].split(','):
    d=a.read(k,path,tools)
    L,tail=lods(d)
    print(f'===== {path.stem}/{k} bytes={len(d)} sha256={hashlib.sha256(d).hexdigest()} lods={len(L)} tail={tail[:6]!r}')
    for res,fields in L:
        c=collections.Counter(f.split(b'\0',1)[0] if b'\0' in f else f+b'<RAW32>' for f in fields)
        rows=[(n,v) for n,v in sorted(c.items()) if (not filt or any(x in n.lower() for x in filt)) and (not only_missing or (n and not resolve(n)))]
        if rows:
            print(f'  LOD {res:g} faces={len(fields)}')
            for n,v in rows:print(f'    {v:5d} {"ok " if resolve(n) else "MISSING" if n else "-"} {n.decode("latin1")}')
