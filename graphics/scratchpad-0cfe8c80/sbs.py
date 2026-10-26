"""sbs.py OUT WIDTH img1 img2 ... : side by side, each scaled to WIDTH px wide, labelled.
A crop can be given as path@x0,y0,x1,y1 (in source pixels)."""
import sys
from PIL import Image, ImageDraw

out = sys.argv[1]
w = int(sys.argv[2])
ims = []
for a in sys.argv[3:]:
    crop = None
    if '@' in a:
        a, c = a.rsplit('@', 1)
        crop = tuple(int(v) for v in c.split(','))
    im = Image.open(a).convert('RGB')
    if crop:
        im = im.crop(crop)
    h = round(im.height * w / im.width)
    im = im.resize((w, h), Image.LANCZOS)
    d = ImageDraw.Draw(im)
    lab = a.replace('\\', '/').split('/')[-1]
    d.rectangle([0, 0, 7 * len(lab) + 6, 14], fill=(0, 0, 0))
    d.text((3, 1), lab, fill=(255, 255, 0))
    ims.append(im)
H = max(i.height for i in ims)
sheet = Image.new('RGB', (w * len(ims) + 4 * (len(ims) - 1), H), (40, 40, 40))
x = 0
for i in ims:
    sheet.paste(i, (x, 0))
    x += w + 4
sheet.save(out)
print(out, sheet.size)
