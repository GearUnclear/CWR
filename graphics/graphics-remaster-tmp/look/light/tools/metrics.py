"""Per-view numbers for the critics' points.  python metrics.py <run dir> mode[,mode...]"""
import sys
from pathlib import Path

import numpy as np
from PIL import Image

run = Path(sys.argv[1])
modes = sys.argv[2].split(",")


def load(v, m):
    p = run / f"{v}__{m}.png"
    if not p.exists():
        return None
    return np.asarray(Image.open(p).convert("RGB")).astype(np.float32)


def L(a):
    return 0.2126 * a[..., 0] + 0.7152 * a[..., 1] + 0.0722 * a[..., 2]


def box(a, x0, y0, x1, y1):
    return a[y0:y1, x0:x1]


def mean(a):
    return tuple(int(round(x)) for x in a.reshape(-1, 3).mean(0))


def fmt(t):
    return "(" + ",".join(str(x) for x in t) + ")"


def white_blob(a, cx, cy, r=250):
    sub = a[max(cy - r, 0):cy + r, max(cx - r, 0):cx + r]
    w = (sub.min(-1) >= 250)
    ys, xs = np.where(w)
    if len(xs) == 0:
        return "white 0"
    return f"white {len(xs)} px {xs.max() - xs.min() + 1}x{ys.max() - ys.min() + 1}"


for m in modes:
    print(f"==== {m}")
    a = load("noon_fields", m)
    if a is not None:
        print(f"noon_fields  p99 {np.percentile(L(a), 99):.0f} p50 {np.percentile(L(a), 50):.0f} "
              f"zenith {fmt(mean(box(a, 0, 0, 1920, 100)))} horizon {fmt(mean(box(a, 0, 290, 560, 335)))} "
              f"ground p99 {np.percentile(L(box(a, 0, 600, 1920, 1080)), 99):.0f} "
              f"jeep p10 {np.percentile(L(box(a, 1065, 585, 1335, 750)), 10):.0f}")
    a = load("sinai_desert_noon", m)
    if a is not None:
        print(f"sinai        p99 {np.percentile(L(a), 99):.0f} p50 {np.percentile(L(a), 50):.0f} "
              f"zenith {fmt(mean(box(a, 0, 0, 1920, 100)))} horizon {fmt(mean(box(a, 0, 340, 1900, 385)))} "
              f"sand p99 {np.percentile(L(box(a, 0, 600, 1920, 1080)), 99):.0f} mean {fmt(mean(box(a, 0, 600, 1920, 1080)))} "
              f"hummer p10 {np.percentile(L(box(a, 1095, 540, 1455, 705)), 10):.0f}")
    a = load("sunrise_ridge", m)
    if a is not None:
        sky = box(a, 0, 0, 1920, 470)
        print(f"sunrise      near-sun rows {fmt(mean(box(a, 600, 380, 1500, 470)))} sky L {np.percentile(L(sky), 5):.0f}..{np.percentile(L(sky), 95):.0f} "
              f"clip% {(a.max(-1) >= 254).mean() * 100:.2f} {white_blob(a, 1050, 435)} "
              f"ground {fmt(mean(box(a, 0, 600, 1920, 1080)))}")
    a = load("coastline_sunset", m)
    if a is not None:
        band = box(a, 0, 330, 1000, 470)
        olive = ((band[..., 1] > band[..., 0]) & (band[..., 0] > band[..., 2]) & (band[..., 1] - band[..., 2] > 8)).mean() * 100
        print(f"coastline    {white_blob(a, 1236, 298)} band {fmt(mean(band))} olive% {olive:.1f} "
              f"sea {fmt(mean(box(a, 0, 600, 1920, 800)))} sky top {fmt(mean(box(a, 0, 0, 1920, 120)))}")
    a = load("lowsun_fp", m)
    if a is not None:
        print(f"lowsun_fp    {white_blob(a, 225, 186)} sun peak {a[150:220, 190:260].max():.0f} "
              f"sky by sun {fmt(mean(box(a, 300, 100, 600, 250)))} sky far {fmt(mean(box(a, 1300, 50, 1900, 250)))}")
    a = load("town_night", m)
    if a is not None:
        fire = box(a, 1155, 560, 1455, 790)
        f = fire.reshape(-1, 3)
        top = f[np.argsort(L(f))[-len(f) // 20:]].mean(0)
        warm = ((f[:, 0] - f[:, 2]) > 60).sum()
        smoke = box(a, 1150, 200, 1450, 550)
        print(f"town_night   fire top5% {fmt(tuple(int(x) for x in top))} r-b>60 {warm} smoke {fmt(mean(smoke))} "
              f"ground {fmt(mean(box(a, 0, 800, 900, 1080)))} frame {np.mean(L(a)):.0f}")
    a = load("woods_pine2_evening", m)
    if a is not None:
        can = box(a, 1000, 60, 1900, 560).reshape(-1, 3)
        can = can[L(can) < 200]
        c = can.mean(0)
        sky = box(a, 1780, 0, 1920, 80)
        print(f"woods        canopy {fmt(tuple(int(x) for x in c))} r/g {c[0] / c[1]:.2f} g/b {c[1] / c[2]:.2f} "
              f"sky gap clip% {(sky.min(-1) >= 250).mean() * 100:.1f} sky gap {fmt(mean(sky))}")
    a = load("blue_hour_coast", m)
    if a is not None:
        print(f"blue_hour    zenith {fmt(mean(box(a, 0, 0, 1920, 150)))} horizon band {fmt(mean(box(a, 0, 500, 1920, 555)))} "
              f"beach luma {np.mean(L(box(a, 0, 950, 1920, 1080))):.0f} sea {fmt(mean(box(a, 0, 600, 1920, 900)))}")
    a = load("overcast_broken", m)
    c = load("overcast_broken", "classic")
    if a is not None and c is not None:
        sa = Image.fromarray(a.astype(np.uint8)).resize((480, 270), Image.LANCZOS)
        sc = Image.fromarray(c.astype(np.uint8)).resize((480, 270), Image.LANCZOS)
        mad = np.abs(np.asarray(sa).astype(float) - np.asarray(sc).astype(float)).mean()
        sky = box(a, 0, 0, 1920, 350)
        sat = ((sky.max(-1) - sky.min(-1)) / np.maximum(sky.max(-1), 1)).mean()
        print(f"overcast     MAD@480 {mad:.1f} sky sat {sat:.2f} sky L p5..p95 {np.percentile(L(sky), 5):.0f}..{np.percentile(L(sky), 95):.0f} "
              f"ground L p5..p95 {np.percentile(L(box(a, 0, 650, 1920, 1080)), 5):.0f}..{np.percentile(L(box(a, 0, 650, 1920, 1080)), 95):.0f}")
    # MAD at thumbnail for every view against classic
    out = []
    for v in ["lowsun_fp", "noon_fields", "forest_edge_golden", "sunrise_ridge", "coastline_sunset", "overcast_broken",
              "town_night", "sinai_desert_noon", "woods_pine2_evening", "blue_hour_coast"]:
        a, c = load(v, m), load(v, "classic")
        if a is None or c is None:
            continue
        sa = np.asarray(Image.fromarray(a.astype(np.uint8)).resize((480, 270), Image.LANCZOS)).astype(float)
        sc = np.asarray(Image.fromarray(c.astype(np.uint8)).resize((480, 270), Image.LANCZOS)).astype(float)
        out.append(f"{v[:10]} {np.abs(sa - sc).mean():.0f}")
    print("MAD@480: " + ", ".join(out))
