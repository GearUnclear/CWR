"""Cloud-shadow share of each view at 'clear' weather (run 12): compare the sun
visibility debug view with cloud shadows on (dbg1) and off (dbg1cs0). A pixel is
under a cloud shadow when switching them off raises its visibility by > 0.25
(display value). Also the mean frame luma of the remastered shot with cloud
shadows on vs off (cs0).

    python cloudfrac.py <run_dir> view [view ...]
"""
import sys
from pathlib import Path

from PIL import Image


def luma_px(im):
    r, g, b = im.split()
    return [0.2126 * a + 0.7152 * c + 0.0722 * d for a, c, d in zip(r.getdata(), g.getdata(), b.getdata())]


def main():
    run = Path(sys.argv[1])
    print("| view | cloud-shadowed share of the frame | mean luma, cloud shadows on | off | on/off |")
    print("|---|---|---|---|---|")
    for v in sys.argv[2:]:
        try:
            on = luma_px(Image.open(run / f"{v}__dbg1.png").convert("RGB"))
            off = luma_px(Image.open(run / f"{v}__dbg1cs0.png").convert("RGB"))
            ron = luma_px(Image.open(run / f"{v}__remastered.png").convert("RGB"))
            roff = luma_px(Image.open(run / f"{v}__cs0.png").convert("RGB"))
        except FileNotFoundError as e:
            print(f"| {v} | missing {e.filename} | | | |")
            continue
        n = len(on)
        shadowed = sum(1 for a, b in zip(on, off) if b - a > 0.25 * 255) / n
        mon, moff = sum(ron) / n / 255, sum(roff) / n / 255
        print(f"| {v} | {shadowed * 100:.1f}% | {mon:.3f} | {moff:.3f} | {mon / max(moff, 1e-6):.3f} |")


if __name__ == "__main__":
    main()
