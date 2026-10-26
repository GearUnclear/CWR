"""The sky sweep (b10_sweep): per sky position, the sun_up frame's mean display
luminance (0..255) on the sun disk (radius 16 px around the sun at sky 0) and
on a ring around it (16 to 60 px), and the ground_sweep frame's ground band
(rows 591 on); with --ref, the pixels over 8 levels against the same frame of
another run, split into those within 100 px of the sun and the rest.

    python sweep.py <run dir> [--ref <run dir>] [--md out.md]
"""
import argparse
from pathlib import Path

import numpy as np
from PIL import Image

ap = argparse.ArgumentParser()
ap.add_argument("run")
ap.add_argument("--ref", default="")
ap.add_argument("--md", default="")
a = ap.parse_args()

K = np.array([0.2126, 0.7152, 0.0722])
SUN = (464, 952)  # row, column of the sun disk at sky 0 (brightest 16 px box)


def frames(run, view):
    d = Path(run) / "tri" / "b10_sweep"
    out = {}
    for p in sorted(d.glob(f"*_{view}__sky*.png")):
        out[int(p.stem.split("__sky")[1])] = p
    return out


def rgb(p):
    return np.asarray(Image.open(p).convert("RGB")).astype(np.int16)


yy, xx = np.mgrid[0:1080, 0:1920]
r = np.hypot(yy - SUN[0], xx - SUN[1])
disk, ring, near = r <= 16, (r > 16) & (r <= 60), r <= 100

sun = frames(a.run, "clouds_sun_up")
ground = frames(a.run, "clouds_ground_sweep")
ref_sun = frames(a.ref, "clouds_sun_up") if a.ref else {}
ref_ground = frames(a.ref, "clouds_ground_sweep") if a.ref else {}

head = ["pos", "turn deg", "sun disk", "sky ring", "ground"]
if a.ref:
    head += ["sun_up over 8 vs ref: near sun / rest", "ground_sweep over 8 vs ref"]
lines = ["| " + " | ".join(head) + " |", "|" + "---|" * len(head)]
for pos in sorted(sun):
    s = rgb(sun[pos])
    ls = s.astype(float) @ K
    cells = [f"{pos:03d}", f"{pos * 0.3:.1f}", f"{ls[disk].mean():.1f}", f"{ls[ring].mean():.1f}"]
    if pos in ground:
        g = rgb(ground[pos])
        cells.append(f"{(g.astype(float) @ K)[591:].mean():.1f}")
    else:
        cells.append("")
    if a.ref:
        if pos in ref_sun:
            d = np.abs(s - rgb(ref_sun[pos])).max(axis=2) > 8
            cells.append(f"{int((d & near).sum()):,} / {int((d & ~near).sum()):,}")
        else:
            cells.append("")
        if pos in ground and pos in ref_ground:
            d = np.abs(rgb(ground[pos]) - rgb(ref_ground[pos])).max(axis=2) > 8
            cells.append(f"{int(d.sum()):,}")
        else:
            cells.append("")
    lines.append("| " + " | ".join(cells) + " |")
text = "\n".join(lines)
print(text)
if a.md:
    Path(a.md).write_text(text + "\n", encoding="utf-8")
