"""Pixel checks for the critic round of the look-surface lane.

  python metrics.py <run dir> [<run dir> ...] [--md out.md]

Finds <seq>_<view>__<suffix>.png (later runs win) and reports, lane off
(`base`) against lane on (`surf`), with the Original (`classic`) where useful:
  - whole-frame mean HSV saturation at 480 px wide (0..100) and the share of
    pixels whose 8-bit luma moved by more than 8 levels;
  - crowns: mean HSV saturation (0..1) and RGB over foliage pixels of fixed
    boxes (foliage = green-dominant, not sky, by the lane-off frame);
  - the top-edge band of forest_morning (rows 0-2, x 780-900);
  - paint: the jeep body in jeep_close_noon and noon_fields;
  - the noon ground (sunlit sand / field boxes): mean luma;
  - low-sun foreground: luma std of the sunlit ground box.
Boxes are in the 960 x 640 frame of the hero captures.
"""
import argparse
import re
from pathlib import Path

import numpy as np
from PIL import Image

PAT = re.compile(r"^\d+_(?P<view>.+?)__(?P<suf>[^_].*)\.png$")

CROWNS = {
    "forest_morning": [("big tree right", (560, 0, 960, 330)), ("broadleaf left", (100, 80, 360, 300))],
    "forest_edge_golden": [("crowns", (0, 0, 960, 330))],
    "town_noon": [("birch", (250, 0, 620, 260))],
}
PAINT = {
    "jeep_close_noon": (400, 240, 620, 400),
    "noon_fields": (530, 350, 720, 450),
}
NOON_GROUND = {
    "sinai_desert_noon": (0, 450, 960, 640),
    "noon_fields": (0, 470, 960, 640),
    "jeep_close_noon": (0, 520, 960, 640),
}
LOWSUN_GROUND = {
    "lowsun_fp": (0, 470, 960, 640),
    "forest_edge_golden": (0, 500, 960, 640),
    "sinai_lowsun_fp": (0, 470, 960, 640),
}


def collect(runs):
    d = {}
    for r in runs:
        for p in Path(r).rglob("*.png"):
            m = PAT.match(p.name)
            if m:
                d[(m.group("view"), m.group("suf"))] = p
    return d


def load(p, size=None):
    im = Image.open(p).convert("RGB")
    if size:
        im = im.resize(size, Image.LANCZOS)
    return np.asarray(im).astype(np.float64)


def luma(a):
    return a[..., 0] * 0.2126 + a[..., 1] * 0.7152 + a[..., 2] * 0.0722


def hsv_s(a):
    mx = a.max(2)
    mn = a.min(2)
    return (mx - mn) / np.maximum(mx, 1.0)


def box(a, b):
    x0, y0, x1, y1 = b
    return a[y0:y1, x0:x1]


