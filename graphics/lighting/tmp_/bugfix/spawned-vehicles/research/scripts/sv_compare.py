"""Comparison sheets and numbers for the spawned-vehicles fix.

Usage: python -I sv_compare.py <before_dir> <after_dir> <out_dir>

Each run dir holds <view>__<mode>.png (make_pairs.py output). For every view
and mode it builds one sheet: BEFORE spawned | AFTER spawned | AFTER placed
(and BEFORE placed when present), and prints difference statistics:
  mean |a-b| over RGB (display levels) and the share of pixels whose largest
  channel difference exceeds 4 and 10 levels.
"""
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont
import numpy as np

before, after, out = (Path(a) for a in sys.argv[1:4])
out.mkdir(parents=True, exist_ok=True)

VIEWS = ["lowsun_fp", "hull_close", "noon_fields", "town_night", "tank_gunner_interior",
         "sinai_lowsun_fp", "sinai_hull_close"]
MODES = ["remastered", "classic", "debug"]


def load(d, name):
    p = d / f"{name}.png"
    return np.asarray(Image.open(p).convert("RGB")).astype(np.int16) if p.exists() else None


def font(size):
    for f in ("C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/arial.ttf"):
        try:
            return ImageFont.truetype(f, size)
        except OSError:
            pass
    return ImageFont.load_default()


def stats(a, b):
    d = np.abs(a - b)
    m = d.max(axis=2)
    return float(d.mean()), float((m > 4).mean() * 100), float((m > 10).mean() * 100)


def magenta_share(a, rows=None):
    sub = a if rows is None else a[rows[0]:rows[1]]
    mask = (sub[..., 0] > 240) & (sub[..., 1] < 15) & (sub[..., 2] > 240)
    return float(mask.mean() * 100)


lines = []
F = font(22)
for view in VIEWS:
    for mode in MODES:
        panels = [
            ("BEFORE: script-spawned (branch tip)", load(before, f"{view}__{mode}")),
            ("AFTER: script-spawned (fix)", load(after, f"{view}__{mode}")),
            ("AFTER: mission-placed", load(after, f"placed_{view}__{mode}")),
            ("BEFORE: mission-placed", load(before, f"placed_{view}__{mode}")),
        ]
        panels = [(t, a) for t, a in panels if a is not None]
        if len(panels) < 2:
            continue
        h, w = panels[0][1].shape[:2]
        sheet = Image.new("RGB", (w * len(panels), h + 40), (20, 20, 20))
        d = ImageDraw.Draw(sheet)
        for i, (t, a) in enumerate(panels):
            sheet.paste(Image.fromarray(a.astype(np.uint8)), (i * w, 40))
            d.text((i * w + 10, 6), t, fill=(255, 255, 255), font=F)
        sheet.save(out / f"cmp_{view}__{mode}.png")
        bs = load(before, f"{view}__{mode}")
        af = load(after, f"{view}__{mode}")
        pa = load(after, f"placed_{view}__{mode}")
        pb = load(before, f"placed_{view}__{mode}")
        row = [view, mode]
        if bs is not None and af is not None:
            row.append("before_vs_after=%.2f/%.2f%%/%.2f%%" % stats(bs, af))
        if af is not None and pa is not None:
            row.append("after_vs_placed=%.2f/%.2f%%/%.2f%%" % stats(af, pa))
        if pa is not None and pb is not None:
            row.append("placed_before_vs_after=%.2f/%.2f%%/%.2f%%" % stats(pb, pa))
        if bs is not None and pb is not None:
            row.append("before_spawned_vs_placed=%.2f/%.2f%%/%.2f%%" % stats(bs, pb))
        if mode == "debug":
            for tag, a in (("before", bs), ("after", af), ("placed_after", pa), ("placed_before", pb)):
                if a is not None:
                    # lower 60% of the frame: below the sky in these views
                    row.append("magenta_%s=%.2f%%" % (tag, magenta_share(a, (int(a.shape[0] * 0.4), a.shape[0]))))
        lines.append(";".join(row))

(out / "stats.txt").write_text("\n".join(lines) + "\n")
print("\n".join(lines))
