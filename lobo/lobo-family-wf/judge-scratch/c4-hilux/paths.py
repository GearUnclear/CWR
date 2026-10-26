import sys, os, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pbo import entries
from scan import lzss
p=sys.argv[1]; want=[w.lower() for w in sys.argv[2:]]
es,base=entries(p)
data=open(p,'rb').read(); off=base
print('ENTRIES:',len(es))
for name,packing,orig,size in es:
    blob=data[off:off+size]; off+=size
    if want and not any(w in name.lower() for w in want): continue
    if packing==0x43707273 or (orig and orig!=size and packing!=0):
        blob=lzss(blob,orig)
    strs=set(m.group().decode('latin1') for m in re.finditer(rb'[A-Za-z0-9_\\.\-]{3,}\.(?:paa|pac|p3d|wav|ogg|sqs|sqf|rtm|jpg|tga)', blob, re.I))
    pref={}
    for s in strs:
        k=s.lower().split(chr(92))[0] if chr(92) in s else '(nopath)'
        pref.setdefault(k,[]).append(s)
    print('--',name,'packed' if packing else '', {k:len(v) for k,v in pref.items()})
    for k,v in pref.items():
        if k not in ('lobotoy','loboter'):
            print('    ',k, sorted(v)[:15])
