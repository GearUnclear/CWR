"""Pixel difference between two captures: max / mean abs difference and the count of
pixels that differ by more than a threshold. Optionally writes an amplified diff image.

usage: imgdiff.py <a.png> <b.png> [<diff_out.png>]
"""
import sys

import numpy as np
from PIL import Image

a = np.asarray(Image.open(sys.argv[1]).convert("RGB")).astype(np.int16)
b = np.asarray(Image.open(sys.argv[2]).convert("RGB")).astype(np.int16)
d = np.abs(a - b)
m = d.max(axis=2)
print(f"max={int(d.max())} mean={d.mean():.4f} px>2={int((m > 2).sum())} px>8={int((m > 8).sum())} of {m.size}")
if len(sys.argv) > 3:
    Image.fromarray(np.clip(m * 16, 0, 255).astype(np.uint8)).save(sys.argv[3])
