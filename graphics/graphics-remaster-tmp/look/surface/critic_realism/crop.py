import sys
from PIL import Image
# usage: crop.py out view_prefix_dir seqbase x0 y0 x1 y1 scale suffixes...
out, a, b, x0, y0, x1, y1, sc = sys.argv[1:9]
sufs = sys.argv[9:]
x0, y0, x1, y1, sc = int(x0), int(y0), int(x1), int(y1), float(sc)
import glob, os
ims = []
for s in sufs:
    p = glob.glob(os.path.join(r"E:\look-lanes\look-surface\captures\final2\tri", a, f"*_{b}__{s}.png"))[0]
    im = Image.open(p).convert("RGB").crop((x0, y0, x1, y1))
    im = im.resize((int(im.width * sc), int(im.height * sc)), Image.NEAREST if sc >= 2 else Image.LANCZOS)
    ims.append(im)
W = sum(i.width for i in ims) + 6 * (len(ims) - 1)
o = Image.new("RGB", (W, ims[0].height))
x = 0
for i in ims:
    o.paste(i, (x, 0)); x += i.width + 6
o.save(out)
print(o.size)
