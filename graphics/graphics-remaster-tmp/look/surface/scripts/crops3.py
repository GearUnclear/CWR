"""Labelled crops across suffixes of one capture run.

  python crops3.py <run> <out.png> <view> x0 y0 x1 y1 scale suf[=label] [suf[=label] ...]

Picks <seq>_<view>__<suf>.png exactly (no prefix matches), crops, scales
(nearest from 2x up) and lays the panels side by side with a label bar.
"""
import re
import sys
from pathlib import Path

from PIL import Image, ImageDraw


def find(run, view, suf):
    pat = re.compile(r"^\d+_" + re.escape(view) + "__" + re.escape(suf) + r"\.png$")
    hits = [p for p in Path(run).rglob("*.png") if pat.match(p.name)]
    if not hits:
        raise SystemExit(f"missing {view}__{suf} in {run}")
    return sorted(hits)[-1]


def main():
    run, out, view = sys.argv[1], sys.argv[2], sys.argv[3]
    x0, y0, x1, y1 = (int(v) for v in sys.argv[4:8])
    sc = float(sys.argv[8])
    panels = []
    for spec in sys.argv[9:]:
        suf, _, label = spec.partition("=")
        im = Image.open(find(run, view, suf)).convert("RGB").crop((x0, y0, x1, y1))
        im = im.resize((int(im.width * sc), int(im.height * sc)), Image.NEAREST if sc >= 2 else Image.LANCZOS)
        bar = Image.new("RGB", (im.width, 18), (20, 20, 20))
        ImageDraw.Draw(bar).text((4, 3), f"{label or suf} | {view}", fill=(240, 240, 240))
        p = Image.new("RGB", (im.width, im.height + 18))
        p.paste(bar, (0, 0))
        p.paste(im, (0, 18))
        panels.append(p)
    W = sum(p.width for p in panels) + 6 * (len(panels) - 1)
    o = Image.new("RGB", (W, panels[0].height), (255, 255, 255))
    x = 0
    for p in panels:
        o.paste(p, (x, 0))
        x += p.width + 6
    o.save(out)
    print(out, o.size)


if __name__ == "__main__":
    main()
