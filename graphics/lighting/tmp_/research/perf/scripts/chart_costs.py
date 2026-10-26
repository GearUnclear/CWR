"""Static cost charts (PNG, drawn with PIL) from UDPERF logs: one horizontal-bar
panel per measure, a shared config axis, the median over rounds as the bar and
each round as a dot. Grey = Classic, blue = Remastered.

usage: chart_costs.py <out.png> <title> <log> <regime> <measure,...> <config,...> [<label;label;...>]
measures: gworld gpost gframe cdraw cobj calls xmin_world ground pass1 u_litKB u_vs
"""
import re
import statistics as st
import sys
from collections import defaultdict
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, str(Path(__file__).parent))
from summarize_rounds import parse  # noqa: E402

out, title, log, regime = sys.argv[1:5]
measures = sys.argv[5].split(",")
configs = sys.argv[6].split(",")
labels = sys.argv[7].split(";") if len(sys.argv) > 7 else configs

NAMES = {"gworld": "GPU world pass, ms", "gpost": "GPU post pass, ms", "gframe": "GPU frame, ms",
         "cdraw": "CPU draw, ms", "cobj": "CPU opaque objects (land:obj), ms", "calls": "draw calls per frame",
         "xmin_world": "GPU world, best frame in window, ms", "ground": "GPU terrain pass, ms",
         "pass1": "GPU opaque objects pass, ms", "u_litKB": "lit UBO upload, KB per frame",
         "u_vs": "VS UBO full flushes per frame", "cframe": "CPU frame, ms"}
KEYS = {"ground": "x_ground", "pass1": "x_pass1"}

rows = parse(log)
vals = defaultdict(lambda: defaultdict(list))
for r in rows:
    m = re.match(r"([A-Za-z]+)(\d+)_(.+)", r["cfg"])
    if not m or m.group(1) != regime:
        continue
    for meas in measures:
        v = r.get(KEYS.get(meas, meas))
        if v is not None:
            vals[meas][m.group(3)].append(v)

BLUE, GREY, INK, MUTED, GRID, SURF = (42, 120, 214), (150, 149, 145), (11, 11, 11), (82, 81, 78), (230, 229, 225), (252, 252, 251)
try:
    F = ImageFont.truetype("arial.ttf", 13)
    FB = ImageFont.truetype("arialbd.ttf", 15)
    FS = ImageFont.truetype("arial.ttf", 11)
except OSError:
    F = FB = FS = ImageFont.load_default()

row_h, top, bottom, label_w, panel_w, gap = 26, 64, 34, 250, 300, 24
H = top + row_h * len(configs) + bottom
W = label_w + len(measures) * (panel_w + gap) + 10
img = Image.new("RGB", (W, H), SURF)
d = ImageDraw.Draw(img)
d.text((10, 8), title, fill=INK, font=FB)
for i, lab in enumerate(labels):
    y = top + i * row_h
    d.text((10, y + 5), lab, fill=INK, font=F)
for p, meas in enumerate(measures):
    x0 = label_w + p * (panel_w + gap)
    d.text((x0, 36), NAMES.get(meas, meas), fill=INK, font=F)
    allv = [v for c in configs for v in vals[meas].get(c, [])]
    vmax = max(allv) if allv else 1.0
    scale = (panel_w - 60) / vmax
    # gridlines at nice steps
    step = 10 ** len(str(int(vmax))) / 10 if vmax >= 1 else 0.1
    while vmax / step > 6:
        step *= 2
    t = 0.0
    while t <= vmax * 1.001:
        gx = x0 + t * scale
        d.line([(gx, top - 4), (gx, top + row_h * len(configs))], fill=GRID, width=1)
        d.text((gx - 6, top + row_h * len(configs) + 4), f"{t:g}", fill=MUTED, font=FS)
        t += step
    for i, c in enumerate(configs):
        v = vals[meas].get(c, [])
        if not v:
            continue
        y = top + i * row_h
        med = st.median(v)
        color = GREY if c.startswith("classic") else BLUE
        d.rounded_rectangle([x0, y + 6, x0 + max(med * scale, 2), y + row_h - 6], radius=3, fill=color)
        for vv in v:
            cx = x0 + vv * scale
            d.ellipse([cx - 2.5, y + row_h / 2 - 2.5, cx + 2.5, y + row_h / 2 + 2.5], fill=INK)
        txt = f"{med:.0f}" if med >= 100 else f"{med:.2f}"
        d.text((x0 + max(v + [med]) * scale + 6, y + 6), txt, fill=INK, font=FS)
d.text((10, H - 18), "bar = median over rounds, dot = one round; grey = Classic, blue = Remastered", fill=MUTED, font=FS)
img.save(out)
print("wrote", out, img.size)
