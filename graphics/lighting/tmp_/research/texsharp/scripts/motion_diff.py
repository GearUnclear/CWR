"""Per variant: a crop of motion frame 3 and the amplified |frame 4 - frame 3|
difference (x4), so frame-to-frame sparkle is visible in a still image.

    python motion_diff.py <run_dir> <view> <x0,y0,x1,y1> <out.png>
"""
import sys
from pathlib import Path
import numpy as np
from PIL import Image, ImageDraw, ImageFont

run, view, box, out = Path(sys.argv[1]) / "motion", sys.argv[2], tuple(int(v) for v in sys.argv[3].split(",")), sys.argv[4]
fnt = ImageFont.truetype("C:/Windows/Fonts/arialbd.ttf", 15)
names = {"mstock": "Remastered (current)", "mrdtile": "detail kept K2=2 + anti-tile", "mself": "self-detail 1.5 rotated",
         "mbest": "texsharp final rotated (sd 1.5, detail kept, CAS 0.3 luma)"}
rows = []
for var in ("mstock", "mrdtile", "mself", "mbest"):
    a = Image.open(run / f"{view}__{var}3.png").convert("RGB").crop(box)
    b = Image.open(run / f"{view}__{var}4.png").convert("RGB").crop(box)
    d = np.clip(np.abs(np.asarray(b, np.float32) - np.asarray(a, np.float32)).mean(-1) * 4, 0, 255).astype(np.uint8)
    dimg = Image.fromarray(d).convert("RGB")
    row = Image.new("RGB", (a.width * 2 + 6, a.height), (20, 20, 20))
    row.paste(a, (0, 0)); row.paste(dimg, (a.width + 6, 0))
    dr = ImageDraw.Draw(row)
    for x, t in ((4, names[var] + ": frame 3"), (a.width + 10, "|frame 4 - frame 3| x4 (camera moved 0.25 m)")):
        bb = dr.textbbox((x, 4), t, font=fnt); dr.rectangle([bb[0] - 3, bb[1] - 2, bb[2] + 3, bb[3] + 2], fill=(0, 0, 0)); dr.text((x, 4), t, font=fnt, fill=(255, 255, 255))
    rows.append(row)
sheet = Image.new("RGB", (rows[0].width, sum(r.height + 6 for r in rows)), (20, 20, 20))
y = 0
for r in rows:
    sheet.paste(r, (0, y)); y += r.height + 6
sheet.save(out)
print(out)
