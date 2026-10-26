"""Pixel difference between two captures per row band (sky / horizon / ground).

usage: banddiff.py <a.png> <b.png> [bands, default 0-250,250-350,350-640]
Prints, per band: max channel difference / pixels over 2 / pixels over 8.
"""
import sys

import numpy as np
from PIL import Image

a = np.asarray(Image.open(sys.argv[1]).convert("RGB")).astype(np.int16)
b = np.asarray(Image.open(sys.argv[2]).convert("RGB")).astype(np.int16)
m = np.abs(a - b).max(axis=2)
bands = sys.argv[3] if len(sys.argv) > 3 else "0-250,250-350,350-640"
out = []
for band in bands.split(","):
    lo, hi = (int(x) for x in band.split("-"))
    s = m[lo:hi]
    out.append(f"rows {lo}-{hi}: {int(s.max())} / {int((s > 2).sum())} / {int((s > 8).sum())}")
print(" | ".join(out) + f" | whole: {int(m.max())} / {int((m > 2).sum())} / {int((m > 8).sum())} of {m.size}")
