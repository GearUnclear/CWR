"""Draw a labelled coordinate grid over a capture (for picking measurement boxes)."""
import sys
from PIL import Image, ImageDraw
src, dst = sys.argv[1], sys.argv[2]
box = tuple(int(v) for v in sys.argv[3].split(',')) if len(sys.argv) > 3 else None
scale = int(sys.argv[4]) if len(sys.argv) > 4 else 1
im = Image.open(src).convert('RGB')
if box:
    im = im.crop(box)
    ox, oy = box[0], box[1]
else:
    ox = oy = 0
im = im.resize((im.width * scale, im.height * scale), Image.NEAREST)
d = ImageDraw.Draw(im)
step = 20 if scale >= 3 else 40
for x in range((ox // step + 1) * step, ox + im.width // scale, step):
    X = (x - ox) * scale
    d.line([(X, 0), (X, im.height)], fill=(255, 255, 0) if x % 100 == 0 else (90, 90, 0))
    if x % (step * 2) == 0:
        d.text((X + 2, 2), str(x), fill=(255, 255, 0))
for y in range((oy // step + 1) * step, oy + im.height // scale, step):
    Y = (y - oy) * scale
    d.line([(0, Y), (im.width, Y)], fill=(0, 255, 255) if y % 100 == 0 else (0, 90, 90))
    if y % (step * 2) == 0:
        d.text((2, Y + 2), str(y), fill=(0, 255, 255))
im.save(dst)
