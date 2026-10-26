"""crops.py <frames dir> <clip tag e.g. night_he> <out png> <frame list e.g. 8,9,10,11,12,15,20,30,45,60,90> [cx cy w h]
Crops a region around the burst from each frame and lays them out in a row, labelled with ms from the spawn (PRE=8)."""
import sys, re
from pathlib import Path
from PIL import Image, ImageDraw, ImageChops
d, tag, out = Path(sys.argv[1]), sys.argv[2], Path(sys.argv[3])
frames = [int(x) for x in sys.argv[4].split(",")]
files = {}
for p in d.glob("*.png"):
    m = re.search(r"mun_" + tag + r"_f(\d+)\.png$", p.name)
    if m:
        files[int(m.group(1))] = p
if len(sys.argv) > 8:
    cx, cy, w, h = [int(x) for x in sys.argv[5:9]]
else:
    a = Image.open(files[8]).convert("L"); b = Image.open(files[min(12, max(files))]).convert("L")
    diff = ImageChops.difference(a, b).point(lambda v: 255 if v > 25 else 0)
    bb = diff.getbbox() or (380, 250, 580, 400)
    cx, cy = (bb[0] + bb[2]) // 2, (bb[1] + bb[3]) // 2
    w, h = 360, 260
row = []
for f in frames:
    if f not in files:
        continue
    im = Image.open(files[f]).convert("RGB")
    x0 = max(0, min(im.width - w, cx - w // 2)); y0 = max(0, min(im.height - h, cy - h // 2))
    c = im.crop((x0, y0, x0 + w, y0 + h))
    ImageDraw.Draw(c).text((4, 4), f"{(f - 8) * 1000 / 30:+.0f} ms", fill=(255, 230, 0))
    row.append(c)
sheet = Image.new("RGB", (w * len(row), h), (0, 0, 0))
for i, c in enumerate(row):
    sheet.paste(c, (i * w, 0))
sheet.save(out)
print(out, "centre", cx, cy, "frames", len(row))
