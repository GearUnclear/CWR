"""A/B of my runs: python ab.py <out.png> <shots dir> [<shots dir> ...]
Rows: the five views at 640x360; then 2x crops of town_night_rain (tree base, open street) and seam_b.
Also prints lower-half luminance stats per view."""
import sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageStat
NEW = {
    "town_night_rain": "look_shine_abel/000_town_night_rain__ovh.png",
    "seam_b": "look_shine_abel/001_seam_b__ovh.png",
    "seam_d": "look_shine_abel/002_seam_d__ovh.png",
    "seam_day": "look_shine_abel/003_seam_day__ovh.png",
    "rain_day": "look_shine_eden/000_rain_day__ovh.png",
}
CROPS = [("town_night_rain", 780, 610), ("town_night_rain", 330, 790), ("seam_b", 640, 760)]
out = sys.argv[1]
dirs = [Path(d) for d in sys.argv[2:]]
W, H = 640, 360
rows = list(NEW) + [f"{v}@{x},{y}" for v, x, y in CROPS]
sheet = Image.new("RGB", (W * len(dirs), H * len(rows) + 24), (16, 16, 16))
d = ImageDraw.Draw(sheet)
for i, dd in enumerate(dirs):
    d.text((i * W + 6, 6), dd.name, fill=(255, 210, 0))
    for j, v in enumerate(NEW):
        p = dd / "tri" / NEW[v]
        if not p.exists():
            continue
        im = Image.open(p).convert("RGB")
        sheet.paste(im.resize((W, H), Image.LANCZOS), (i * W, 24 + j * H))
        lo = im.crop((0, 600, 1920, 1080)).convert("L")
        st = ImageStat.Stat(lo)
        hist = lo.histogram()
        bright = sum(hist[120:]) / (1920 * 480)
        print(f"{dd.name:14s} {v:16s} lower mean {st.mean[0]:6.2f} sd {st.stddev[0]:6.2f} >120 {bright:.4f}")
    for k, (v, x, y) in enumerate(CROPS):
        p = dd / "tri" / NEW[v]
        if not p.exists():
            continue
        im = Image.open(p).convert("RGB")
        c = im.crop((x, y, x + 320, y + 180)).resize((W, H), Image.LANCZOS)
        sheet.paste(c, (i * W, 24 + (len(NEW) + k) * H))
sheet.save(out)
print(out)
