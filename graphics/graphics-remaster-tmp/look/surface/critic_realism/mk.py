import sys, glob, os
from PIL import Image, ImageDraw
src = r"E:\look-lanes\look-surface\captures\final2\tri"
files = glob.glob(os.path.join(src, "*", "*.png"))
by = {}
for f in files:
    b = os.path.basename(f)[4:-4]
    v, s = b.split("__")
    by.setdefault(v, {})[s] = f
print(Image.open(files[0]).size)
W = 480
for v, d in by.items():
    ims = []
    for s in ("classic", "surf"):
        im = Image.open(d[s]).convert("RGB")
        h = round(im.height * W / im.width)
        ims.append(im.resize((W, h), Image.LANCZOS))
    out = Image.new("RGB", (W * 2 + 6, ims[0].height), (0, 0, 0))
    out.paste(ims[0], (0, 0)); out.paste(ims[1], (W + 6, 0))
    out.save(f"thumb2_{v}.png")
    ims3 = []
    for s in ("classic", "base", "surf"):
        im = Image.open(d[s]).convert("RGB")
        h = round(im.height * W / im.width)
        ims3.append(im.resize((W, h), Image.LANCZOS))
    out = Image.new("RGB", (W * 3 + 12, ims3[0].height), (0, 0, 0))
    for i, im in enumerate(ims3):
        out.paste(im, (i * (W + 6), 0))
    out.save(f"thumb3_{v}.png")
print(sorted(by))
