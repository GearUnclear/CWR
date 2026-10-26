"""diffimg.py a.png b.png out.png [--crop x0,y0,x1,y1] [--zoom 4]: an image of
where a and b differ (max channel difference, x16, red over 8 levels), and a
listing of the bounding boxes of the differing clusters."""
import sys
import numpy as np
from PIL import Image

a = np.asarray(Image.open(sys.argv[1]).convert("RGB")).astype(np.int16)
b = np.asarray(Image.open(sys.argv[2]).convert("RGB")).astype(np.int16)
m = np.abs(a - b).max(axis=2)
out = np.zeros(a.shape, np.uint8)
g = (a.mean(axis=2) * 0.35).astype(np.uint8)
out[..., 0] = g
out[..., 1] = g
out[..., 2] = g
out[m > 2] = (255, 255, 0)
out[m > 8] = (255, 0, 0)
Image.fromarray(out).save(sys.argv[3])
ys, xs = np.nonzero(m > 2)
print("pixels >2:", len(ys), ">8:", int((m > 8).sum()), "max:", int(m.max()))
if len(ys):
    # coarse clusters on a 64 px grid
    cells = {}
    for y, x in zip(ys, xs):
        cells.setdefault((y // 64, x // 64), []).append((y, x))
    top = sorted(cells.items(), key=lambda kv: -len(kv[1]))[:12]
    for (cy, cx), pts in top:
        print(f"cell y{cy*64}-{cy*64+63} x{cx*64}-{cx*64+63}: {len(pts)} px, max {max(int(m[y, x]) for y, x in pts)}")
