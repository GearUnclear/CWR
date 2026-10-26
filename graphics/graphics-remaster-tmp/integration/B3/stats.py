"""Mean luma, the 5th / 50th / 95th percentiles and the share of near-black
pixels (luma under 8) of __off and __on shots of a run, per view.

    python stats.py <run dir>
"""
import sys
from pathlib import Path

import numpy as np
from PIL import Image

run = Path(sys.argv[1])
print("| view | mode | mean | p5 | p50 | p95 | under 8 |")
print("|---|---|---|---|---|---|---|")
for off in sorted(run.glob("*__off.png")):
    view = off.name[: -len("__off.png")]
    for mode in ("off", "on"):
        im = np.asarray(Image.open(run / f"{view}__{mode}.png").convert("RGB")).astype(float)
        y = im @ np.array([0.2126, 0.7152, 0.0722])
        p5, p50, p95 = np.percentile(y, [5, 50, 95])
        print(f"| {view} | {mode} | {y.mean():.1f} | {p5:.1f} | {p50:.1f} | {p95:.1f} | {(y < 8).mean() * 100:.1f}% |")
