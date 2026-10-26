"""Is the change two-sided? Per view, the display-luminance difference b - a in
8-bit levels over the sky band (rows 0..421) and the whole frame: the share of
pixels darker by more than 2 levels, brighter by more than 2, the mean and the
1st and 99th percentiles.

    python onesided.py <dir> [--a off] [--b on] [--md out.md] [--views v1,v2]
    python onesided.py <dir> --other <dir2> --a on --b on   (same view, two runs)
"""
import argparse
from pathlib import Path

import numpy as np
from PIL import Image

ap = argparse.ArgumentParser()
ap.add_argument("run")
ap.add_argument("--other", default="")
ap.add_argument("--a", default="off")
ap.add_argument("--b", default="on")
ap.add_argument("--md", default="")
ap.add_argument("--views", default="")
ap.add_argument("--title", default="")
a = ap.parse_args()

K = np.array([0.2126, 0.7152, 0.0722])


def lum(path):
    return (np.asarray(Image.open(path).convert("RGB")).astype(np.float64) @ K)


def stats(d):
    n = d.size
    return (f"{(d < -2).sum() / n * 100:.1f}% / {(d > 2).sum() / n * 100:.1f}% / "
            f"{d.mean() / 255:+.4f} / {np.percentile(d, 1) / 255:+.3f} / {np.percentile(d, 99) / 255:+.3f}")


run = Path(a.run)
other = Path(a.other) if a.other else run
wanted = [v for v in a.views.split(",") if v]
rows = []
for pa in sorted(run.glob(f"*__{a.a}.png")):
    view = pa.name[: -len(f"__{a.a}.png")]
    if wanted and view not in wanted:
        continue
    pb = other / f"{view}__{a.b}.png"
    if not pb.exists():
        continue
    la, lb = lum(pa), lum(pb)
    d = lb - la
    rows.append([view, stats(d[0:422]), stats(d)])

head = ["view", "sky: darker >2 / brighter >2 / mean / p1 / p99", "whole frame: same"]
title = a.title or f"{a.b} ({other.name}) minus {a.a} ({run.name}), display luminance"
lines = [f"# {title}", "", "| " + " | ".join(head) + " |", "|---|---|---|"]
lines += ["| " + " | ".join(r) + " |" for r in rows]
text = "\n".join(lines)
print(text)
if a.md:
    Path(a.md).write_text(text + "\n", encoding="utf-8")
