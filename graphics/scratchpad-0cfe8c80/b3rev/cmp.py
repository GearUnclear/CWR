"""Pairwise differences: max channel difference, pixels over 2 and over 8 levels,
and mean luma of each side, for the whole frame and per row third."""
import sys
import numpy as np
from PIL import Image

def load(p):
    return np.asarray(Image.open(p).convert("RGB")).astype(np.int16)

def row(label, a, b):
    m = np.abs(a - b).max(axis=2)
    ya = (a @ np.array([0.2126, 0.7152, 0.0722]))
    yb = (b @ np.array([0.2126, 0.7152, 0.0722]))
    h = m.shape[0]
    cells = []
    for lo, hi in ((0, h // 3), (h // 3, 2 * h // 3), (2 * h // 3, h), (0, h)):
        s = m[lo:hi]
        cells.append(f"{s.max()} / {(s > 2).sum():,} / {(s > 8).sum():,}")
    print(f"| {label} | " + " | ".join(cells) + f" | {ya.mean():.1f} / {yb.mean():.1f} |")

print("| pair | top third | middle third | bottom third | whole | mean luma a / b |")
print("|---|---|---|---|---|---|")
args = sys.argv[1:]
for i in range(0, len(args), 3):
    row(args[i], load(args[i + 1]), load(args[i + 2]))
