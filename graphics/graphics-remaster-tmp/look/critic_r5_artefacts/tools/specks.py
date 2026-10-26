# specks.py img... : isolated green / dark pixels off the 5x5 per-channel median, where the median is not dark (mist, not the sight mask)
import sys, numpy as np
from PIL import Image
from scipy.ndimage import median_filter
for p in sys.argv[1:]:
    a=np.asarray(Image.open(p).convert('RGB')).astype(int)
    m=np.stack([median_filter(a[...,c],5) for c in range(3)],-1)
    lm=m.mean(-1); l=a.mean(-1)
    bright=lm>60
    dark=bright&(l<lm-45)
    g=a[...,1]-np.maximum(a[...,0],a[...,2]); gm=m[...,1]-np.maximum(m[...,0],m[...,2])
    green=bright&(g-gm>25)&(np.abs(a-m).max(-1)>30)
    purple=bright&((a[...,0]+a[...,2])/2-a[...,1]-((m[...,0]+m[...,2])/2-m[...,1])>25)
    ys,xs=np.nonzero(dark|green)
    print(f"{p.split('/')[-3]}/{p.split('/')[-1]}: dark {dark.sum()} green {green.sum()} purple {purple.sum()}", (list(zip(xs[:6],ys[:6])) if len(xs) else ''))
