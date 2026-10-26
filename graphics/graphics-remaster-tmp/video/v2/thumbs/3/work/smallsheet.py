import sys, os
from PIL import Image, ImageDraw

# usage: smallsheet.py out.png img1 img2 ...  (each shown at 360x202 and 168x94)
out = sys.argv[1]
paths = sys.argv[2:]
cols = 3
cw, ch = 360 + 12 + 168 + 24, 202 + 26
rows = (len(paths) + cols - 1) // cols
im = Image.new("RGB", (cols * cw, rows * ch), (24, 24, 24))
d = ImageDraw.Draw(im)
for k, p in enumerate(paths):
    src = Image.open(p).convert("RGB")
    x, y = (k % cols) * cw + 8, (k // cols) * ch + 20
    im.paste(src.resize((360, 202), Image.LANCZOS), (x, y))
    im.paste(src.resize((168, 94), Image.LANCZOS), (x + 372, y))
    d.text((x, y - 15), os.path.basename(p)[:60], fill=(230, 230, 120))
im.save(out)
