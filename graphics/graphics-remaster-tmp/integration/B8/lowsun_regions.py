"""Region stats for lowsun_fp review items.
python reg.py <run tri dir> [prefix]
"""
import sys
from pathlib import Path
import numpy as np
from PIL import Image
L = np.array([0.2126, 0.7152, 0.0722])
REG = {"hillside": (1450, 1920, 330, 480), "plain_sun": (0, 600, 470, 560)}
d = Path(sys.argv[1])
view = sys.argv[2] if len(sys.argv) > 2 else "b8_lowsun_fp"
shots = {p.stem.split("__")[1]: p for p in d.glob(f"*_{view}__*.png")}
def st(a, r):
    x0, x1, y0, y1 = r
    s = a[y0:y1, x0:x1].astype(np.float64)
    l = (s * L).sum(axis=2)
    return l.mean(), l.std(), s[..., 0].mean() / s[..., 2].mean()
base = np.asarray(Image.open(shots["off"]).convert("RGB"))
for name, r in REG.items():
    l0, s0, rb0 = st(base, r)
    print(f"{name}: off luma {l0:.1f} std {s0:.1f} R/B {rb0:.3f}")
    for k in sorted(shots):
        if k == "off":
            continue
        a = np.asarray(Image.open(shots[k]).convert("RGB"))
        ref = base
        if k == "onnotaa" and "offnotaa" in shots:
            ref = np.asarray(Image.open(shots["offnotaa"]).convert("RGB"))
        lr, sr, rbr = st(ref, r)
        l1, s1, rb1 = st(a, r)
        print(f"  {k:9s} luma {l1:6.1f} ({(l1-lr)/lr*100:+.1f}%) std {s1:5.1f} ({(s1-sr)/sr*100:+.1f}%) R/B {rbr:.3f}->{rb1:.3f}")
