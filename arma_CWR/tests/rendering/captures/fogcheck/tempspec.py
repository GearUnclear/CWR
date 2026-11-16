"""Temporal spectrum of the in-place change on a held-camera clip: python tempspec.py clip ... [--from 10]
Per region: mean power of the mid band (DoG 2..16 px) over time frequency; prints the ratio of the power at
1/8, 1/4, 3/8, 1/2 cycles/frame to the median of its neighbours (a cycle of 8 frames shows peaks there)."""
import sys, os
import numpy as np
from scipy import ndimage
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fogmotion as fm
args = sys.argv[1:]
a = 10
if "--from" in args:
    i = args.index("--from"); a = int(args[i + 1]); del args[i:i + 2]
for src in args:
    out = []
    for name in ("valley", "farhill", "haze", "bay"):
        y0, y1, x0, x1 = fm.REGIONS[name]
        x = fm.decode(src, a, a + 176, (int(y0 * 1080), int(y1 * 1080), int(x0 * 1920), int(x1 * 1920)))
        M = np.stack([ndimage.gaussian_filter(f, 2.0) - ndimage.gaussian_filter(f, 16.0) for f in x])
        M = M - M.mean(axis=0)
        P = (np.abs(np.fft.rfft(M * np.hanning(len(M))[:, None, None], axis=0)) ** 2).mean(axis=(1, 2))
        f = np.fft.rfftfreq(len(M))
        r = []
        for q in (0.125, 0.25, 0.375, 0.5):
            i = int(round(q * len(M)))
            nb = np.r_[P[max(i - 8, 1):i - 2], P[i + 3:i + 9]] if i + 3 < len(P) else P[i - 8:i - 2]
            r.append(P[max(i - 1, 0):i + 2].max() / np.median(nb))
        out.append(f"{name} " + "/".join(f"{v:.1f}" for v in r))
    print(f"{os.path.basename(src)}: cycle-8 peaks at 1/8,1/4,3/8,1/2: " + " | ".join(out))
