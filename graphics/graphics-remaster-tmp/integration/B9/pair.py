"""pair.py <off.png> <on.png> <out.png> [x0 y0 x1 y1] [scale]: off above on (or a crop of both), labelled."""
import sys
from PIL import Image, ImageDraw
a = Image.open(sys.argv[1]).convert("RGB")
b = Image.open(sys.argv[2]).convert("RGB")
if len(sys.argv) >= 8:
    box = tuple(int(v) for v in sys.argv[4:8])
    a, b = a.crop(box), b.crop(box)
scale = float(sys.argv[8]) if len(sys.argv) >= 9 else (1.0 if a.width > 1000 else 2.0)
size = (int(a.width * scale), int(a.height * scale))
a, b = a.resize(size), b.resize(size)
out = Image.new("RGB", (size[0], size[1] * 2 + 6), (0, 0, 0))
out.paste(a, (0, 0))
out.paste(b, (0, size[1] + 6))
d = ImageDraw.Draw(out)
for y, t in ((4, "off"), (size[1] + 10, "on")):
    d.rectangle((4, y, 44, y + 18), fill=(0, 0, 0))
    d.text((8, y + 3), t, fill=(255, 255, 255))
out.save(sys.argv[3])
