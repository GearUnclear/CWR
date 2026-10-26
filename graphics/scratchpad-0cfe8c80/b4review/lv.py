import sys
import numpy as np
from PIL import Image
d = sys.argv[1]
views = sys.argv[2:]
for v in views:
    row = [v]
    for m in ("classic", "off", "on"):
        try:
            a = np.asarray(Image.open(f"{d}/{v}__{m}.png").convert("RGB")).astype(np.float64)
        except FileNotFoundError:
            row.append(f"{m}: missing"); continue
        mean = a.mean()
        # lower half = foreground ground; lit pixels = luminance above 60
        g = a[a.shape[0] // 2 :]
        lum = 0.2126 * g[..., 0] + 0.7152 * g[..., 1] + 0.0722 * g[..., 2]
        sel = lum > 60
        rb = g[..., 0][sel].mean() / max(g[..., 2][sel].mean(), 1e-6) if sel.any() else float("nan")
        rg = g[..., 0][sel].mean() / max(g[..., 1][sel].mean(), 1e-6) if sel.any() else float("nan")
        row.append(f"{m}: mean {mean:5.1f} litfrac {sel.mean():.2f} R/B {rb:.2f} R/G {rg:.2f}")
    print(" | ".join(row))
