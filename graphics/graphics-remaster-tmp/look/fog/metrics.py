"""metrics.py <ref run dir> <fog run dir> [label=fog]: the critics' round-1 numbers on the
look-fog views (1920x1080 shots, sRGB 0..255), base = the landed B8 frame, fog = this lane.
Prints one block per view."""
import glob
import os
import sys

import numpy as np
from PIL import Image

ref, run = sys.argv[1], sys.argv[2]
label = sys.argv[3] if len(sys.argv) > 3 else "fog"


def shot(d, view, lab):
    hits = glob.glob(os.path.join(d, "tri", "foglane_*", f"*_{view}__{lab}.png"))
    return np.asarray(Image.open(hits[0]).convert("RGB")).astype(float) if hits else None


def lum(a):
    return a[..., 0] * 0.2126 + a[..., 1] * 0.7152 + a[..., 2] * 0.0722


def thumb(a):
    return np.asarray(Image.fromarray(a.astype(np.uint8)).resize((480, 270), Image.LANCZOS)).astype(float)


def sat(a):
    mx, mn = a.max(axis=2), a.min(axis=2)
    return np.where(mx > 0, (mx - mn) / np.maximum(mx, 1e-6), 0)


def fmt(v):
    return "(" + ",".join(f"{x:.0f}" for x in v) + ")"


def views():
    out = {}
    for v in ["sunrise_ridge", "forest_edge_golden", "coastline_sunset", "overcast_broken", "overcast_heavy",
              "town_night", "noon_vista", "dawn_valley", "pine_rays", "town_fog_night", "noon_fields",
              "sinai_lowsun_fp", "sinai_desert_noon"]:
        b, f, c = shot(ref, v, "base"), shot(run, v, label), shot(ref, v, "classic")
        if b is not None and f is not None:
            out[v] = (c, b, f)
    return out


for v, (c, b, f) in views().items():
    d = np.abs(f - b).mean() / 255
    dc = np.abs(f - c).mean() / 255 if c is not None else float("nan")
    tb, tf = thumb(b), thumb(f)
    print(f"== {v}: mean |fog-base| {d:.3f}, |fog-classic| {dc:.3f}; thumb sat base {sat(tb).mean():.3f} fog "
          f"{sat(tf).mean():.3f}; thumb p1 lum base {np.percentile(lum(tb), 1) / 255:.3f} fog "
          f"{np.percentile(lum(tf), 1) / 255:.3f}; frame min fog {f.min():.0f} base {b.min():.0f}")
    if v == "sunrise_ridge":
        lb = lum(b)
        for x in (700, 750, 800, 850, 900):
            col = np.convolve(lb[350:600, x - 2:x + 3].mean(axis=1), np.ones(5) / 5, mode="same")
            r = 350 + int(np.argmax(np.abs(np.diff(col[5:-5])))) + 5
            sky_f, hill_f = f[r - 14:r - 5, x - 3:x + 4].mean(axis=(0, 1)), f[r + 5:r + 14, x - 3:x + 4].mean(axis=(0, 1))
            sky_b, hill_b = b[r - 14:r - 5, x - 3:x + 4].mean(axis=(0, 1)), b[r + 5:r + 14, x - 3:x + 4].mean(axis=(0, 1))
            print(f"   x={x} ridge row {r}: fog sky {fmt(sky_f)} hill {fmt(hill_f)} | base sky {fmt(sky_b)} hill {fmt(hill_b)}")
    if v == "sinai_lowsun_fp":
        for name, box in (("humvee", (120, 560, 860, 780)), ("wheel-front", (180, 640, 300, 760)),
                          ("wheel-rear", (660, 640, 800, 760))):
            x0, y0, x1, y1 = box
            rb, rf = lum(b[y0:y1, x0:x1]), lum(f[y0:y1, x0:x1])
            print(f"   {name}: p5 base {np.percentile(rb, 5):.0f} fog {np.percentile(rf, 5):.0f}; median base "
                  f"{np.median(rb):.0f} fog {np.median(rf):.0f}")
        hill = (0, 380, 500, 450)
        x0, y0, x1, y1 = hill
        print(f"   left hill {fmt(f[y0:y1, x0:x1].mean(axis=(0, 1)))} base {fmt(b[y0:y1, x0:x1].mean(axis=(0, 1)))}; "
              f"sky above {fmt(f[300:360, x0:x1].mean(axis=(0, 1)))} base {fmt(b[300:360, x0:x1].mean(axis=(0, 1)))}")
    if v == "dawn_valley":
        lf, lb = lum(f), lum(b)
        print(f"   bottom-left 700x380: p5 fog {np.percentile(lf[700:, :700], 5):.0f} base "
              f"{np.percentile(lb[700:, :700], 5):.0f}; std fog {lf[700:, :700].std():.1f} base {lb[700:, :700].std():.1f}")
        print(f"   right 550x400 (x 1370.., y 300..700): std fog {lf[300:700, 1370:].std():.1f} base "
              f"{lb[300:700, 1370:].std():.1f}; mean fog {fmt(f[300:700, 1370:].mean(axis=(0, 1)))}")
    if v == "town_fog_night":
        for name, box in (("sky upper right", (1600, 20, 1900, 150)), ("sky upper left", (40, 20, 300, 120)),
                          ("jeep", (980, 540, 1100, 650))):
            x0, y0, x1, y1 = box
            print(f"   {name}: fog {fmt(f[y0:y1, x0:x1].mean(axis=(0, 1)))} base {fmt(b[y0:y1, x0:x1].mean(axis=(0, 1)))}"
                  f"; std fog {lum(f[y0:y1, x0:x1]).std():.1f} base {lum(b[y0:y1, x0:x1]).std():.1f}")
    if v == "town_night":
        for name, box in (("sky", (1600, 20, 1900, 150)), ("tree line", (1500, 330, 1900, 420))):
            x0, y0, x1, y1 = box
            print(f"   {name}: fog {fmt(f[y0:y1, x0:x1].mean(axis=(0, 1)))} base {fmt(b[y0:y1, x0:x1].mean(axis=(0, 1)))}")
    if v in ("noon_fields", "sinai_desert_noon", "noon_vista", "overcast_broken"):
        band = (380, 560) if v != "noon_vista" else (300, 600)
        y0, y1 = band
        print(f"   band rows {y0}..{y1}: fog {fmt(f[y0:y1].mean(axis=(0, 1)))} base {fmt(b[y0:y1].mean(axis=(0, 1)))}; "
              f"blue minus red fog {(f[y0:y1, :, 2] - f[y0:y1, :, 0]).mean():.1f} base "
              f"{(b[y0:y1, :, 2] - b[y0:y1, :, 0]).mean():.1f}")
