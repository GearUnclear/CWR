import sys, os, glob
import numpy as np
from PIL import Image
old, new = sys.argv[1], sys.argv[2]
def load(p):
    return np.asarray(Image.open(p).convert('RGB')).astype(np.float32)
def luma(a):
    return 0.2126*a[...,0]+0.7152*a[...,1]+0.0722*a[...,2]
views = sorted(set(os.path.basename(p).rsplit('__',1)[0] for p in glob.glob(os.path.join(new,'*__remastered.png'))+glob.glob(os.path.join(new,'*__classic.png'))))
print(f"{'view':28s} {'mode':11s} {'oldY':>6s} {'newY':>6s} {'dY':>6s} {'mad':>6s} {'%>8':>6s} {'lowY_old':>8s} {'lowY_new':>8s}")
for v in views:
    for m in ('classic','remastered'):
        po=os.path.join(old,f'{v}__{m}.png'); pn=os.path.join(new,f'{v}__{m}.png')
        if not (os.path.exists(po) and os.path.exists(pn)):
            print(f"{v:28s} {m:11s} missing old={os.path.exists(po)} new={os.path.exists(pn)}"); continue
        a=load(po); b=load(pn)
        if a.shape!=b.shape:
            print(f"{v:28s} {m:11s} shape {a.shape} vs {b.shape}"); continue
        d=np.abs(a-b).max(axis=2)
        ya=luma(a); yb=luma(b); h=a.shape[0]; lo=int(h*0.72)
        print(f"{v:28s} {m:11s} {ya.mean():6.1f} {yb.mean():6.1f} {yb.mean()-ya.mean():+6.1f} {np.abs(a-b).mean():6.2f} {100*(d>8).mean():6.2f} {ya[lo:].mean():8.1f} {yb[lo:].mean():8.1f}")
