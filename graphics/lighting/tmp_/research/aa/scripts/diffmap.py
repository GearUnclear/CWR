"""Amplified difference map between two captures, plus summary numbers.

    python diffmap.py a.png b.png out.png [--gain 4] [--title "..."]

Writes a, b and |b-a|*gain side by side (full frame). Prints the mean absolute
difference, the fraction of pixels that changed by more than 8/255, and the
mean signed luminance change (b - a), so "only darker" results show up.
"""
import argparse

import numpy as np
from PIL import Image, ImageDraw, ImageFont


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("a")
    ap.add_argument("b")
    ap.add_argument("out")
    ap.add_argument("--gain", type=float, default=4.0)
    ap.add_argument("--title", default="")
    ap.add_argument("--labels", nargs=2, default=["A", "B"])
    x = ap.parse_args()
    A = np.asarray(Image.open(x.a).convert("RGB")).astype(np.float32)
    B = np.asarray(Image.open(x.b).convert("RGB")).astype(np.float32)
    d = np.abs(B - A).mean(axis=2)
    lum = lambda I: I @ np.array([0.2126, 0.7152, 0.0722], dtype=np.float32)
    changed = float((d > 8).mean())
    signed = float((lum(B) - lum(A)).mean())
    print(f"mean |diff| {d.mean():.3f}/255, changed>8: {100 * changed:.2f}% of pixels, mean luminance change {signed:+.3f}/255")
    h, w = d.shape
    gap, head = 8, 40
    out = Image.new("RGB", (3 * w + 2 * gap, h + head), (24, 24, 24))
    out.paste(Image.fromarray(A.astype(np.uint8)), (0, head))
    out.paste(Image.fromarray(B.astype(np.uint8)), (w + gap, head))
    heat = np.clip(d * x.gain, 0, 255).astype(np.uint8)
    out.paste(Image.fromarray(heat).convert("RGB"), (2 * (w + gap), head))
    dr = ImageDraw.Draw(out)
    try:
        f = ImageFont.truetype("arialbd.ttf", 18)
    except OSError:
        f = ImageFont.load_default()
    dr.text((8, 10), f"{x.title}   {x.labels[0]} | {x.labels[1]} | |diff| x{x.gain:g}   "
                     f"(changed>8/255: {100 * changed:.1f}% px, mean luma change {signed:+.2f}/255)",
            fill=(235, 235, 235), font=f)
    out.save(x.out)


if __name__ == "__main__":
    main()
