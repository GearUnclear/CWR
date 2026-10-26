"""kx-nu power image of a static-camera clip's fluctuation (agent 1, fogwind).
usage: python kxnu.py out.png clip1.mkv:LABEL [clip2 ...] [--from 15] [--region y0,y1,x0,x1]
Per clip a panel: horizontal axis kx (cycles/cell, -2..2, 12 px cells), vertical axis nu (cycles/frame,
-0.5 top .. +0.5 bottom), log power summed over ky, the temporal mean removed. A pattern travelling at
v cells/frame lies on the line nu = -v kx (a slanted line through the centre). In-place flicker lies on
a horizontal spread at kx ~ 0; static content on nu = 0.
Also prints the share of mid-band (0.25 < |kx| < 1.5 cyc/cell) energy at nu != 0 lying in the left- vs
right-travelling half-planes (kx*nu > 0 means travelling -x).
"""
import sys, os
import numpy as np
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fogmotion as fm
from PIL import Image, ImageDraw
args = sys.argv[1:]
a = 15; region = "0.47,0.70,0.66,0.97"
if "--from" in args:
    i = args.index("--from"); a = int(args[i + 1]); del args[i:i + 2]
if "--region" in args:
    i = args.index("--region"); region = args[i + 1]; del args[i:i + 2]
out, specs = args[0], args[1:]
y0, y1, x0, x1 = (float(v) for v in region.split(","))
box = (int(y0 * 1080), int(y1 * 1080), int(x0 * 1920), int(x1 * 1920))
panels = []
for spec in specs:
    src, lab = spec.rsplit(":", 1)
    x = fm.decode(src, a, 0, box)
    T, H, W = x.shape
    f = x - x.mean(axis=0, keepdims=True)
    win = np.hanning(T)[:, None, None] * np.hanning(W)[None, None, :]
    F = np.fft.fft(np.fft.fft(f * win, axis=0), axis=2)
    P = (np.abs(F) ** 2).sum(axis=1)  # T x W
    nu = np.fft.fftfreq(T); kx = np.fft.fftfreq(W) * 12
    NU, KX = np.meshgrid(nu, kx, indexing="ij")
    mid = (np.abs(KX) > 0.25) & (np.abs(KX) < 1.5) & (NU != 0)
    left = P[mid & (KX * NU > 0)].sum(); right = P[mid & (KX * NU < 0)].sum()
    slow = P[mid & (np.abs(NU) < 0.1)].sum() / P[mid].sum()
    # energy-weighted |v| for mid band
    v = -NU[mid] / KX[mid]
    hist, edges = np.histogram(np.clip(v, -4, 4), bins=16, range=(-4, 4), weights=P[mid])
    hist /= hist.sum()
    print(f"{lab}: mid-band energy {P[mid].sum():.3g}  travelling -x share {left / (left + right):.2f}  slow(|nu|<0.1) share {slow:.2f}")
    print("   v (cells/frame) energy histogram: " + " ".join(f"{(edges[i] + edges[i + 1]) / 2:+.1f}:{hist[i]:.2f}" for i in range(16)))
    S = np.fft.fftshift(P)
    keep = (np.abs(np.fft.fftshift(kx)) <= 2.0)
    S = S[:, keep]
    S = np.log10(S + 1e-6)
    lo, hi = np.percentile(S, 30), S.max()
    img = (np.clip((S - lo) / (hi - lo), 0, 1) * 255).astype(np.uint8)
    im = Image.fromarray(img).resize((img.shape[1] * 2, 300), Image.NEAREST).convert("RGB")
    d = ImageDraw.Draw(im); d.text((4, 2), lab, fill=(255, 255, 0))
    panels.append(im)
Wt = sum(p.width for p in panels) + 8 * len(panels)
o = Image.new("RGB", (Wt, 300)); xo = 0
for p in panels:
    o.paste(p, (xo, 0)); xo += p.width + 8
o.save(out)
