"""Plume and fire-pool readings for the B4 review fixes.

    python plume.py <png> [<png> ...]

Per image (1920x1080): mean luminance (sRGB levels, Rec.709 weights) of
  plume: the wreck's smoke column above its base, rows 60-300, cols 1250-1500
         (the reviewer's region);
  sky:   the night sky beside it, rows 60-300, cols 1650-1900 and 850-1100;
  base:  the column's foot over the wreck, rows 380-470, cols 1250-1450;
and the reviewer's camp-fire pool crop (rows 560-700, cols 150-700, pixels
with luminance above 25): its red over blue.
"""
import sys

import numpy as np
from PIL import Image


def lum(a):
    return 0.2126 * a[..., 0] + 0.7152 * a[..., 1] + 0.0722 * a[..., 2]


for p in sys.argv[1:]:
    a = np.asarray(Image.open(p).convert("RGB")).astype(np.float64)
    L = lum(a)
    plume = L[60:300, 1250:1500].mean()
    sky = 0.5 * (L[60:300, 1650:1900].mean() + L[60:300, 850:1100].mean())
    base = L[380:470, 1250:1450].mean()
    pool = a[560:700, 150:700]
    pl = lum(pool)
    sel = pl > 25
    rb = pool[..., 0][sel].mean() / max(pool[..., 2][sel].mean(), 1e-6) if sel.sum() > 100 else float("nan")
    name = p.replace("\\", "/").split("/")[-1]
    print(f"{name:42s} plume {plume:5.1f} sky {sky:5.1f} base {base:5.1f} | pool R/B {rb:4.2f} (lit {sel.mean():.2f})")
