"""strip.py <tri dir> <view> [labels...] -> <out png>: panels side by side at 640 px wide, labelled."""
import sys, glob, os
from PIL import Image, ImageDraw
d, view = sys.argv[1], sys.argv[2]
labels = sys.argv[3:-1] or ["classic", "base", "fog"]
out = sys.argv[-1]
W = 640
ims = []
for lab in labels:
    m = sorted(glob.glob(os.path.join(d, f"*_{view}__{lab}.png")))
    if not m:
        print("missing", lab); continue
    im = Image.open(m[-1]).convert("RGB")
    h = int(im.height * W / im.width)
    im = im.resize((W, h), Image.LANCZOS)
    dr = ImageDraw.Draw(im)
    dr.rectangle([0, 0, 120, 18], fill=(0, 0, 0))
    dr.text((4, 3), lab, fill=(255, 255, 255))
    ims.append(im)
sheet = Image.new("RGB", (W * len(ims), ims[0].height))
for i, im in enumerate(ims):
    sheet.paste(im, (i * W, 0))
sheet.save(out)
print(out, sheet.size)
