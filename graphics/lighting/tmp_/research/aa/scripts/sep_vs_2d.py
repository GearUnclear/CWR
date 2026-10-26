"""Separable vs 2D Lanczos2: pixel difference between the captures of one run.

    python sep_vs_2d.py <test_dir_in_tri_tree>
"""
import sys
from pathlib import Path

import numpy as np
from PIL import Image

d = Path(sys.argv[1])


def load(tag):
    hits = sorted(d.glob(f"*__{tag}.png"))
    return np.asarray(Image.open(hits[-1]).convert("RGB"), dtype=np.float32) if hits else None


pairs = [("H-lz-a", "H-seplz-a"), ("H-lzrcas05-a", "H-seplzrcas05-a"), ("H-p0-a", "H-lz-a"), ("H-lz-a", "H-lzrcas05-a"),
         ("H-p0-a", "H-p0-b"), ("H-lz-a", "H-lz-b")]
# bottom 55% of the frame: ground, soldier, trees; the sky dome clouds drift between shots
for a, b in pairs:
    A, B = load(a), load(b)
    if A is None or B is None:
        print(a, b, "missing")
        continue
    h = A.shape[0]
    D = np.abs(A - B)[int(h * 0.45):]
    print(f"{a:>16} vs {b:<18} mean |d| {D.mean():.3f}  p99 {np.percentile(D, 99):.1f}  max {D.max():.0f}  >2: {100 * (D.max(axis=2) > 2).mean():.2f}%")
