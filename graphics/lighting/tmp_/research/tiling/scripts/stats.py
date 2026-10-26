"""Ground statistics for the tiling captures: tint guard and texture energy.

    python stats.py <cmp_dir> <view> <y0> <y1> [<x0> <x1>]

For every mode image of the view, over the ground band rows y0..y1:
  mean RGB, mean luma, mean chroma (max-min of RGB), hue (deg),
  fine energy  = std of (img - 3px blur) luma      (detail / grit)
  macro std    = std of 24px-blurred luma            (large-scale variation)
A tint or wash shows as a hue or chroma shift against v0; "only darker" shows
as a luma drop.
"""
import colorsys
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageFilter


def stats(p, box):
    im = Image.open(p).convert("RGB").crop(box)
    a = np.asarray(im).astype(np.float64) / 255.0
    luma = a @ np.array([0.2126, 0.7152, 0.0722])
    rgb = a.reshape(-1, 3).mean(0)
    h, s, v = colorsys.rgb_to_hsv(*rgb)
    chroma = (a.max(2) - a.min(2)).mean()
    L = Image.fromarray((luma * 255).astype(np.uint8))
    b3 = np.asarray(L.filter(ImageFilter.GaussianBlur(3))).astype(np.float64) / 255
    b24 = np.asarray(L.filter(ImageFilter.GaussianBlur(24))).astype(np.float64) / 255
    fine = (luma - b3).std()
    macro = b24[24:-24, 24:-24].std() if b24.shape[0] > 60 and b24.shape[1] > 60 else b24.std()
    return rgb, luma.mean(), chroma, h * 360, fine, macro


def main():
    d = Path(sys.argv[1])
    view = sys.argv[2]
    y0, y1 = int(sys.argv[3]), int(sys.argv[4])
    x0, x1 = (int(sys.argv[5]), int(sys.argv[6])) if len(sys.argv) > 6 else (0, 960)
    box = (x0, y0, x1, y1)
    print(f"{view} box={box}")
    print(f"{'mode':8s} {'R':>6s} {'G':>6s} {'B':>6s} {'luma':>6s} {'chroma':>6s} {'hue':>6s} {'fine':>7s} {'macro':>7s}")
    for p in sorted(d.glob(f"{view}__*.png")):
        mode = p.stem.split("__")[-1]
        if not (mode == "classic" or (mode.startswith("v") and mode[1:].isdigit())):
            continue
        rgb, l, c, h, f, m = stats(p, box)
        print(f"{mode:8s} {rgb[0]:6.3f} {rgb[1]:6.3f} {rgb[2]:6.3f} {l:6.3f} {c:6.3f} {h:6.1f} {f:7.4f} {m:7.4f}")


if __name__ == "__main__":
    main()
