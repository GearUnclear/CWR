"""strips.py <dir> <out.png> <still> [y0 y1 x0 x1 scale]: stack the band of each variant shot of a still."""
import sys, glob, os
from PIL import Image, ImageDraw
d, out, still = sys.argv[1], sys.argv[2], sys.argv[3]
y0, y1, x0, x1, sc = (int(v) for v in (sys.argv[4:9] if len(sys.argv) > 8 else (290, 380, 0, 960, 2)))
files = sorted(f for f in glob.glob(os.path.join(d, "**", "*.png"), recursive=True) if f"_{still}__" in os.path.basename(f) and "pair" not in f and os.sep + "tri" + os.sep in f)
rows = []
for f in files:
    im = Image.open(f).convert("RGB").crop((x0, y0, x1, y1))
    im = im.resize(((x1 - x0) * sc, (y1 - y0) * sc), Image.LANCZOS)
    dr = ImageDraw.Draw(im)
    lab = os.path.basename(f).split("__")[-1][:-4]
    dr.rectangle((0, 0, 8 * len(lab) + 8, 14), fill=(0, 0, 0))
    dr.text((4, 1), lab, fill=(255, 255, 0))
    rows.append(im)
W = rows[0].width
H = sum(r.height for r in rows)
o = Image.new("RGB", (W, H))
y = 0
for r in rows:
    o.paste(r, (0, y)); y += r.height
o.save(out)
print(out, len(rows))
