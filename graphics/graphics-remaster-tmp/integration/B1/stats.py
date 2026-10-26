"""Frame statistics for the look check: mean display value of the mean linear
luminance (the research brightness measure), mean colour, R/B, 5th/95th
percentile of luma, and the same for an optional crop.

    python stats.py [--crop x0,y0,x1,y1] <label=path> ...
"""
import sys

import numpy as np
from PIL import Image


def stats(path, crop=None):
    im = Image.open(path).convert("RGB")
    if crop:
        im = im.crop(crop)
    a = np.asarray(im).astype(np.float64) / 255.0
    lin = a ** 2.2
    lum = 0.2126 * lin[..., 0] + 0.7152 * lin[..., 1] + 0.0722 * lin[..., 2]
    disp = lum.mean() ** (1 / 2.2)
    luma = 0.2126 * a[..., 0] + 0.7152 * a[..., 1] + 0.0722 * a[..., 2]
    mean = a.reshape(-1, 3).mean(axis=0) * 255
    return disp, mean, mean[0] / max(mean[2], 1e-6), np.percentile(luma, 5) * 255, np.percentile(luma, 95) * 255


def main():
    args = sys.argv[1:]
    crop = None
    if args and args[0] == "--crop":
        crop = tuple(int(v) for v in args[1].split(","))
        args = args[2:]
    print(f"{'label':28s} {'bright':>7s} {'R':>6s} {'G':>6s} {'B':>6s} {'R/B':>5s} {'p5':>6s} {'p95':>6s}")
    base = None
    for item in args:
        label, path = item.split("=", 1)
        d, m, rb, p5, p95 = stats(path, crop)
        base = base if base is not None else d
        print(f"{label:28s} {d:7.4f} {m[0]:6.1f} {m[1]:6.1f} {m[2]:6.1f} {rb:5.2f} {p5:6.1f} {p95:6.1f}  "
              f"({(d / base - 1) * 100:+.1f}%)")


main()
