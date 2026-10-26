"""Side-by-side crops of shots, scaled up with nearest-neighbour.

    python crop.py <out.png> <x0> <y0> <x1> <y1> <scale> <png> [<png> ...]
"""
import sys

from PIL import Image, ImageDraw

out, x0, y0, x1, y1, scale = sys.argv[1], *map(int, sys.argv[2:6]), int(sys.argv[6])
files = sys.argv[7:]
w, h = (x1 - x0) * scale, (y1 - y0) * scale
sheet = Image.new("RGB", (w * len(files), h + 16), (20, 20, 20))
d = ImageDraw.Draw(sheet)
for i, f in enumerate(files):
    im = Image.open(f).convert("RGB").crop((x0, y0, x1, y1)).resize((w, h), Image.NEAREST)
    sheet.paste(im, (i * w, 16))
    d.text((i * w + 4, 2), f.replace("\\", "/").split("/")[-1], fill=(230, 230, 230))
sheet.save(out)
print(out, sheet.size)
