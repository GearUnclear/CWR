"""Region measurements for the shade track.

For each view, named boxes (x0, y0, x1, y1 in the 960x640 capture). For every
capture label of that view (classic, remastered, variants) and each box:

  F    on-screen fraction of the texture brightness: luma(shot) / luma(albedo),
       both post-gamma display values (the README calibration metric)
  L    display-space light colour on that surface, per channel:
       (mean(shot_c) / mean(albedo_c)) ** 1.2  (undoes the 1.2 display gamma;
       comparable with Classic's per-vertex light, e.g. ambient*1.6)
  CCT  correlated colour temperature of L taken to linear (McCamy), the
       effective colour of the light on that surface (lower = warmer)
  rb   L.r / L.b (warmth of the light, >1 warm)

The albedo shot is the remastered debug 3 override (shader returns the display
albedo, post off), shot from the same camera.

    python measure.py <run_dir> [view ...]   -> prints tables, writes measure_<view>.json
    python measure.py <run_dir> --boxes <view>  -> writes boxes_<view>.png
"""
import json
import math
import sys
from pathlib import Path

from PIL import Image, ImageDraw

BOXES = {
    "lowsun_fp": {
        "jeep_side_front": (45, 352, 100, 372),
        "jeep_side_rear": (150, 360, 195, 377),
        "m113_side": (690, 287, 790, 315),
        "m113_rear45": (607, 280, 640, 330),
        "tent": (300, 280, 490, 340),
        "ground_lit": (625, 400, 825, 430),
        "ground_shadow": (240, 500, 310, 540),
    },
}


def load_boxes():
    p = Path(__file__).with_name("boxes.json")
    if p.exists():
        BOXES.update({k: {n: tuple(b) for n, b in v.items()} for k, v in json.loads(p.read_text()).items()})


def mean_rgb(im, box):
    c = im.crop(box)
    px = list(c.getdata())
    n = len(px)
    return tuple(sum(p[i] for p in px) / n / 255.0 for i in range(3))


def luma(c):
    return 0.2126 * c[0] + 0.7152 * c[1] + 0.0722 * c[2]


def cct(lin):
    r, g, b = lin
    X = 0.4124 * r + 0.3576 * g + 0.1805 * b
    Y = 0.2126 * r + 0.7152 * g + 0.0722 * b
    Z = 0.0193 * r + 0.1192 * g + 0.9505 * b
    s = X + Y + Z
    if s <= 1e-9:
        return float("nan")
    x, y = X / s, Y / s
    n = (x - 0.3320) / (0.1858 - y)
    return 449 * n ** 3 + 3525 * n ** 2 + 6823.3 * n + 5520.33


def labels_for(run, view):
    out = []
    for p in sorted(Path(run).glob(f"{view}__*.png")):
        lab = p.stem.split("__", 1)[1]
        if lab in ("pair",) or lab.startswith("cmp"):
            continue
        out.append(lab)
    return out


def find_shot(run, view, label):
    p = Path(run) / f"{view}__{label}.png"
    if p.exists():
        return p
    hits = sorted(Path(run).rglob(f"*_{view}__{label}.png"), key=lambda q: q.stat().st_mtime)
    return hits[-1] if hits else None


def collect(run):
    """Copy NNN_<view>__<label>.png out of the tri tree (newest wins)."""
    import re
    import shutil
    rx = re.compile(r"^\d{3}_(.+?)__(.+)\.png$")
    best = {}
    for p in Path(run, "tri").rglob("*.png"):
        m = rx.match(p.name)
        if not m:
            continue
        key = (m.group(1), m.group(2))
        if key not in best or p.stat().st_mtime > best[key].stat().st_mtime:
            best[key] = p
    for (v, lab), src in best.items():
        shutil.copy2(src, Path(run) / f"{v}__{lab}.png")
    return best


def measure(run, view, boxes=None, base_labels=None):
    boxes = boxes or BOXES[view]
    alb_p = find_shot(run, view, "albedo")
    alb = Image.open(alb_p).convert("RGB")
    res = {}
    for lab in labels_for(run, view):
        if lab == "albedo":
            continue
        im = Image.open(find_shot(run, view, lab)).convert("RGB")
        row = {}
        for name, box in boxes.items():
            a = mean_rgb(alb, box)
            s = mean_rgb(im, box)
            F = luma(s) / max(luma(a), 1e-6)
            L = tuple((s[i] / max(a[i], 1e-6)) ** 1.2 for i in range(3))
            Llin = tuple(max(v, 0) ** 2.2 for v in L)
            row[name] = {"shot": s, "albedo": a, "F": F, "L": L, "CCT": cct(Llin), "rb": L[0] / max(L[2], 1e-6)}
        res[lab] = row
    return res


def print_table(view, res, labels=None):
    labels = labels or list(res)
    names = list(next(iter(res.values())).keys())
    print(f"\n## {view}  (F = on-screen fraction of albedo; CCT of the light on the surface)")
    print("| label | " + " | ".join(names) + " |")
    print("|---|" + "---|" * len(names))
    for lab in labels:
        if lab not in res:
            continue
        cells = []
        for n in names:
            m = res[lab][n]
            cells.append(f"{m['F']:.2f} {m['CCT']:.0f}K")
        print(f"| {lab} | " + " | ".join(cells) + " |")


def draw_boxes(run, view, boxes=None):
    boxes = boxes or BOXES[view]
    p = find_shot(run, view, "remastered") or find_shot(run, view, "classic")
    im = Image.open(p).convert("RGB")
    d = ImageDraw.Draw(im)
    for n, b in boxes.items():
        d.rectangle(b, outline=(255, 0, 255), width=2)
        d.text((b[0] + 2, b[1] - 11), n, fill=(255, 255, 0))
    out = Path(run) / f"boxes_{view}.png"
    im.save(out)
    return out


if __name__ == "__main__":
    load_boxes()
    run = sys.argv[1]
    if len(sys.argv) > 2 and sys.argv[2] == "--collect":
        print(len(collect(run)), "shots collected")
        sys.exit(0)
    if len(sys.argv) > 2 and sys.argv[2] == "--boxes":
        for v in sys.argv[3:]:
            print(draw_boxes(run, v))
        sys.exit(0)
    views = sys.argv[2:] or [v for v in BOXES]
    for v in views:
        if v not in BOXES or not find_shot(run, v, "albedo"):
            continue
        r = measure(run, v)
        print_table(v, r)
        Path(run, f"measure_{v}.json").write_text(json.dumps(r, indent=1))