def foliage_mask(a):
    r, g, b = a[..., 0], a[..., 1], a[..., 2]
    lum = luma(a)
    return (g >= r * 0.92) & (g >= b) & (lum > 12) & (lum < 220) & ~((b > g) & (b > r))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("runs", nargs="+")
    ap.add_argument("--md")
    ap.add_argument("--on", default="surf", help="the suffix read as lane on")
    a = ap.parse_args()
    f = collect(a.runs)
    if a.on != "surf":
        f = {(v, ("surf" if s == a.on else ("_surf" if s == "surf" else s))): p for (v, s), p in f.items()}
    views = sorted({v for (v, _s) in f})
    out = ["## Whole frame (480 px wide)", "",
           "| view | sat Original | sat lane off | sat lane on | px >8 levels on vs off | px >8 Original vs on | mean luma off / on |",
           "|---|---|---|---|---|---|---|"]
    for v in views:
        if (v, "base") not in f or (v, "surf") not in f:
            continue
        B = load(f[(v, "base")], (480, 320))
        S = load(f[(v, "surf")], (480, 320))
        C = load(f[(v, "classic")], (480, 320)) if (v, "classic") in f else None
        sat = lambda X: f"{hsv_s(X).mean() * 100:.1f}" if X is not None else "-"
        d_on = (np.abs(luma(S) - luma(B)) > 8).mean() * 100
        d_c = (np.abs(luma(S) - luma(C)) > 8).mean() * 100 if C is not None else float("nan")
        out.append(f"| {v} | {sat(C)} | {sat(B)} | {sat(S)} | {d_on:.1f}% | {d_c:.1f}% | {luma(B).mean():.1f} / {luma(S).mean():.1f} |")
    out += ["", "## Crowns (foliage pixels of the lane-off frame, full size)", "",
            "| view | box | n px | HSV S off | HSV S on | RGB off | RGB on |", "|---|---|---|---|---|---|---|"]
    for v, boxes in CROWNS.items():
        if (v, "base") not in f or (v, "surf") not in f:
            continue
        B = load(f[(v, "base")])
        S = load(f[(v, "surf")])
        for name, bx in boxes:
            bb, ss = box(B, bx), box(S, bx)
            m = foliage_mask(bb)
            if m.sum() == 0:
                continue
            rb = bb[m].mean(0)
            rs = ss[m].mean(0)
            out.append(f"| {v} | {name} {bx} | {int(m.sum())} | {hsv_s(bb)[m].mean():.3f} | {hsv_s(ss)[m].mean():.3f} | "
                       f"({rb[0]:.1f}, {rb[1]:.1f}, {rb[2]:.1f}) | ({rs[0]:.1f}, {rs[1]:.1f}, {rs[2]:.1f}) |")
    if ("forest_morning", "base") in f and ("forest_morning", "surf") in f:
        B = luma(load(f[("forest_morning", "base")]))
        S = luma(load(f[("forest_morning", "surf")]))
        rows = [f"{(S[y, 780:900] - B[y, 780:900]).mean():+.1f}" for y in range(4)]
        ref = (S[8:20, 780:900] - B[8:20, 780:900]).mean()
        out += ["", f"Top edge, forest_morning x 780-900, on minus off, rows 0..3: {', '.join(rows)} (rows 8-19: {ref:+.1f})"]
    out += ["", "## Paint (body box, full size)", "", "| view | box | HSV S off | HSV S on | RGB off | RGB on |",
            "|---|---|---|---|---|---|"]
    for v, bx in PAINT.items():
        if (v, "base") not in f or (v, "surf") not in f:
            continue
        bb = box(load(f[(v, "base")]), bx)
        ss = box(load(f[(v, "surf")]), bx)
        # The body: not the sky or the bright ground around it.
        m = luma(bb) < 110
        rb, rs = bb[m].mean(0), ss[m].mean(0)
        out.append(f"| {v} | {bx} | {hsv_s(bb)[m].mean():.3f} | {hsv_s(ss)[m].mean():.3f} | "
                   f"({rb[0]:.1f}, {rb[1]:.1f}, {rb[2]:.1f}) | ({rs[0]:.1f}, {rs[1]:.1f}, {rs[2]:.1f}) |")
    out += ["", "## Noon ground (mean luma)", "", "| view | box | Original | lane off | lane on |", "|---|---|---|---|---|"]
    for v, bx in NOON_GROUND.items():
        if (v, "base") not in f or (v, "surf") not in f:
            continue
        c = luma(box(load(f[(v, "classic")]), bx)).mean() if (v, "classic") in f else float("nan")
        out.append(f"| {v} | {bx} | {c:.1f} | {luma(box(load(f[(v, 'base')]), bx)).mean():.1f} | "
                   f"{luma(box(load(f[(v, 'surf')]), bx)).mean():.1f} |")
    out += ["", "## Low-sun foreground (luma std)", "", "| view | box | Original | lane off | lane on | on / off |",
            "|---|---|---|---|---|---|"]
    for v, bx in LOWSUN_GROUND.items():
        if (v, "base") not in f or (v, "surf") not in f:
            continue
        c = luma(box(load(f[(v, "classic")]), bx)).std() if (v, "classic") in f else float("nan")
        b = luma(box(load(f[(v, "base")]), bx)).std()
        s = luma(box(load(f[(v, "surf")]), bx)).std()
        out.append(f"| {v} | {bx} | {c:.1f} | {b:.1f} | {s:.1f} | {s / b:.2f} |")
    text = "\n".join(out)
    print(text)
    if a.md:
        Path(a.md).write_text(text + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
