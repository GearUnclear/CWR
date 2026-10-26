import sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont, ImageStat
D = Path(r"D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/director")
views = sys.argv[2].split(",")
W = int(sys.argv[3]) if len(sys.argv) > 3 else 420
f = ImageFont.truetype("segoeui.ttf", 15)
rows = []
for v in views:
    a = Image.open(D / "round4/final" / f"{v}__overhaul.png").convert("RGB")
    b = Image.open(D / "final" / f"{v}__overhaul.png").convert("RGB")
    H = round(W * a.height / a.width)
    a = a.resize((W, H), Image.LANCZOS); b = b.resize((W, H), Image.LANCZOS)
    row = Image.new("RGB", (2 * W + 6, H + 20), (24, 24, 26))
    row.paste(a, (0, 20)); row.paste(b, (W + 6, 20))
    d = ImageDraw.Draw(row)
    ma = sum(ImageStat.Stat(a).mean) / 3; mb = sum(ImageStat.Stat(b).mean) / 3
    d.text((4, 1), f"{v}  round 4 (mean {ma:.0f})", fill=(230, 210, 120), font=f)
    d.text((W + 10, 1), f"round 5 (mean {mb:.0f})", fill=(230, 210, 120), font=f)
    rows.append(row)
cols = 2
import math
nr = math.ceil(len(rows) / cols)
cw = max(r.width for r in rows)
rh = [max(r.height for r in rows[i*cols:(i+1)*cols]) for i in range(nr)]
sheet = Image.new("RGB", (cols * cw + 12 * (cols - 1), sum(rh) + 8 * nr), (24, 24, 26))
y = 0
for i in range(nr):
    for j, r in enumerate(rows[i*cols:(i+1)*cols]):
        sheet.paste(r, (j * (cw + 12), y))
    y += rh[i] + 8
sheet.save(sys.argv[1])
print(sheet.size)
