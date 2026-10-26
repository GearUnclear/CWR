"""Time-of-day strip for the haze track: rows = labels, columns = valley_tHHMM views.

python tod_strip.py <run_dir> <out_png> [--scale 0.5]
"""
import argparse
import re
from pathlib import Path

from PIL import Image, ImageDraw

import compare

ap = argparse.ArgumentParser()
ap.add_argument("run_dir")
ap.add_argument("out")
ap.add_argument("--scale", type=float, default=0.5)
ap.add_argument("--crop", default="")
a = ap.parse_args()
shots, times = compare.collect(Path(a.run_dir))
views = sorted({v for (v, _l) in shots if re.match(r"valley_t\d{4}$", v)})
rows = [("haze_v0", "Remastered (current)"), ("haze_v33", "Remastered + haze v33")]
panels = []
for lab, title in rows:
    row = []
    for v in views:
        p = shots.get((v, lab))
        if p is None:
            raise SystemExit(f"missing {v}__{lab}")
        img = Image.open(p).convert("RGB")
        if a.crop:
            img = img.crop(tuple(int(x) for x in a.crop.split(",")))
        hhmm = v.split("_t", 1)[1]
        row.append(compare.panel(img, f"{hhmm[:2]}:{hhmm[2:]}  {title}", times.get((v, lab)), a.scale))
    panels.append(row)
W, H = panels[0][0].size
gap = 6
sheet = Image.new("RGB", (len(views) * W + (len(views) - 1) * gap, len(rows) * H + (len(rows) - 1) * gap + 34), (24, 24, 24))
d = ImageDraw.Draw(sheet)
d.text((10, 6), "valley_morning (Abel, heading 50, July 5): morning mist vs sun height. Top: current Remastered. Bottom: haze v33.",
       fill=(235, 235, 235), font=compare.font(18))
for r, row in enumerate(panels):
    for c, im in enumerate(row):
        sheet.paste(im, (c * (W + gap), 34 + r * (H + gap)))
sheet.save(a.out)
print("wrote", a.out, sheet.size)
