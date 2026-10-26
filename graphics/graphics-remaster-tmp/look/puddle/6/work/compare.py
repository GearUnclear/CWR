"""compare.py MINE_DIR OUT.png [label]: rows = five views, columns = round 3 | first rework | mine,
plus 2x crops of the tree base and the open street in town_night_rain."""
import sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

GR = Path(r"D:/Arma_CWA_worktrees/graphics-remaster/tmp/look")
R3 = GR / "fix/seam/shots_4/tri"
RW = GR / "fix4/shine/shots_3/tri"
mine = Path(sys.argv[1])
out = Path(sys.argv[2])
label = sys.argv[3] if len(sys.argv) > 3 else "approach 6 (puddles only)"

def find(root, name):
    hits = sorted(root.rglob(f"*_{name}__ovh.png"))
    if not hits:
        hits = sorted(root.rglob(f"{name}.png"))
    return hits[0] if hits else None

views = ["town_night_rain", "seam_b", "seam_d", "seam_day", "rain_day"]
cols = [("round 3 (shiny)", R3), ("first rework", RW), (label, mine)]
W, H = 640, 360
try:
    font = ImageFont.truetype("arial.ttf", 18)
except OSError:
    font = ImageFont.load_default()
TREE = (780, 711, 1100, 891)
STREET = (330, 891, 650, 1071)
rows = len(views) + 2
top = 30
sheet = Image.new("RGB", (W * 3, top + rows * H), (16, 16, 16))
d = ImageDraw.Draw(sheet)
d.text((8, 6), "rows: " + ", ".join(views) + ", then 2x crops of town_night_rain (tree base, open street)", fill=(230, 230, 230), font=font)

def tag(x, y, text):
    w = d.textlength(text, font=font)
    d.rectangle([x, y, x + w + 12, y + 24], fill=(0, 0, 0))
    d.text((x + 6, y + 2), text, fill=(255, 200, 40), font=font)

for c, (name, root) in enumerate(cols):
    for r, v in enumerate(views):
        p = find(root, v)
        x, y = c * W, top + r * H
        if p is None:
            tag(x + 4, y + 4, f"{name}: {v} missing")
            continue
        im = Image.open(p).convert("RGB")
        sheet.paste(im.resize((W, H), Image.LANCZOS), (x, y))
        if v == "town_night_rain":
            dd = ImageDraw.Draw(sheet)
            for box, col in ((TREE, (255, 200, 40)), (STREET, (60, 200, 255))):
                dd.rectangle([x + box[0] // 3, y + box[1] // 3, x + box[2] // 3, y + box[3] // 3], outline=col, width=2)
            for k, box in enumerate((TREE, STREET)):
                crop = im.crop(box).resize((W, H), Image.LANCZOS)
                sheet.paste(crop, (x, top + (len(views) + k) * H))
                tag(x + 4, top + (len(views) + k) * H + 4, "2x tree base" if k == 0 else "2x open street")
        tag(x + 4, y + 4, f"{name}: {v}")
sheet.save(out)
print(out)
