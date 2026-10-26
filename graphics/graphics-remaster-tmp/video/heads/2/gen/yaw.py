"""yaw.py <frames dir> <label glob> x0,y0,x1,y1 ...: a head-yaw proxy per frame
= horizontal offset of the face (skin) centroid from the helmet centroid in the
head crop, px. Reports per man: yaw range, per-frame |dyaw| > 1.5 px count
(turn events) and direction flips (consecutive turn events of opposite sign
within 10 frames = a left-right snap)."""
import sys, glob, re
import numpy as np
from PIL import Image
d, pat = sys.argv[1], sys.argv[2]
boxes = [tuple(int(v) for v in b.split(",")) for b in sys.argv[3:]]
files = sorted(glob.glob(f"{d}/**/*{pat}*.png", recursive=True), key=lambda f: re.search(r"__f(\d+)", f).group(1))
Y = [[] for _ in boxes]
for f in files:
    a = np.asarray(Image.open(f).convert("RGB"), dtype=np.float32)
    for bi, (x0, y0, x1, y1) in enumerate(boxes):
        c = a[y0:y1, x0:x1]
        r, g, b = c[..., 0], c[..., 1], c[..., 2]
        skin = (r > g + 12) & (r > b + 12) & (r > 90)
        helm = (g >= r - 4) & (g > b) & (r + g + b < 330) & (r + g + b > 60)
        xs = np.arange(c.shape[1])[None, :].repeat(c.shape[0], 0)
        if skin.sum() < 15 or helm.sum() < 15:
            Y[bi].append(np.nan); continue
        Y[bi].append(float(xs[skin].mean() - xs[helm].mean()))
for bi, y in enumerate(Y):
    y = np.array(y)
    dy = np.diff(y)
    ev = [(i, v) for i, v in enumerate(dy) if abs(v) > 1.5]
    flips = sum(1 for (i1, v1), (i2, v2) in zip(ev, ev[1:]) if np.sign(v1) != np.sign(v2) and i2 - i1 <= 10)
    print(f"box{bi}: yaw proxy range {np.nanmin(y):.1f}..{np.nanmax(y):.1f} px, turn events {len(ev)}, left-right flips {flips}, events at {[i for i, _ in ev][:20]}")
