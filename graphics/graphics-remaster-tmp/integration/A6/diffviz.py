"""Difference picture of two captures: the largest channel difference per
pixel, times a gain, over a darkened copy of image A; plus counts.

    python diffviz.py <a.png> <b.png> <out.png> [--gain 8] [--crop x0,y0,x1,y1]
"""
import argparse

import numpy as np
from PIL import Image


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("a")
    ap.add_argument("b")
    ap.add_argument("out")
    ap.add_argument("--gain", type=float, default=8.0)
    ap.add_argument("--crop", default="")
    o = ap.parse_args()
    a = np.asarray(Image.open(o.a).convert("RGB")).astype(np.int16)
    b = np.asarray(Image.open(o.b).convert("RGB")).astype(np.int16)
    d = np.abs(a - b).max(axis=2)
    signed = (b.astype(np.float32).mean(axis=2) - a.astype(np.float32).mean(axis=2))
    print(f"max {d.max()} over1 {(d > 1).sum()} over2 {(d > 2).sum()} over8 {(d > 8).sum()} of {d.size}; "
          f"mean luma change where over2: {signed[d > 2].mean() if (d > 2).any() else 0:.2f}")
    base = (a.astype(np.float32) * 0.35).astype(np.float32)
    heat = np.clip(d.astype(np.float32) * o.gain, 0, 255)
    out = base.copy()
    brighter = signed > 0
    out[..., 0] = np.where(brighter, np.maximum(base[..., 0], heat), base[..., 0])
    out[..., 2] = np.where(~brighter, np.maximum(base[..., 2], heat), base[..., 2])
    out[..., 1] = np.maximum(base[..., 1], heat * 0.35)
    img = Image.fromarray(np.clip(out, 0, 255).astype(np.uint8))
    if o.crop:
        x0, y0, x1, y1 = (int(v) for v in o.crop.split(","))
        img = img.crop((x0, y0, x1, y1))
    img.save(o.out)


if __name__ == "__main__":
    main()
