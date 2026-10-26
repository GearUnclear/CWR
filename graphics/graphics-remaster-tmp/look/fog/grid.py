"""grid.py <run dir> <out prefix> [labels]: 3-way (classic|base|fog) grids of 480 px panels, 4 views per image."""
import sys, glob, os, re
from PIL import Image
run, prefix = sys.argv[1], sys.argv[2]
labels = sys.argv[3].split(",") if len(sys.argv) > 3 else ["classic", "base", "fog"]
shots = {}
for p in glob.glob(os.path.join(run, "tri", "**", "*.png"), recursive=True):
    m = re.match(r"\d+_(.+)__(\w+)\.png$", os.path.basename(p))
    if m:
        shots[(m.group(1), m.group(2))] = p
order = ["sunrise_ridge", "forest_edge_golden", "coastline_sunset", "town_night", "dawn_valley", "pine_rays",
         "town_fog_night", "noon_fields", "overcast_broken", "overcast_heavy"]
views = [v for v in order if (v, labels[-1]) in shots] + sorted({v for v, _ in shots if v not in order})
W, H = 480, 270
for part in range(0, len(views), 4):
    vs = views[part:part + 4]
    s = Image.new("RGB", (W * len(labels), H * len(vs)))
    for r, v in enumerate(vs):
        for i, lab in enumerate(labels):
            if (v, lab) in shots:
                s.paste(Image.open(shots[(v, lab)]).convert("RGB").resize((W, H), Image.LANCZOS), (i * W, r * H))
    out = f"{prefix}_{part // 4}.png"
    s.save(out)
    print(out, vs)
