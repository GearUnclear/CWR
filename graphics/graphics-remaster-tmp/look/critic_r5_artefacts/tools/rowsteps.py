# rows where a wide horizontal step (median over x of |dL/dy|) is new in r5 vs r4
import numpy as np, glob, os
from PIL import Image
D='D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/director/'
def prof(p):
    a=np.asarray(Image.open(p).convert('RGB')).astype(float)@[.2126,.7152,.0722]
    dy=np.abs(np.diff(a,axis=0))
    return np.median(dy,axis=1), np.percentile(dy,75,axis=1)
for p in sorted(glob.glob(D+'final/*__overhaul.png')):
    v=os.path.basename(p); q=D+'round4/final/'+v
    if not os.path.exists(q): continue
    m5,p5=prof(p); m4,p4=prof(q)
    bad=[(y,round(m5[y],1),round(m4[y],1)) for y in range(len(m5)) if m5[y]>=4 and m5[y]>2*m4[y]+1]
    print(v, 'new wide steps:', bad[:8])
