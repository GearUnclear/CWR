"""Zoomed N-way crop of the same region from several shots, labelled.

    python cropn.py <out.png> x0 y0 x1 y1 scale "<label>=<png>" ["<label>=<png>" ...] [--cols N]
"""
import sys
from PIL import Image, ImageDraw, ImageFont

args = [a for a in sys.argv[1:]]
cols = None
if "--cols" in args:
    i = args.index("--cols"); cols = int(args[i + 1]); del args[i:i + 2]
out = args[0]
x0, y0, x1, y1 = map(int, args[1:5])
sc = float(args[5])
items = [a.split("=", 1) for a in args[6:]]
try:
    f = ImageFont.truetype("C:/Windows/Fonts/arialbd.ttf", 22)
except OSError:
    f = ImageFont.load_default()
w, h = int((x1 - x0) * sc), int((y1 - y0) * sc)
n = len(items)
cols = cols or n
rows = (n + cols - 1) // cols
c = Image.new("RGB", (cols * (w + 8) - 8, rows * (h + 8) - 8 + 34), (24, 24, 24))
d = ImageDraw.Draw(c)
d.text((8, 4), f"crop ({x0},{y0})-({x1},{y1}) x{sc}", fill=(230, 230, 230), font=f)
for k, (lab, p) in enumerate(items):
    im = Image.open(p).convert("RGB").crop((x0, y0, x1, y1)).resize((w, h), Image.LANCZOS)
    cx, cy = (k % cols) * (w + 8), 34 + (k // cols) * (h + 8)
    c.paste(im, (cx, cy))
    bb = d.textbbox((0, 0), lab, font=f)
    d.rectangle((cx + 4, cy + 4, cx + 12 + bb[2], cy + 10 + bb[3]), fill=(0, 0, 0))
    d.text((cx + 8, cy + 4), lab, fill=(255, 255, 255), font=f)
c.save(out)
print(out)
