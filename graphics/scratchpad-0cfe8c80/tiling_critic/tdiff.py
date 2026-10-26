import sys, os
from PIL import Image
import numpy as np
base = r"D:\Arma_CWA_worktrees\lighting\tmp\research\tiling"
def load(p):
    return np.asarray(Image.open(p).convert("RGB")).astype(np.float32)/255.0
def luma(a): return (a*[0.2126,0.7152,0.0722]).sum(-1)
for run, pairs in [("run6",[("v0","v6"),("v0","v4"),("v0","classic")]),("run5",[("v0","v1"),("v0","v4")]),("run2",[("v0","v6")])]:
    cmp = os.path.join(base, run, "cmp")
    views = sorted(set(f.split("__")[0] for f in os.listdir(cmp) if "__" in f))
    for v in views:
        for a,b in pairs:
            pa = os.path.join(cmp, f"{v}__{a}.png"); pb = os.path.join(cmp, f"{v}__{b}.png")
            if not (os.path.exists(pa) and os.path.exists(pb)): continue
            A = load(pa); B = load(pb)
            if A.shape != B.shape:
                print(run, v, a, b, "shape", A.shape, B.shape); continue
            d = np.abs(A-B).max(-1)
            h = A.shape[0]
            g = slice(h//2, h)
            la, lb = luma(A[g]), luma(B[g])
            print(f"{run} {v:20s} {a}->{b:8s} shape={A.shape[:2]} meanAbs={np.abs(A-B).mean()*255:6.2f} frac>4={(d>4/255).mean()*100:5.1f}% lowerHalfLuma {la.mean():.3f}->{lb.mean():.3f} ({(lb.mean()/la.mean()-1)*100:+.1f}%) topSixthDiff={np.abs(A[:h//6]-B[:h//6]).mean()*255:.2f}")
