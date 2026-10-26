"""Temporal stability of the sun-visibility term alone (debug=1) during a slow dolly.

    python motion_sunvis.py <run dir>

Reads <run>/tri/abel_shadows_r7/*_forest_edge_golden__v{rem,obj,best}_mNN.png.
Writes <run>/motion_sunvis.md, <run>/cmp/sunvis_jerkmap.png (per-pixel mean
|2nd temporal difference|, shared scale) and <run>/cmp/sunvis_side.gif.
"""
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

RUN = Path(sys.argv[1])
SRC = RUN / "tri" / "abel_shadows_r7"
CMP = RUN / "cmp"
CMP.mkdir(exist_ok=True)
VARS = [("vrem", "sun visibility: Remastered (current)"), ("vobj", "sun visibility: + casters"), ("vbest", "sun visibility: best (PCSS 1 deg, 4096)")]


def lab(im, t):
    d = ImageDraw.Draw(im)
    d.rectangle([0, 0, 7 * len(t) + 8, 16], fill=(0, 0, 0))
    d.text((4, 3), t, fill=(255, 255, 255))
    return im


rows = ["| variant | frames | mean abs 1st diff | mean abs 2nd diff | p99 per-pixel 2nd diff | pixels with mean 2nd diff > 0.05 |", "|---|---|---|---|---|---|"]
maps, seqs = [], {}
for v, name in VARS:
    fs = sorted(SRC.glob("*_forest_edge_golden__%s_m*.png" % v))
    a = np.stack([np.asarray(Image.open(f).convert("L"), dtype=np.float32) / 255.0 for f in fs])
    d1 = np.abs(np.diff(a, axis=0)).mean()
    j = np.abs(a[2:] - 2 * a[1:-1] + a[:-2]).mean(axis=0)
    rows.append("| %s | %d | %.5f | %.5f | %.4f | %.3f%% |" % (v, len(fs), d1, j.mean(), np.percentile(j, 99), 100.0 * (j > 0.05).mean()))
    maps.append((name, j))
    seqs[v] = [Image.open(f).convert("RGB") for f in fs]
vmax = max(np.percentile(m, 99.8) for _, m in maps)
tiles = []
for name, m in maps:
    h = np.clip(m / vmax, 0, 1)
    tiles.append(lab(Image.fromarray((np.stack([h, h ** 2, h ** 4], -1) * 255).astype(np.uint8)), name + " - temporal 2nd diff"))
W = sum(t.width for t in tiles) + 16
out = Image.new("RGB", (W, tiles[0].height), (40, 40, 40))
x = 0
for t in tiles:
    out.paste(t, (x, 0))
    x += t.width + 8
out.save(CMP / "sunvis_jerkmap.png")
frames = []
n = min(len(s) for s in seqs.values())
for k in range(n):
    ts = [lab(seqs[v][k].crop((300, 180, 780, 500)), name) for v, name in VARS]
    im = Image.new("RGB", (sum(t.width for t in ts) + 12, ts[0].height), (40, 40, 40))
    x = 0
    for t in ts:
        im.paste(t, (x, 0))
        x += t.width + 6
    frames.append(im)
frames[0].save(CMP / "sunvis_side.gif", save_all=True, append_images=frames[1:], duration=66, loop=0)
(RUN / "motion_sunvis.md").write_text("\n".join(rows) + "\n", encoding="utf-8")
print("\n".join(rows))
