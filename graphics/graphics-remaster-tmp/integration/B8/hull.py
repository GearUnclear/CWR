import sys, glob
import numpy as np
from PIL import Image
L = np.array([0.2126, 0.7152, 0.0722])
box = (1270, 455, 1500, 525)  # M113 side, frame px (x0, y0, x1, y1)
for run in sys.argv[1:]:
    for p in sorted(glob.glob(run + '/tri/*/*_b8_lowsun_fp__*.png')):
        a = np.asarray(Image.open(p).convert('RGB')).astype(np.float64)
        s = a[box[1]:box[3], box[0]:box[2]]
        print(f"{p.split('__')[-1][:-4]:9s} {run.split('/')[-1][:22]} luma {(s*L).sum(axis=2).mean():6.2f} R/B {s[...,0].mean()/s[...,2].mean():.3f} R/G {s[...,0].mean()/s[...,1].mean():.3f}")
