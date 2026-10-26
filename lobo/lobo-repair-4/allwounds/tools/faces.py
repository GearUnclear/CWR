import sys,struct,collections,hashlib,json
sys.path.insert(0,r'D:/Arma_CWA/.claude/worktrees/lobo-repair-4/arma_CWR/tools/lobo')
from repair.pbo import Archive
from pathlib import Path
TOOLS=r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe'
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
    return out
path=Path(sys.argv[1]);a=Archive(path.read_bytes())
members=set(a.by_key)
result={}
for k in sorted(x for x in a.by_key if x.endswith('.p3d')):
    d=a.read(k,path,TOOLS)
    L=lods(d)
    per={}
    for res,fields in L:
        c=collections.Counter(f.split(b'\0',1)[0].decode('latin1') for f in fields)
        per[repr(res)]=dict(sorted(c.items()))
    result[k]=per
json.dump(result,open('faces.json','w'),indent=1)
# summary: union of lobo_pilot textures per model, with LODs
for k,per in result.items():
    print('=====',k)
    tex=collections.defaultdict(list)
    for res,c in per.items():
        for n,v in c.items(): tex[n].append(f'{res}:{v}')
    for n in sorted(tex):
        if 'lobo_pilot' in n.lower():
            key=n.lower().replace(chr(92),'/').lstrip('/')[len('lobo_pilot/'):]
            print(f'  {"ok " if key in members else "MISSING"} {n}  [{", ".join(tex[n])}]')
