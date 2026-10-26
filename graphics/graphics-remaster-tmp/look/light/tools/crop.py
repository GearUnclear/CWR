"""Side-by-side crops of several shots, optionally scaled and contrast-stretched.

    python crop.py out.png x0 y0 x1 y1 [--scale S] [--gain G] img1 img2 ...

--gain stretches each crop about its own mean (G x the deviation) to show
banding and streaks.
"""
import argparse

from PIL import Image, ImageDraw
import numpy as np

ap = argparse.ArgumentParser()
ap.add_argument("out")
ap.add_argument("box", nargs=4, type=int)
ap.add_argument("imgs", nargs="+")
ap.add_argument("--scale", type=float, default=1.0)
ap.add_argument("--gain", type=float, default=0.0)
a = ap.parse_args()
x0, y0, x1, y1 = a.box
tiles = []
for p in a.imgs:
    im = Image.open(p).convert("RGB").crop((x0, y0, x1, y1))
    if a.gain > 0:
        arr = np.asarray(im).astype(np.float32)
        m = arr.mean(axis=(0, 1), keepdims=True)
        arr = np.clip(m + (arr - m) * a.gain, 0, 255).astype(np.uint8)
        im = Image.fromarray(arr)
    if a.scale != 1.0:
        im = im.resize((int(im.width * a.scale), int(im.height * a.scale)), Image.NEAREST if a.scale > 1 else Image.LANCZOS)
    d = ImageDraw.Draw(im)
    name = p.replace("\\", "/").split("/")[-1].replace(".png", "")
    d.rectangle((0, 0, 8 + 7 * len(name), 14), fill=(0, 0, 0))
    d.text((3, 1), name, fill=(255, 255, 255))
    tiles.append(im)
w = sum(t.width for t in tiles) + 4 * (len(tiles) - 1)
h = max(t.height for t in tiles)
out = Image.new("RGB", (w, h), (20, 20, 20))
x = 0
for t in tiles:
    out.paste(t, (x, 0))
    x += t.width + 4
out.save(a.out)
print(a.out, out.size)
