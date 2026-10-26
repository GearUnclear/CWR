import numpy as np
from noise_scale import build
n=256
tex = build(256, 1).astype(np.float64)/255.0
def sample(u, v):
    x = u*n - 0.5; y = v*n - 0.5
    x0 = np.floor(x).astype(int); y0 = np.floor(y).astype(int)
    tx = x - x0; ty = y - y0
    t = lambda xx, yy: tex[yy % n, xx % n]
    a = t(x0,y0) + (t(x0+1,y0)-t(x0,y0))*tx
    b = t(x0,y0+1) + (t(x0+1,y0+1)-t(x0,y0+1))*tx
    return a + (b-a)*ty
def ss(e0,e1,x):
    t = np.clip((x-e0)/(e1-e0),0,1); return t*t*(3-2*t)
def S(e0,e1,x):
    t = min(1,max(0,(x-e0)/(e1-e0))); return t*t*(3-2*t)
keys=[(-0.08,1.0),(0.5,0.7),(0.66,0.1),(1.0,0.0)]
def through(o):
    for i in range(1,4):
        if o < keys[i][0]:
            t=(o-keys[i-1][0])/(keys[i][0]-keys[i-1][0]); return keys[i-1][1]+(keys[i][1]-keys[i-1][1])*t
    return 0.0
m=512
ii, jj = np.meshgrid(np.arange(m), np.arange(m))
nv = sample((ii+0.37)/m, (jj+0.71)/m)
for o in (0.0, 0.01, 0.02, 0.05, 0.1, 0.3, 0.5, 0.6, 0.66, 0.8, 1.0):
    c = 1 - through(o)
    s = c*S(0.05, 0.1, c)
    edge = 0.08*min(1,max(0,4*s*(1-s)))
    # the shader path is off when the rig hands it cover 0 (litCloud.w = 0)
    cov = 0.0 if s <= 0 else (ss(1-s-edge, 1-s+edge, nv).mean() if edge>1e-4 else (nv >= 1-s).mean())
    T=0.2
    classic = (1-c)*0.8+0.2
    fm = 1 - s*(1-T)
    od = min(1, classic/fm)
    mean = od*(1-cov*(1-T))
    print(f"overcast {o:.2f} through {1-c:.4f} cover {c:.4f} shadowCover {s:.4f} covered {cov:.4f} meanSun {mean:.4f} classic {classic:.4f} rel {mean/classic-1:+.4f} litgap {od:.4f}")
