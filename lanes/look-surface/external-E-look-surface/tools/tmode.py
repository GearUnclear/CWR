import re,sys
from pathlib import Path
import numpy as np
from PIL import Image
PAT=re.compile(r"^\d+_(?P<view>.+?)__(?P<mode>[a-z]+)(?P<r>\d)\.png$")
d={}
for p in Path(sys.argv[1]).rglob('*.png'):
    m=PAT.match(p.name)
    if m: d[(m['view'],m['mode'],int(m['r']))]=p
def L(p):
    a=np.asarray(Image.open(p).convert('RGB')).astype(float); return a[...,0]*.2126+a[...,1]*.7152+a[...,2]*.0722
print("| view | mode | rounds 1-2 whole / bottom third | rounds 2-3 whole / bottom third |")
print("|---|---|---|---|")
for v in sorted({k[0] for k in d}):
    for mode in ['base','surf','nocav','aoonly']:
        out=[]
        for r1,r2 in [(1,2),(2,3)]:
            if (v,mode,r1) in d and (v,mode,r2) in d:
                a,b=L(d[(v,mode,r1)]),L(d[(v,mode,r2)]); h=a.shape[0]
                out.append(f"{np.abs(a-b).mean():.2f} / {np.abs(a-b)[2*h//3:].mean():.2f}")
        if out: print(f"| {v} | {mode} | "+" | ".join(out)+" |")
