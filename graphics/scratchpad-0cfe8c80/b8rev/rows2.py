"""Row profiles: luma deltas vs the first image, and decoded debug values.
python rows2.py x0 x1 y0 y1 step img...  (names ending c3/c4/c7 are decoded: glow luminance)"""
import sys
import numpy as np
from PIL import Image
L = np.array([0.2126, 0.7152, 0.0722])
x0, x1, y0, y1, st = map(int, sys.argv[1:6])
paths = sys.argv[6:]
def lum(p):
    a = np.asarray(Image.open(p).convert("RGB")).astype(float)
    return (a * L).sum(axis=2)
ims = [lum(p) for p in paths]
names = [p.split("__")[-1].split(".")[0] for p in paths]
dec = {i for i, n in enumerate(names) if n in ("c3", "c4", "c7")}
print("y    " + " ".join(n.rjust(7) for n in names))
for y in range(y0, y1, st):
    cells = []
    for i, im in enumerate(ims):
        v = im[y:y + st, x0:x1].mean()
        if i in dec:
            cells.append(f"{((v / 255) ** 2.2) / 4:7.4f}")
        elif i == 0:
            cells.append(f"{v:7.1f}")
        else:
            cells.append(f"{v - ims[0][y:y + st, x0:x1].mean():+7.1f}")
    print(f"{y:4d} " + " ".join(cells))
