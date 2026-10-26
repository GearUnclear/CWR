"""B9 lane: motion metrics of b9walk and b9lapse, off pass against on pass.

    python motion_metrics.py <run folder>

The canopy region is where the first frames of the two passes differ by more
than 6 levels of luma (the far shadows the feature adds), dilated a little.

  b9walk   a walk at 0.5 m per frame: the mean absolute second temporal
           difference of luma (a pattern fixed to the screen while the image
           moves shows as frame-to-frame jerks), over the ground band (rows
           591-1080) and over the canopy region, and its share above 0.05.
  b9lapse  a still camera, the sun moving about 0.2 deg a frame (the map is
           rebuilt every frame): the per-frame mean absolute luma change in
           the canopy region; steps or pops show as single frames far above
           the median (max / median).
"""
import glob
import os
import re
import sys

import numpy as np
from PIL import Image


def frames(run, clip, pas):
    out = {}
    for p in glob.glob(os.path.join(run, "tri", "**", f"*_{clip}__{pas}__f*.png"), recursive=True):
        out[int(re.search(r"__f(\d+)\.png$", p).group(1))] = p
    return [out[k] for k in sorted(out)]


def luma(p):
    a = np.asarray(Image.open(p).convert("RGB")).astype(np.float64) / 255.0
    return 0.2126 * a[..., 0] + 0.7152 * a[..., 1] + 0.0722 * a[..., 2]


def region(off0, on0):
    m = np.abs(on0 - off0) > 6.0 / 255.0
    # dilate by 2 px so the region's own edges are inside it
    d = m.copy()
    for dy in (-2, -1, 1, 2):
        d |= np.roll(m, dy, axis=0)
    m = d.copy()
    for dx in (-2, -1, 1, 2):
        d |= np.roll(m, dx, axis=1)
    return d


def main():
    run = sys.argv[1]
    lines = []
    for clip in ("b9walk", "b9lapse"):
        st = {}
        for pas in ("off", "on"):
            fs = [luma(p) for p in frames(run, clip, pas)]
            st[pas] = np.stack(fs) if fs else None
        if st["off"] is None or st["on"] is None or len(st["off"]) < 3:
            lines.append(f"{clip}: frames missing")
            continue
        reg = region(st["off"][0], st["on"][0])
        lines.append(f"{clip}: {len(st['on'])} frames; canopy region {reg.mean() * 100:.2f}% of the frame")
        for pas in ("off", "on"):
            s = st[pas]
            if clip == "b9walk":
                d2 = np.abs(s[2:] - 2 * s[1:-1] + s[:-2])
                g = d2[:, 591:1080]
                r = d2[:, reg]
                lines.append(f"  {pas}: ground d2 {g.mean():.5f} (>0.05: {(g.mean(axis=0) > 0.05).mean() * 100:.2f}%); "
                             f"canopy region d2 {r.mean():.5f} (>0.05: {(r.mean(axis=0) > 0.05).mean() * 100:.2f}%)")
            else:
                d1 = np.abs(np.diff(s, axis=0))[:, reg].mean(axis=1)
                lines.append(f"  {pas}: canopy region d1 per frame median {np.median(d1):.5f} max {d1.max():.5f} "
                             f"(max/median {d1.max() / max(np.median(d1), 1e-9):.2f}); "
                             f"series {' '.join(f'{v * 1000:.2f}' for v in d1)} (x1e-3)")
    text = "\n".join(lines)
    print(text)
    with open(os.path.join(run, "motion_metrics.txt"), "w", encoding="utf-8") as f:
        f.write(text + "\n")


main()
