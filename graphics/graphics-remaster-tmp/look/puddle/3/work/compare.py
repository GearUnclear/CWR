"""compare.png: rows = five views, columns = round 3 (shiny) | first rework | this (approach 3),
then 2x crops of the tree base and the open street in town_night_rain."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
LOOK = Path("D:/Arma_CWA_worktrees/graphics-remaster/tmp/look")
R3 = LOOK / "fix/seam/shots_4/tri"
RW = LOOK / "fix4/shine/shots_3/tri"
MINE = LOOK / "puddle/3/final"
VIEWS = ["town_night_rain", "seam_b", "seam_d", "seam_day", "rain_day"]
OLD = {"town_night_rain": "look_probe_f2_abel/000_town_night_rain__ovh.png",
       "seam_b": "look_probe_f2_abel/001_seam_b__ovh.png",
       "seam_d": "look_probe_f2_abel/003_seam_d__ovh.png",
       "seam_day": "look_probe_f2_abel/004_seam_day__ovh.png",
       "rain_day": "look_probe_f2_eden/000_rain_day__ovh.png"}
NEW = {"town_night_rain": "look_shine_abel/000_town_night_rain__ovh.png",
       "seam_b": "look_shine_abel/001_seam_b__ovh.png",
       "seam_d": "look_shine_abel/002_seam_d__ovh.png",
       "seam_day": "look_shine_abel/003_seam_day__ovh.png",
       "rain_day": "look_shine_eden/000_rain_day__ovh.png"}
CROPS = [("tree base", 780, 610), ("open street", 330, 790)]
HEADS = ["round 3 (shiny)", "first rework", "puddle 3: tuning only"]


def font(size):
    for f in ("arial.ttf", "segoeui.ttf"):
        try:
            return ImageFont.truetype(f, size)
        except OSError:
            pass
    return ImageFont.load_default()


W, H, TOP = 640, 360, 34
f, fs = font(20), font(16)
rows = len(VIEWS) + len(CROPS)
sheet = Image.new("RGB", (W * 3, TOP + H * rows), (16, 16, 16))
d = ImageDraw.Draw(sheet)
d.text((8, 6), "rows: five views, then 2x crops of town_night_rain | columns: round 3 | first rework | puddle 3",
       fill=(240, 240, 240), font=f)
for c in range(3):
    ims = {}
    for v in VIEWS:
        p = [R3 / OLD[v], RW / NEW[v], MINE / f"{v}.png"][c]
        ims[v] = Image.open(p).convert("RGB")
    for r, v in enumerate(VIEWS):
        y = TOP + r * H
        sheet.paste(ims[v].resize((W, H), Image.LANCZOS), (c * W, y))
        d.rectangle((c * W, y, c * W + 330, y + 22), fill=(0, 0, 0))
        d.text((c * W + 6, y + 2), f"{HEADS[c]}: {v}", fill=(255, 210, 0), font=fs)
        if v == "town_night_rain":
            s = W / 1920.0
            for j, (_, x0, y0) in enumerate(CROPS):
                d.rectangle((c * W + x0 * s, y + y0 * s, c * W + (x0 + 320) * s, y + (y0 + 180) * s),
                            outline=(255, 210, 0) if j == 0 else (0, 200, 255))
    for j, (label, x0, y0) in enumerate(CROPS):
        y = TOP + (len(VIEWS) + j) * H
        sheet.paste(ims["town_night_rain"].crop((x0, y0, x0 + 320, y0 + 180)).resize((W, H), Image.LANCZOS), (c * W, y))
        d.rectangle((c * W, y, c * W + 330, y + 22), fill=(0, 0, 0))
        d.text((c * W + 6, y + 2), f"{HEADS[c]}: 2x {label}", fill=(255, 210, 0) if j == 0 else (0, 200, 255), font=fs)
out = LOOK / "puddle/3/compare.png"
sheet.save(out)
print(out, sheet.size)
