"""metrics.py DIR : the critics' numbers on a collected folder (Original vs Overhaul)."""
import sys
from pathlib import Path

import numpy as np
from PIL import Image

d = Path(sys.argv[1])


def img(v, lab):
    p = d / f"{v}__{lab}.png"
    return np.asarray(Image.open(p).convert("RGB")).astype(np.float64) if p.exists() else None


def box(a, x0, y0, x1, y1):
    h, w = a.shape[:2]
    return a[int(y0 * h):int(y1 * h), int(x0 * w):int(x1 * w)]


def luma(a):
    return a @ np.array([0.2126, 0.7152, 0.0722])


def fmt(v):
    return "(" + ",".join(f"{x:.0f}" for x in v) + ")"


def report(name, v, region, fn):
    out = []
    for lab in ("classic", "overhaul"):
        a = img(v, lab)
        if a is None:
            out.append(f"{lab}: missing")
            continue
        out.append(f"{lab}: {fn(box(a, *region))}")
    print(f"{name:34s} " + " | ".join(out))


mean = lambda b: fmt(b.reshape(-1, 3).mean(0)) + f" L{luma(b).mean():.0f}"
rb = lambda b: f"R-B {(b[..., 0] - b[..., 2]).mean():.1f} R-G {(b[..., 0] - b[..., 1]).mean():.1f} L{luma(b).mean():.0f}"


def sat(b):
    mx, mn = b.max(-1), b.min(-1)
    return f"sat {np.where(mx > 0, (mx - mn) / np.maximum(mx, 1), 0).mean():.3f} L{luma(b).mean():.0f}"


def white(b):
    return f"255px {(b.min(-1) >= 254).sum()} " + mean(b)


def hp(b):
    l = luma(b)
    k = np.ones((3, 3)) / 9.0
    from numpy.lib.stride_tricks import sliding_window_view
    w = sliding_window_view(l, (3, 3)).mean(axis=(-1, -2))
    r = l[1:-1, 1:-1] - w
    return f"hp3 std {r.std():.2f}"


for v in ("firefight_1", "firefight_2", "firefight_3"):
    report(v + " bottom band", v, (0, 0.75, 1, 1), mean)
    report(v + " sky over horizon", v, (0, 0.40, 1, 0.50), mean)
report("rain_day right hill", "rain_day", (0.72, 0.42, 0.9, 0.5), mean)
report("rain_day gap", "rain_day", (0.42, 0.40, 0.55, 0.55), mean)
report("rain_day sky band", "rain_day", (0.3, 0.30, 0.7, 0.36), mean)
report("pine_rays ground", "pine_rays", (0.0, 0.75, 1, 1), rb)
report("woods ground", "woods_pine2_evening", (0.0, 0.75, 1, 1), rb)
report("sinai_lowsun ground", "sinai_lowsun_fp", (0.0, 0.8, 1, 1), rb)
report("lowsun_fp ground", "lowsun_fp", (0.0, 0.8, 1, 1), rb)
report("town_night fire box", "town_night", (0.66, 0.6, 0.8, 0.82), white)
report("town_night fire ground", "town_night", (0.6, 0.8, 0.85, 0.9), rb)
report("noon_fields foreground", "noon_fields", (0, 0.7, 1, 1), mean)
report("jeep_close_noon foreground", "jeep_close_noon", (0, 0.8, 1, 1), mean)
report("sinai_desert_noon foreground", "sinai_desert_noon", (0, 0.8, 1, 1), mean)
report("town_noon grass", "town_noon", (0, 0.75, 1, 1), sat)
report("town_noon plaster", "town_noon", (0.24, 0.26, 0.36, 0.36), mean)
report("forest_edge_golden sky", "forest_edge_golden", (0.0, 0.0, 0.3, 0.15), mean)
report("forest_edge_golden grass", "forest_edge_golden", (0.4, 0.7, 1.0, 1.0), mean)
report("overcast_broken sky", "overcast_broken", (0.0, 0.0, 1.0, 0.3), sat)
report("sunrise sky hp", "sunrise_ridge", (0.0, 0.0, 1.0, 0.3), hp)
report("coastline sky hp", "coastline_sunset", (0.0, 0.0, 0.5, 0.3), hp)
report("sunrise foreground", "sunrise_ridge", (0.0, 0.7, 1.0, 1.0), rb)
report("dawn_valley under horizon", "dawn_valley", (0.6, 0.25, 0.75, 0.3), mean)
report("dawn_valley sky over it", "dawn_valley", (0.6, 0.15, 0.75, 0.2), mean)
report("mist_hud right hill", "optics_mist_hud", (0.85, 0.5, 1.0, 0.65), mean)
report("mist_hud sky band", "optics_mist_hud", (0.0, 0.1, 0.2, 0.3), mean)
report("town_fog_night houses std", "town_fog_night", (0.0, 0.3, 0.25, 0.5), lambda b: f"std {luma(b).std():.1f} L{luma(b).mean():.0f}")
report("tank interior", "tank_gunner_interior", (0, 0, 1, 1), mean)
report("whole frame mean (blue hour)", "blue_hour_coast", (0, 0, 1, 1), mean)
