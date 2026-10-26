"""BEST_<view>.png (v7-v9 resume): Classic | Remastered | Remastered + SSGI |
|change| x<amp> (and optionally the added-light debug shot as a fifth panel).

python make_best_v2.py <out png> <run dir> <view> <variant> <caption> [--base LABEL] [--amp N] [--dbg LABEL]

--base: the gi=0 shot the change map is taken against (default "remastered");
use a later gi=0 shot when something time-dependent (far street lamps) changed
between the first Remastered shot and the variant.
"""
import argparse
import sys
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
from compare import font, hstack, panel, parse_gpu  # noqa: E402
from make_best import find  # noqa: E402


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("run")
    ap.add_argument("view")
    ap.add_argument("variant")
    ap.add_argument("caption")
    ap.add_argument("--base", default="remastered")
    ap.add_argument("--amp", type=float, default=8.0)
    ap.add_argument("--dbg", default="")
    a = ap.parse_args()
    run = Path(a.run)
    gpu = {}
    for log in run.glob("tri/**/game_stdout.log"):
        gpu.update(parse_gpu(log))
    ld = lambda k: Image.open(find(run, a.view, k)).convert("RGB")
    cl, rem, var, base = ld("classic"), ld("remastered"), ld(a.variant), ld(a.base)
    x = np.asarray(base).astype(float)
    y = np.asarray(var).astype(float)
    d = np.abs(y - x).mean(axis=2)
    heat = np.clip(np.stack([d, d * 0.6, d * 0.25], axis=2) * a.amp, 0, 255).astype(np.uint8)
    stats = f"mean {d.mean():.2f}, p99 {np.percentile(d, 99):.1f} levels vs {a.base}"
    f1, f2 = font(18), font(16)
    ms = lambda k: gpu.get(f"{a.view}__{k}", {}).get("frame")
    ps = [
        panel(cl, "Classic", ms("classic"), f1, f2),
        panel(rem, "Remastered (current)", ms("remastered"), f1, f2),
        panel(var, a.caption, ms(a.variant), f1, f2),
        panel(Image.fromarray(heat), f"|change| x{a.amp:g} ({stats})", None, f1, f2),
    ]
    if a.dbg:
        ps.append(panel(ld(a.dbg), "added light only (x4)", ms(a.dbg), f1, f2))
    hstack(ps).save(a.out)
    print("wrote", a.out, stats)


if __name__ == "__main__":
    main()
