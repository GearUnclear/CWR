"""make_sheet.py <out.png> <cols> <run dir>... : the contact sheet, Original (Classic) | Overhaul
per view at thumbnail size (480 px panels), every view found in the run dirs' tri trees.
cols: comma list of labels to show (default classic,fog), headed by their names below."""
import sys, glob, os, re
from PIL import Image, ImageDraw, ImageFont

NAMES = {"classic": "Original (Classic)", "fog": "Overhaul (volumetric fog)", "base": "B8 air (landed)"}
ORDER = ["sunrise_ridge", "forest_edge_golden", "coastline_sunset", "overcast_broken", "overcast_heavy", "town_night", "noon_vista",
         "dawn_valley", "pine_rays", "town_fog_night", "noon_fields", "sinai_lowsun_fp", "sinai_desert_noon"]
out, cols, dirs = sys.argv[1], sys.argv[2].split(","), sys.argv[3:]
W = 480
shots = {}
for d in dirs:
    for p in sorted(glob.glob(os.path.join(d, "tri", "foglane_*", "*.png"))):
        m = re.match(r"\d+_(.+)__(\w+)\.png$", os.path.basename(p))
        if m:
            shots[(m.group(1), m.group(2))] = p
views = [v for v in ORDER if all((v, c) in shots for c in cols)]
views += sorted({v for (v, c) in shots if v not in views and all((v, cc) in shots for cc in cols)})
try:
    font = ImageFont.truetype("arial.ttf", 15)
    small = ImageFont.truetype("arial.ttf", 13)
except OSError:
    font = small = ImageFont.load_default()
H = int(W * 1080 / 1920)
head = 26
sheet = Image.new("RGB", (W * len(cols) + 150, head + H * len(views)), (24, 24, 24))
dr = ImageDraw.Draw(sheet)
for i, c in enumerate(cols):
    dr.text((150 + i * W + 6, 5), NAMES.get(c, c), fill=(230, 230, 230), font=font)
for r, v in enumerate(views):
    y = head + r * H
    dr.text((6, y + H // 2 - 8), v, fill=(230, 230, 230), font=small)
    for i, c in enumerate(cols):
        im = Image.open(shots[(v, c)]).convert("RGB").resize((W, H), Image.LANCZOS)
        sheet.paste(im, (150 + i * W, y))
sheet.save(out)
print(out, sheet.size, len(views), "views")
