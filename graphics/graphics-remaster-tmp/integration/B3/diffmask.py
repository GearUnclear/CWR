"""Where two shots differ by more than 2 levels: a mask image (red = over 8,
yellow = over 2) over the first shot darkened, and the bounding box of the
over-8 pixels.

    python diffmask.py <a.png> <b.png> <out.png>
"""
import sys

import numpy as np
from PIL import Image

a = np.asarray(Image.open(sys.argv[1]).convert("RGB")).astype(np.int16)
b = np.asarray(Image.open(sys.argv[2]).convert("RGB")).astype(np.int16)
m = np.abs(a - b).max(axis=2)
out = (a * 0.35).astype(np.uint8)
out[m > 2] = (230, 200, 0)
out[m > 8] = (255, 40, 40)
Image.fromarray(out).save(sys.argv[3])
ys, xs = np.nonzero(m > 8)
if len(xs):
    print(f"over 8: {len(xs)} px, box x {xs.min()}-{xs.max()} y {ys.min()}-{ys.max()}")
ys, xs = np.nonzero(m > 2)
print(f"over 2: {len(xs)} px")
