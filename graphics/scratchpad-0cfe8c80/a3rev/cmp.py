import sys
from pathlib import Path
import numpy as np
from PIL import Image
a, b = Path(sys.argv[1]), Path(sys.argv[2])
views = sys.argv[3].split(",")
for v in views:
    for mode in ("off", "on"):
        fa = sorted((a / "tri").rglob(f"*_{v}__{mode}.png"))
        fb = sorted((b / "tri").rglob(f"*_{v}__{mode}.png"))
        if not fa or not fb:
            print(v, mode, "missing", fa[:1], fb[:1]); continue
        x = np.asarray(Image.open(fa[0]).convert("RGB")).astype(int)
        y = np.asarray(Image.open(fb[0]).convert("RGB")).astype(int)
        if x.shape != y.shape:
            print(v, mode, "shape", x.shape, y.shape); continue
        d = np.abs(x - y).max(axis=2)
        print(f"{v:32s} {mode:3s} {x.shape[1]}x{x.shape[0]} max={d.max()} px>0={(d>0).sum()}")
