import sys, math, colorsys
import numpy as np
from PIL import Image
im = np.asarray(Image.open(sys.argv[1]).convert("RGB")).astype(float)/255
H,W,_ = im.shape
mx = im.max(2); mn = im.min(2)
sat = np.where(mx>0, (mx-mn)/np.maximum(mx,1e-6), 0)
mask = (sat>0.22) & (mx>0.55)
ys, xs = np.nonzero(mask)
print("golden px", len(xs), "bbox x", xs.min(), xs.max(), "y", ys.min(), ys.max())
# camera: heading 69.6, pitch -4, eye 1.7, fov 0.7 (tan half-width), jeep 8 fwd 2.5 right facing -75 rel
fov=0.7; aspect=H/W
eye=1.7; pitch=math.radians(-4)
def ground(x,y):
    u=(x/(W/2)-1)*fov; v=-(y/(H/2)-1)*fov*aspect
    # camera space: forward z, right x, up y
    d=np.array([u, v, 1.0])
    # pitch rotation about x
    cy, sy = math.cos(pitch), math.sin(pitch)
    dy = d[1]*cy + d[2]*sy; dz = -d[1]*sy + d[2]*cy
    dx = d[0]
    if dy >= 0: return None
    t = eye/(-dy)
    return np.array([dx*t, dz*t])  # right, forward
jeep = np.array([2.5, 8.0])
hd = math.radians(-75)
fwd = np.array([math.sin(hd), math.cos(hd)])
rgt = np.array([math.cos(hd), -math.sin(hd)])
pts=[]
for x,y in zip(xs[::50], ys[::50]):
    g=ground(x,y)
    if g is None: continue
    r = g - jeep
    pts.append((float(r@fwd), float(r@rgt)))
pts=np.array(pts)
print("ahead of jeep m: p10 %.1f p50 %.1f p90 %.1f" % tuple(np.percentile(pts[:,0],[10,50,90])))
print("side of jeep m:  p10 %.1f p50 %.1f p90 %.1f" % tuple(np.percentile(pts[:,1],[10,50,90])))
m = im[mask].mean(0)
h,s,v = colorsys.rgb_to_hsv(*m); print("mean golden hue %.1f sat %.3f val %.1f" % (h*360, s, v*255))
