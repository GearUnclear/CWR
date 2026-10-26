"""Characterise a travelling residual on a STATIC-camera clip (agent 1, fogwind).
usage: python windchar.py clip.mkv [--from 15] [--region y0,y1,x0,x1] [--ref frozen.mkv]
D_t = I(t+1) - I(t), mid band (DoG 1.5..12 px). Prints:
  - the 2D power spectrum peak(s) of D (spatial period in px and cells, orientation), minus --ref's
  - for lags L = 1..6 frames: the mean normalised xcorr surface of D_t vs D_{t+L}: peak lag and value
    (a coherent travel at v gives peaks at L*v; an alias / lattice effect does not scale with L)
"""
import sys, os
import numpy as np
from scipy import ndimage
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fogmotion as fm
args = sys.argv[1:]
a = 15; region = "0.47,0.70,0.62,0.97"; ref = None
for k in ("--from", "--region", "--ref"):
    if k in args:
        i = args.index(k); v = args[i + 1]; del args[i:i + 2]
        if k == "--from": a = int(v)
        elif k == "--region": region = v
        else: ref = v
y0, y1, x0, x1 = (float(v) for v in region.split(","))
box = (int(y0 * 1080), int(y1 * 1080), int(x0 * 1920), int(x1 * 1920))
def mid(d): return ndimage.gaussian_filter(d, 1.5) - ndimage.gaussian_filter(d, 12.0)
def spec(src):
    x = fm.decode(src, a, 0, box)
    D = np.stack([mid(x[t + 1] - x[t]) for t in range(len(x) - 1)])
    T, H, W = D.shape
    win = fm.hann(H, W)
    P = np.zeros((H, W))
    for d in D:
        P += np.abs(np.fft.fft2(d * win)) ** 2
    return D, P / T
D, P = spec(sys.argv[1] if False else args[0])
T, H, W = D.shape
if ref:
    _, Pr = spec(ref)
    Pn = np.clip(P - Pr, 0, None)
else:
    Pn = P
ky = np.fft.fftfreq(H); kx = np.fft.fftfreq(W)
KY, KX = np.meshgrid(ky, kx, indexing="ij")
Ps = ndimage.uniform_filter(Pn, 3)
Ps[np.hypot(KX, KY) < 1 / 200] = 0
order = np.argsort(Ps.ravel())[::-1]
print(f"{args[0]} frames {T + 1} region {H}x{W}; D mid rms {np.sqrt((D ** 2).mean()):.3f}")
seen = []
for i in order:
    iy, ix = np.unravel_index(i, Ps.shape)
    fx, fy = kx[ix], ky[iy]
    if fx < 0 or (fx == 0 and fy < 0):
        fx, fy = -fx, -fy
    if any(np.hypot(fx - s[0], fy - s[1]) < 0.01 for s in seen):
        continue
    seen.append((fx, fy))
    f = np.hypot(fx, fy)
    ang = np.degrees(np.arctan2(fy, fx))
    print(f"  spectral peak: k=({fx:+.4f},{fy:+.4f}) cyc/px (y down) period {1 / f:.1f} px = {12 * f and 1 / (12 * f):.2f} cells, "
          f"wave-vector angle {ang:+.0f} deg (crests run at {ang + 90:+.0f} deg), power share {Ps[iy, ix] / Ps.sum() * 9:.4f}")
    if len(seen) >= 5:
        break
LG = 40
inner = (slice(40, H - 40), slice(40, W - 40))
for L in range(1, 7):
    s = np.zeros((2 * LG + 1, 2 * LG + 1))
    for t in range(T - L):
        s += fm.xcorr_local(D[t][inner], D[t + L][inner], LG)
    s /= (T - L)
    iy, ix = np.unravel_index(np.argmax(s), s.shape)
    print(f"  lag {L} frames: best shift ({ix - LG:+d},{iy - LG:+d}) px  corr {s[iy, ix]:+.3f}   corr at 0: {s[LG, LG]:+.3f}  at (-12L,0): {s[LG, LG - 12 * L] if 12 * L <= LG else float("nan"):+.3f}")
