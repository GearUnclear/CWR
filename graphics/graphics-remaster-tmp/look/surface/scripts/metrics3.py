"""Critic round 2 checks for the look-surface lane.

  python metrics3.py <run dir> [--md out.md]

Lane off = suffix `base` (remastered,surfaces=0), lane on = `surf`, the
Original = `classic`. Boxes are in the 960 x 640 hero frame.
  - changed share: pixels whose 8-bit luma moved by more than 16 (and 8)
    levels, frames scaled to 480 px wide (the thumbnail);
  - noon ground: luma std of the bottom third, on / off;
  - shade to sun: mean luma of a shaded and a sunlit patch of the same
    object (boxes picked on the lane-off frame), and their ratio;
  - paint: HSV saturation of the jeep body (pixels under 110 luma);
  - the windshield and the town facade boxes the critics named.
"""
import argparse
import re
from pathlib import Path

import numpy as np
from PIL import Image

PAT = re.compile(r"^\d+_(?P<view>.+?)__(?P<suf>[^_].*)\.png$")

PAINT = {"jeep_close_noon": (400, 240, 620, 400), "noon_fields": (530, 350, 720, 450)}
NOON = ["noon_fields", "sinai_desert_noon", "jeep_close_noon", "town_noon"]
# (label, shaded box, sunlit box) per view; filled from the frames.
SHADE = {"jeep_close_noon": [("jeep side / fender top", (555, 358, 615, 378), (625, 325, 680, 334)),
                              ("jeep side panel / canvas top", (455, 352, 545, 378), (440, 262, 600, 272)),
                              ("ground: jeep shadow / open", (420, 412, 620, 428), (660, 425, 780, 445))]}
BOXES = {
    ("lowsun_fp", "windshield"): (108, 305, 135, 340),
    ("town_noon", "upper storey"): (190, 185, 360, 250),
    ("town_noon", "window left"): (205, 292, 228, 332),
    ("town_noon", "window right"): (268, 292, 292, 332),
    ("town_noon", "brick front"): (620, 300, 750, 355),
    ("jeep_close_noon", "spare tyre"): (395, 300, 430, 380),
}


def collect(run):
    d = {}
    for p in Path(run).rglob("*.png"):
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


def box(a, b):
    x0, y0, x1, y1 = b
    return a[y0:y1, x0:x1]


def sat(a):
    mx = a.max(2)
    mn = a.min(2)
    return (mx - mn) / np.maximum(mx, 1.0)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--md", default="")
    ap.add_argument("--off", default="base")
    ap.add_argument("--on", default="surf")
    a = ap.parse_args()
    d = collect(a.run)
    views = sorted({v for v, s in d})
    out = []
    out.append("| view | >16 levels at 480 px | >8 levels | Original vs Overhaul >16 |")
    out.append("|---|---|---|---|")
    for v in views:
        if (v, a.off) not in d or (v, a.on) not in d:
            continue
        off = luma(load(d[(v, a.off)], (480, 320)))
        on = luma(load(d[(v, a.on)], (480, 320)))
        dd = np.abs(on - off)
        cl = ""
        if (v, "classic") in d:
            c = luma(load(d[(v, "classic")], (480, 320)))
            cl = f"{100 * (np.abs(on - c) > 16).mean():.1f}%"
        out.append(f"| {v} | {100 * (dd > 16).mean():.1f}% | {100 * (dd > 8).mean():.1f}% | {cl} |")
    out.append("")
    out.append("| noon view | bottom-third luma mean off / on | std off / on | ratio |")
    out.append("|---|---|---|---|")
    for v in NOON:
        if (v, a.off) not in d or (v, a.on) not in d:
            continue
        r = []
        for s in (a.off, a.on):
            L = luma(load(d[(v, s)]))
            b = L[L.shape[0] * 2 // 3:]
            r.append((b.mean(), b.std()))
        out.append(f"| {v} | {r[0][0]:.1f} / {r[1][0]:.1f} | {r[0][1]:.1f} / {r[1][1]:.1f} | {r[1][1] / r[0][1]:.2f} |")
    out.append("")
    out.append("| view, object | shade off / on | sunlit off / on | shade/sun off / on |")
    out.append("|---|---|---|---|")
    for v, rows in SHADE.items():
        if (v, a.off) not in d or (v, a.on) not in d:
            continue
        for label, bs, bl in rows:
            vals = []
            for s in (a.off, a.on):
                L = luma(load(d[(v, s)]))
                vals.append((box(L, bs).mean(), box(L, bl).mean()))
            out.append(f"| {v}, {label} | {vals[0][0]:.1f} / {vals[1][0]:.1f} | {vals[0][1]:.1f} / {vals[1][1]:.1f} | "
                       f"{vals[0][0] / vals[0][1]:.2f} / {vals[1][0] / vals[1][1]:.2f} |")
    out.append("")
    out.append("| view, paint box | S off / on | RGB off | RGB on |")
    out.append("|---|---|---|---|")
    for v, b in PAINT.items():
        if (v, a.off) not in d or (v, a.on) not in d:
            continue
        r = []
        mask = None
        for s in (a.off, a.on):
            A = box(load(d[(v, s)]), b)
            if mask is None:
                mask = luma(A) < 110
            r.append((sat(A)[mask].mean(), A[mask].mean(0)))
        out.append(f"| {v} | {r[0][0]:.3f} / {r[1][0]:.3f} | {np.round(r[0][1], 1).tolist()} | {np.round(r[1][1], 1).tolist()} |")
    out.append("")
    out.append("| view, box | RGB Original | RGB off | RGB on |")
    out.append("|---|---|---|---|")
    for (v, label), b in BOXES.items():
        r = []
        for s in ("classic", a.off, a.on):
            if (v, s) in d:
                r.append(np.round(box(load(d[(v, s)]), b).reshape(-1, 3).mean(0), 1).tolist())
            else:
                r.append("-")
        out.append(f"| {v}, {label} {b} | {r[0]} | {r[1]} | {r[2]} |")
    text = "\n".join(out)
    print(text)
    if a.md:
        Path(a.md).write_text(text + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
