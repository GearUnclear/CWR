"""cropgrid.py out.png x0 y0 x1 y1 scale label=path ...: crops side by side (stacked vertically) with labels."""
import sys
from PIL import Image, ImageDraw
out = sys.argv[1]; x0, y0, x1, y1 = map(int, sys.argv[2:6]); sc = float(sys.argv[6])
items = [a.split("=", 1) for a in sys.argv[7:]]
crops = []
for lab, p in items:
    im = Image.open(p).convert("RGB").crop((x0, y0, x1, y1))
    im = im.resize((int(im.width * sc), int(im.height * sc)), Image.LANCZOS)
    d = ImageDraw.Draw(im); d.rectangle((0, 0, 8 * len(lab) + 6, 14), fill=(0, 0, 0)); d.text((3, 1), lab, fill=(255, 255, 0))
    crops.append(im)
W = max(c.width for c in crops); H = sum(c.height for c in crops)
sheet = Image.new("RGB", (W, H)); y = 0
for c in crops:
    sheet.paste(c, (0, y)); y += c.height
sheet.save(out); print(out, sheet.size)
