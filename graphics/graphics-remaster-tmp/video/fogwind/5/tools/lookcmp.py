"""Agent 5: stack crops of several frames (labelled) and print region means, for a look check.

usage: python lookcmp.py <out.png> <img>:<label> [...] [--box x,y,w,h] [--scale 1]
"""
import argparse
import numpy as np
from PIL import Image, ImageDraw

ap = argparse.ArgumentParser()
ap.add_argument("out")
ap.add_argument("imgs", nargs="+")
ap.add_argument("--box", default="0,300,1920,520")
ap.add_argument("--scale", type=float, default=0.5)
o = ap.parse_args()
x, y, w, h = map(int, o.box.split(","))
tiles = []
for spec in o.imgs:
    path, lab = spec.rsplit(":", 1)
    im = Image.open(path).convert("RGB").crop((x, y, x + w, y + h))
    a = np.asarray(im).astype(np.float64)
    print(f"{lab:24s} mean rgb {a[..., 0].mean():6.1f} {a[..., 1].mean():6.1f} {a[..., 2].mean():6.1f}")
    if o.scale != 1:
        im = im.resize((int(w * o.scale), int(h * o.scale)), Image.LANCZOS)
    d = ImageDraw.Draw(im)
    d.text((8, 6), lab, fill=(255, 255, 0))
    tiles.append(im)
W = max(t.width for t in tiles)
H = sum(t.height for t in tiles)
canvas = Image.new("RGB", (W, H))
yy = 0
for t in tiles:
    canvas.paste(t, (0, yy))
    yy += t.height
canvas.save(o.out)
print("wrote", o.out)
