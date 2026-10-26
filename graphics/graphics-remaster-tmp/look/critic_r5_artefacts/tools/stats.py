import numpy as np, glob, os
from PIL import Image
D='D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/'
def st(p):
    if not os.path.exists(p): return None
    a=np.asarray(Image.open(p).convert('RGB')).astype(float); l=a@[.2126,.7152,.0722]
    return l.mean(), (a.min(-1)>=254).mean()*100, (l<8).mean()*100
views=sorted(set(os.path.basename(p).replace('__overhaul.png','') for p in glob.glob(D+'director/final/*__overhaul.png')))
print(f"{'view':34s} {'r3':>6s} {'r4':>6s} {'r5':>6s} {'landed':>6s} {'vanil':>6s} | r5 clip% r5 black%")
for v in views:
    r=[]
    for p in [D+'director/round3/final/'+v+'__overhaul.png',D+'director/round4/final/'+v+'__overhaul.png',D+'director/final/'+v+'__overhaul.png',D+'ref/landed/'+v.replace('__1080','__1080')+'.png',D+'ref/vanilla/'+v+'.png']:
        r.append(st(p))
    f=lambda x: f"{x[0]:6.1f}" if x else '     -'
    print(f"{v:34s} {f(r[0])} {f(r[1])} {f(r[2])} {f(r[3])} {f(r[4])} | {r[2][1]:.4f} {r[2][2]:.2f}")
