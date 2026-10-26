"""Invert PoseidonTools `terrain render` colour ramp (AssetPreview.cpp PreviewTerrain) to metres."""
import sys
import numpy as np
from PIL import Image

RANGES = {"abel": (-100.0, 521.0), "eden": (-49.8, 357.8)}

def decode(png, lo, hi):
    a = np.asarray(Image.open(png).convert("RGB")).astype(np.float64)
    r, g, b = a[..., 0], a[..., 1], a[..., 2]
    t = np.zeros(r.shape)
    sea = (r == 30) & (g == 60) & (b == 180)
    s1 = (~sea) & (r < 130)
    s2 = (~sea) & (r >= 130) & (r < 190)
    s3 = (~sea) & (r >= 190)
    t[s1] = 0.01 + 0.29 * (r[s1] - 30) / 100.0
    t[s2] = 0.3 + 0.4 * (r[s2] - 130) / 60.0
    t[s3] = 0.7 + 0.3 * (b[s3] - 30) / 225.0
    t[sea] = 0.0
    h = lo + t * (hi - lo)
    # image row 0 is the north edge: flip so index [zg, x] is grid north index zg
    return h[::-1, :]

if __name__ == "__main__":
    import os
    d = os.path.dirname(os.path.abspath(__file__))
    for w, (lo, hi) in RANGES.items():
        h = decode(os.path.join(d, "..", "terrain", w + ".png"), lo, hi)
        np.save(os.path.join(d, "..", "terrain", w + "_h.npy"), h)
        print(w, h.min(), h.max(), h.shape)
