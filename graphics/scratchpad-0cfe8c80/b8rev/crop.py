"""python crop.py out.png x0 x1 y0 y1 scale img1 [img2 ...] (stacked vertically, labelled)"""
import sys
from PIL import Image, ImageDraw
out, x0, x1, y0, y1, sc = sys.argv[1], *map(int, sys.argv[2:6]), float(sys.argv[6])
ims = []
for p in sys.argv[7:]:
    im = Image.open(p).convert("RGB").crop((x0, y0, x1, y1))
    im = im.resize((int(im.width * sc), int(im.height * sc)), Image.NEAREST)
    ImageDraw.Draw(im).text((4, 2), p.split("/")[-1].split("\\")[-1][:40], fill=(255, 0, 0))
    ims.append(im)
W = max(i.width for i in ims); H = sum(i.height for i in ims) + 4 * len(ims)
c = Image.new("RGB", (W, H), (0, 0, 0)); y = 0
for i in ims:
    c.paste(i, (0, y)); y += i.height + 4
c.save(out)
