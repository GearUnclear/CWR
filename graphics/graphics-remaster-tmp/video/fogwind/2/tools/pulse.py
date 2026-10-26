"""Agent 2: per-frame camera-compensated change in small boxes of a moving clip (finds periodic pulses).
usage: python pulse.py clip.mkv [...] [--box x,y,w,h] [--from 15]
Prints per clip: rms of D_t (camera-compensated frame difference, 6 px box blurred) in the box per frame,
its mean, and the autocorrelation peak lag in 5..40 frames (a periodic pulse shows a strong peak).
"""
import argparse, os, sys
import numpy as np
from scipy import ndimage
sys.path.insert(0, "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/1/kit")
import fogmotion as fm
from xtslice import cam_shifts, resid

ap = argparse.ArgumentParser()
ap.add_argument("clips", nargs="+")
ap.add_argument("--box", default="1700,600,200,120")
ap.add_argument("--from", dest="a", type=int, default=15)
ap.add_argument("--csv", default="")
o = ap.parse_args()
x, y, w, h = (int(v) for v in o.box.split(","))
rows = {}
for c in o.clips:
    st = fm.decode(c, o.a, None, (y - 40, y + h + 40, x - 40, x + w + 40))
    sh = cam_shifts(st)
    r = resid(st, sh)
    r = ndimage.uniform_filter(r, size=(1, 6, 6))[:, 40:-40, 40:-40]
    e = np.sqrt((r ** 2).mean(axis=(1, 2)))
    z = e - e.mean()
    ac = np.array([np.dot(z[:-L], z[L:]) / (np.dot(z, z) + 1e-9) for L in range(1, 41)])
    L = int(np.argmax(ac[4:]) + 5)
    print("%s  mean %.3f  max %.3f  p90 %.3f  autocorr peak lag %d (%.2f)" % (c, e.mean(), e.max(), np.percentile(e, 90), L, ac[L - 1]))
    print("   per frame:", " ".join("%.2f" % v for v in e))
    rows[c] = e
