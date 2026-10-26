import sys
from PIL import Image
import numpy as np
for v in sys.argv[1:]:
    a=np.asarray(Image.open(v).convert('RGB')).astype(float)
    l=a@[0.299,0.587,0.114]
    m=l>=l.max()-3
    ys,xs=np.where(m); cy,cx=int(np.median(ys)),int(np.median(xs))
    out=[]
    for r in [0,4,8,12,16,24,32,48,64,96,128,192]:
        pts=[(cy-r,cx),(cy,cx+r),(cy-int(r*.7),cx+int(r*.7))]
        vals=[a[p] for p in pts if 0<=p[0]<a.shape[0] and 0<=p[1]<a.shape[1]]
        out.append('%d:%s'%(r,','.join(str(int(x)) for x in np.mean(vals,axis=0))))
    print(v.split('/')[-2]+'/'+v.split('/')[-1],'c',cx,cy,' '.join(out))
