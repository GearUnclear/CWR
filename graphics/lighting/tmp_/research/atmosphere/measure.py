"""Region statistics of capture PNGs: mean display RGB, luminance and R/B of
the top band (sky), a middle band (horizon / distance) and the bottom band
(near ground). python measure.py <png> [<png> ...]"""
import sys
import numpy as np
from PIL import Image

for p in sys.argv[1:]:
    a = np.asarray(Image.open(p).convert("RGB")).astype(float) / 255.0
    h = a.shape[0]
    out = []
    for name, (y0, y1) in {"top": (0.0, 0.25), "mid": (0.40, 0.55), "bottom": (0.75, 1.0)}.items():
        r = a[int(y0 * h):int(y1 * h)]
        m = r.reshape(-1, 3).mean(0)
        lum = m @ [0.2126, 0.7152, 0.0722]
        out.append(f"{name} rgb {m[0]:.3f} {m[1]:.3f} {m[2]:.3f} lum {lum:.3f} r/b {m[0] / max(m[2], 1e-3):.2f}")
    print(p.split('\\')[-1].split('/')[-1], " | ".join(out))
