"""B1 lane: motion metrics of b1still and b1walk, off pass against on pass.

    python motion_metrics.py <run folder>

  b1still  a still camera: the mean absolute luma change between consecutive
           frames (the soft filter's per-frame turn left over after the TAA
           history), over the frame, the ground (rows 600-1080) and the crowns
           (rows 0-420).
  b1walk   a walk at 0.08 m per frame: the mean absolute second temporal
           difference of luma (the research's crawl measure: a pattern fixed
           to the screen while the image moves shows as frame-to-frame
           jerks), same regions, and the share of pixels whose mean second
           difference is above 0.05.
"""
import glob
import os
import re
import sys

import numpy as np
from PIL import Image

REGIONS = {"frame": (0, 1080), "crowns": (0, 420), "ground": (600, 1080)}


def frames(run, clip, pas):
    out = {}
    for p in glob.glob(os.path.join(run, "tri", "**", f"*_{clip}__{pas}__f*.png"), recursive=True):
        out[int(re.search(r"__f(\d+)\.png$", p).group(1))] = p
    return [out[k] for k in sorted(out)]


def luma(p):
    a = np.asarray(Image.open(p).convert("RGB")).astype(np.float64) / 255.0
    return 0.2126 * a[..., 0] + 0.7152 * a[..., 1] + 0.0722 * a[..., 2]


def main():
    run = sys.argv[1]
    lines = []
    for clip in ("b1still", "b1walk"):
        for pas in ("off", "on"):
            fs = [luma(p) for p in frames(run, clip, pas)]
            if len(fs) < 3:
                lines.append(f"{clip} {pas}: {len(fs)} frames, skipped")
                continue
            st = np.stack(fs)
            d1 = np.abs(np.diff(st, axis=0))
            d2 = np.abs(st[2:] - 2 * st[1:-1] + st[:-2])
            parts = []
            for name, (r0, r1) in REGIONS.items():
                if clip == "b1still":
                    parts.append(f"{name} d1 {d1[:, r0:r1].mean():.5f}")
                else:
                    m2 = d2[:, r0:r1].mean(axis=0)
                    parts.append(f"{name} d2 {d2[:, r0:r1].mean():.5f} (>0.05: {(m2 > 0.05).mean() * 100:.1f}%)")
            lines.append(f"{clip} {pas} ({len(fs)} frames): " + "; ".join(parts))
    text = "\n".join(lines)
    print(text)
    with open(os.path.join(run, "motion_metrics.txt"), "w", encoding="utf-8") as f:
        f.write(text + "\n")


main()
