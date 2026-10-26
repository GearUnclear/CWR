#!/usr/bin/env python
"""Before/after head-crop sheet: stacks the first R rows (10 frames each) of
crops_<label>_<i>.png images made by metric.py, one block per (title, png).
  python compare.py out.png "BEFORE ud base A=path/crops_ud-base_0.png" "AFTER ud fix A=path/crops_ud-combo2_0.png" ... [--rows 2]
"""
import sys
from PIL import Image, ImageDraw
args = [a for a in sys.argv[2:] if not a.startswith("--rows")]
rows = 2
for a in sys.argv[2:]:
    if a.startswith("--rows="):
        rows = int(a.split("=")[1])
blocks = []
for a in args:
    title, path = a.rsplit("=", 1)
    im = Image.open(path)
    # metric.py tiles: 60 frames in 6 rows of 10, 2 px gutters
    nrows = 6
    tile_h = (im.height - 2) // nrows - 2
    cut = im.crop((0, 0, im.width, 2 + min(rows, nrows) * (tile_h + 2)))
    blocks.append((title, cut))
W = max(b.width for _, b in blocks)
H = sum(b.height + 22 for _, b in blocks)
out = Image.new("RGB", (W, H), (20, 20, 20))
d = ImageDraw.Draw(out)
y = 0
for t, b in blocks:
    d.text((4, y + 4), t, fill=(255, 255, 255))
    out.paste(b, (0, y + 20))
    y += b.height + 22
out.save(sys.argv[1])
print(sys.argv[1], out.size)
