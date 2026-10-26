"""Mean colour statistics of rectangular regions in captures.

    python regstats.py x0 y0 x1 y1 img1.png [img2.png ...]

Prints mean display RGB, Rec.709 luma, HSV saturation, warmth (R/B) and green
dominance (G / mean(R, B)) per image, so crown colour can be compared across
Classic, Remastered and variants.
"""
import sys

from PIL import Image


def stats(path, box):
    im = Image.open(path).convert("RGB").crop(box)
    px = list(im.getdata())
    n = len(px)
    r = sum(p[0] for p in px) / n
    g = sum(p[1] for p in px) / n
    b = sum(p[2] for p in px) / n
    sat = 0.0
    for p in px:
        mx, mn = max(p), min(p)
        sat += (mx - mn) / mx if mx > 0 else 0.0
    sat /= n
    luma = 0.2126 * r + 0.7152 * g + 0.0722 * b
    return r, g, b, luma, sat


def main():
    box = tuple(int(v) for v in sys.argv[1:5])
    for p in sys.argv[5:]:
        r, g, b, luma, sat = stats(p, box)
        name = p.replace("\\", "/").rsplit("/", 1)[-1]
        print(f"{name:48s} rgb=({r:5.1f},{g:5.1f},{b:5.1f}) luma={luma:5.1f} sat={sat:.3f} "
              f"R/B={r / max(b, 1):.2f} G/RB={g / max((r + b) / 2, 1):.2f}")


if __name__ == "__main__":
    main()
