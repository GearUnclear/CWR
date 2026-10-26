"""B4 levels: mean luma of the frame and lower third, and R/B (mean R over
mean B) of the lower third, per <view>__{classic,off,on}.png.

    python lv.py <dir> [views,comma,separated]
"""
import sys
from pathlib import Path
from PIL import Image
import numpy as np


def load(p):
    return np.asarray(Image.open(p).convert("RGB")).astype(np.float32)


def luma(a):
    return 0.2126 * a[..., 0] + 0.7152 * a[..., 1] + 0.0722 * a[..., 2]


d = Path(sys.argv[1])
views = sys.argv[2].split(",") if len(sys.argv) > 2 else sorted({p.name.rsplit("__", 1)[0] for p in d.glob("*__on.png")})
for v in views:
    row = [f"{v:22s}"]
    for mode in ("classic", "off", "on"):
        p = d / f"{v}__{mode}.png"
        if not p.exists():
            continue
        a = load(p)
        h = a.shape[0]
        low = a[2 * h // 3:]
        rb = low[..., 0].mean() / max(low[..., 2].mean(), 1e-3)
        row.append(f"{mode}: all {luma(a).mean():5.1f} low {luma(low).mean():5.1f} R/B {rb:4.2f}")
    print("  ".join(row))
