import sys, numpy as np
from PIL import Image
d = r'D:/Arma_CWA_worktrees/graphics-remaster/tmp/integration/A7/final/'
views = ['coastline_sunset','firefight_dusk','sunrise_ridge','noon_fields','coast_oc_up','pillarbox','town_night']
def lum(a): return (0.2126*a[...,0]+0.7152*a[...,1]+0.0722*a[...,2])
for v in views:
    off = np.asarray(Image.open(d+f'atmosphere_{v}__off.png').convert('RGB')).astype(np.float64)
    on = np.asarray(Image.open(d+f'atmosphere_{v}__on.png').convert('RGB')).astype(np.float64)
    H,W,_ = off.shape
    print(f'== {v} {W}x{H}')
    for name,(r0,r1) in {'sky0-380':(0,380),'band380-520':(380,520),'ground520-1080':(520,1080)}.items():
        for side,(c0,c1) in {'L':(0,W//3),'M':(W//3,2*W//3),'R':(2*W//3,W)}.items():
            a=off[r0:r1,c0:c1].reshape(-1,3).mean(0); b=on[r0:r1,c0:c1].reshape(-1,3).mean(0)
            print(f'  {name:15s}{side} off {a.round(0)} on {b.round(0)}  lum {lum(a):.1f}->{lum(b):.1f} ({(lum(b)/max(lum(a),1e-6)-1)*100:+.1f}%)')
