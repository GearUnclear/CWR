"""Temporal analysis of the round-6 motion captures (shadow swimming / PCSS dither).

    python motion_analyse.py <run dir>

For each view and variant (rem, obj, best) in <run>/tri/abel_shadows_motion/:
  * static: mean |f(k+1) - f(k)| over the 4 fixed-camera frames (should be ~0)
  * moving: mean |f(k+1) - f(k)| over the 16 dolly frames (content motion + artefacts)
  * jerk:   mean |f(k+1) - 2 f(k) + f(k-1)| over the dolly frames. A 2 cm/frame
            dolly moves content smoothly, so this second difference is small for
            smooth motion; shadow-map texel snapping and screen-fixed dither that
            the content slides through raise it.
  * jerk95: 95th percentile of the per-pixel temporal mean of |2nd difference|
Writes <run>/motion_metrics.md, a per-pixel jerk heat map strip per view
(<run>/cmp/<view>__jerkmap.png) and animated GIF crops (<run>/cmp/<view>__<variant>.gif)
plus a side-by-side GIF (<run>/cmp/<view>__side.gif).
"""
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

RUN = Path(sys.argv[1])
SRC = RUN / "tri" / "abel_shadows_motion"
CMP = RUN / "cmp"
CMP.mkdir(exist_ok=True)
VARIANTS = ["rem", "obj", "best"]
NAMES = {"rem": "Remastered (current)", "obj": "+ tree/building casters", "best": "+ casters, PCSS 1 deg, 4096 tight, contact"}
CROPS = {"forest_edge_golden": (300, 180, 780, 500), "lowsun_fp": (0, 300, 720, 600)}


def frames(view, var):
    files = sorted(SRC.glob("*_%s__%s_*.png" % (view, var)))
    st = [f for f in files if f.stem.split("_")[-1].startswith("s")]
    mv = [f for f in files if f.stem.split("_")[-1].startswith("m")]
    load = lambda fs: np.stack([np.asarray(Image.open(f).convert("RGB"), dtype=np.float32) / 255.0 for f in fs])
    return load(st), load(mv), mv


def label(im, text):
    d = ImageDraw.Draw(im)
    d.rectangle([0, 0, 7 * len(text) + 8, 16], fill=(0, 0, 0))
    d.text((4, 3), text, fill=(255, 255, 255))
    return im


rows = ["| view | variant | static mean abs diff | moving mean abs diff | jerk mean | jerk p95 (per-pixel mean) | jerk mean in crop |", "|---|---|---|---|---|---|---|"]
for view in ("forest_edge_golden", "lowsun_fp"):
    maps = []
    gif_tiles = {}
    for var in VARIANTS:
        st, mv, mvfiles = frames(view, var)
        if len(mv) < 3:
            print("missing", view, var, len(mv))
            continue
        sd = np.abs(np.diff(st, axis=0)).mean() if len(st) > 1 else float("nan")
        md = np.abs(np.diff(mv, axis=0)).mean()
        j = np.abs(mv[2:] - 2 * mv[1:-1] + mv[:-2]).mean(axis=(0, 3))  # per-pixel
        x0, y0, x1, y1 = CROPS[view]
        rows.append("| %s | %s | %.5f | %.5f | %.5f | %.5f | %.5f |" % (view, var, sd, md, j.mean(), np.percentile(j, 95), j[y0:y1, x0:x1].mean()))
        maps.append((var, j))
        gif_tiles[var] = [Image.fromarray((f[y0:y1, x0:x1] * 255).astype(np.uint8)) for f in mv]
    # heat maps, shared scale
    vmax = max(np.percentile(m, 99.5) for _, m in maps)
    tiles = []
    for var, m in maps:
        h = np.clip(m / vmax, 0, 1)
        rgb = np.stack([h, h ** 2, h ** 4], axis=-1)
        tiles.append(label(Image.fromarray((rgb * 255).astype(np.uint8)), NAMES[var] + " - temporal 2nd difference"))
    W = sum(t.width for t in tiles) + 8 * (len(tiles) - 1)
    strip = Image.new("RGB", (W, tiles[0].height), (40, 40, 40))
    x = 0
    for t in tiles:
        strip.paste(t, (x, 0))
        x += t.width + 8
    strip.save(CMP / ("%s__jerkmap.png" % view))
    # GIFs: 2x nearest-upscaled crops
    side = []
    for var in VARIANTS:
        fr = [t.resize((t.width * 2, t.height * 2), Image.NEAREST) for t in gif_tiles[var]]
        fr[0].save(CMP / ("%s__%s.gif" % (view, var)), save_all=True, append_images=fr[1:], duration=66, loop=0)
    n = len(gif_tiles["rem"])
    for k in range(n):
        ts = [label(gif_tiles[v][k].copy(), NAMES[v]) for v in VARIANTS]
        Wd = sum(t.width for t in ts) + 6 * (len(ts) - 1)
        im = Image.new("RGB", (Wd, ts[0].height), (40, 40, 40))
        x = 0
        for t in ts:
            im.paste(t, (x, 0))
            x += t.width + 6
        side.append(im)
    side[0].save(CMP / ("%s__side.gif" % view), save_all=True, append_images=side[1:], duration=66, loop=0)
(RUN / "motion_metrics.md").write_text("\n".join(rows) + "\n", encoding="utf-8")
print("\n".join(rows))
