"""Side-by-side crops: python strip.py out.png x0 y0 x1 y1 scale img..."""
import sys
from PIL import Image, ImageDraw
out = sys.argv[1]
x0, y0, x1, y1 = map(int, sys.argv[2:6])
sc = float(sys.argv[6])
ims = []
for p in sys.argv[7:]:
    im = Image.open(p).convert("RGB").crop((x0, y0, x1, y1))
    im = im.resize((int(im.width * sc), int(im.height * sc)), Image.LANCZOS if sc < 1 else Image.NEAREST)
    ImageDraw.Draw(im).text((4, 2), p.replace("\\", "/").split("/")[-1].split("__")[-1][:12], fill=(255, 0, 0))
    ims.append(im)
c = Image.new("RGB", (sum(i.width + 4 for i in ims), max(i.height for i in ims)))
x = 0
for i in ims:
    c.paste(i, (x, 0))
    x += i.width + 4
c.save(out)
