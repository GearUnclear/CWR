"""sbs.py out.png title crop(x0,y0,x1,y1) panelW vanilla landed before after
Row 1: the four full views at panelW; row 2: the same four crops at 2x (scaled to fit panelW)."""
import sys
from PIL import Image, ImageDraw
out, title, crop, pw = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4])
paths = sys.argv[5:9]
labels = ["vanilla", "landed", "before (r3)", "after (r4 burst)"]
x0, y0, x1, y1 = [int(v) for v in crop.split(",")]
ims = [Image.open(p).convert("RGB") for p in paths]
fulls, crops = [], []
for im in ims:
    s = pw / im.width
    fulls.append(im.resize((pw, int(im.height * s)), Image.LANCZOS))
    # crop in the coordinates of a 960-wide frame, scaled to this image
    k = im.width / 960.0
    c = im.crop((int(x0 * k), int(y0 * k), int(x1 * k), int(y1 * k)))
    cw = (x1 - x0) * 2
    c = c.resize((cw, int(c.height * cw / c.width)), Image.LANCZOS)
    if c.width > pw:
        c = c.resize((pw, int(c.height * pw / c.width)), Image.LANCZOS)
    crops.append(c)
fh = max(f.height for f in fulls); ch = max(c.height for c in crops)
W = pw * 4 + 12; H = 22 + 16 + fh + 4 + 16 + ch
sheet = Image.new("RGB", (W, H), (16, 16, 16)); d = ImageDraw.Draw(sheet)
d.text((4, 4), title, fill=(255, 255, 255))
for i in range(4):
    x = i * (pw + 4)
    d.text((x + 4, 24), labels[i], fill=(255, 255, 0))
    sheet.paste(fulls[i], (x, 38))
    d.text((x + 4, 38 + fh + 4), labels[i] + " 2x crop", fill=(255, 255, 0))
    sheet.paste(crops[i], (x, 38 + fh + 20))
sheet.save(out); print(out, sheet.size)
