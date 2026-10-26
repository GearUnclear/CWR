"""Build BEST_<view>.png three-way strips: Classic | Remastered (current) | Night prototype, GPU ms per panel."""
import json, re, sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
R = Path(r"D:\Arma_CWA_worktrees\lighting\tmp\research\night")
def font(sz):
    for n in ("arialbd.ttf", "arial.ttf"):
        p = Path("C:/Windows/Fonts") / n
        if p.exists(): return ImageFont.truetype(str(p), sz)
    return ImageFont.load_default()
def box(d, xy, text, f):
    b = d.textbbox(xy, text, font=f); d.rectangle([b[0]-5, b[1]-3, b[2]+5, b[3]+3], fill=(0,0,0)); d.text(xy, text, font=f, fill=(255,255,255))
def rows(run):
    return {(r["view"], r["label"]): r for r in json.loads((R/run/"timings.json").read_text(encoding="utf-8"))}
jobs = [
  ("run10", "road_flat_drive_moonless", "v9", "moonless night, driver's eye: physical low beam on a flat road"),
  ("run10", "road_flat_moonless", "v9", "moonless night, third person"),
  ("run10", "road_flat_head", "v9", "full moon 36 deg, third person: moonlit field, moon shadow, headlights"),
  ("run10", "town_night", "v9", "full moon 69 deg, six street lamps and a campfire"),
  ("run10", "town_lamps", "v9", "moonless, street lamps only"),
  ("run8", "town_moonlow", "v8", "full moon 24 deg in the east"),
  ("run8", "road_head", "v8", "full moon, 14 deg uphill road (hard case for a low beam)"),
  ("run8", "moon_sky", "v8", "looking at the moon: moonlit crowns, star splats"),
  ("run8", "nv_town", "v8", "night vision: unchanged by design"),
]
for run, view, lab, caption in jobs:
    rw = rows(run)
    panels = [("classic", "Classic"), ("remastered", "Remastered (current)"), (lab, "Night prototype " + lab)]
    ims = [Image.open(R/run/"shots"/f"{view}__{l}.png").convert("RGB") for l, _ in panels]
    w, h = ims[0].size; gap, title = 6, 40
    c = Image.new("RGB", (3*w + 2*gap, title + h), (20,20,20)); d = ImageDraw.Draw(c)
    d.text((8, 8), f"{view}: {caption}", font=font(20), fill=(235,235,235))
    for i, (im, (l, nm)) in enumerate(zip(ims, panels)):
        x0 = i*(w+gap); c.paste(im, (x0, title)); box(d, (x0+10, title+8), nm, font(18))
        r = rw.get((view, l))
        if r and r.get("frame") is not None:
            t = f"GPU frame {r['frame']:.2f} ms"
            if r.get("shadow") is not None: t += f" (shadow {r['shadow']:.2f})"
            box(d, (x0+10, title+h-28), t, font(14))
    out = R/f"BEST_{view}.png"; c.save(out); print(out)
