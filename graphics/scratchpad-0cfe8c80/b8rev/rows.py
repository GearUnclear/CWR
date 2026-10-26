"""Row profile of luma in an x range for several shots: python rows.py x0 x1 y0 y1 step img..."""
import sys
import numpy as np
from PIL import Image
L = np.array([0.2126, 0.7152, 0.0722])
x0, x1, y0, y1, st = map(int, sys.argv[1:6])
ims = [(np.asarray(Image.open(p).convert("RGB")).astype(float) * L).sum(axis=2) for p in sys.argv[6:]]
print("y    " + " ".join(p.split("__")[-1][:8].rjust(8) for p in sys.argv[6:]))
for y in range(y0, y1, st):
    print(f"{y:4d} " + " ".join(f"{im[y:y+st, x0:x1].mean():8.1f}" for im in ims))
