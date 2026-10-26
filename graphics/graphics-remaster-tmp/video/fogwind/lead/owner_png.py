"""owner_compare.png: the x-t RESID panels of xtslice.py's xt.png (clips stacked in xt.png) laid side by side,
labelled, with a one-line legend.

python owner_png.py <xt.png> <out.png> LABEL1 LABEL2 LABEL3 [--x0 0 --x1 1]
"""
import argparse
from PIL import Image, ImageDraw, ImageFont


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("xt")
    ap.add_argument("out")
    ap.add_argument("labels", nargs="+")
    ap.add_argument("--x0", type=float, default=0.0, help="share of the RESID panel's width to start at")
    ap.add_argument("--x1", type=float, default=1.0)
    ap.add_argument("--scale", type=float, default=1.0)
    ap.add_argument("--yscale", type=float, default=1.0)
    o = ap.parse_args()
    im = Image.open(o.xt).convert("RGB")
    W, H = im.size
    n = len(o.labels)
    sep = 4  # xtslice draws a 4 px black line between clips
    ph = (H - sep * (n - 1)) // n
    rx0 = 2 * W // 3 + 4
    pw = W - rx0
    a, b = int(rx0 + o.x0 * pw), int(rx0 + o.x1 * pw)
    panels = []
    for k in range(n):
        y = k * (ph + sep)
        p = im.crop((a, y + 16, b, y + ph))  # skip xtslice's own small label row
        import numpy as np
        arr = np.asarray(p).astype(float).mean(axis=(1, 2))
        keep = len(arr)
        while keep > 10 and arr[keep - 1] < 8:
            keep -= 1
        p = p.crop((0, 0, p.size[0], keep))
        if o.scale != 1.0 or o.yscale != 1.0:
            p = p.resize((int(p.size[0] * o.scale), int(p.size[1] * o.scale * o.yscale)), Image.NEAREST)
        panels.append(p)
    try:
        font = ImageFont.truetype("C:/Windows/Fonts/arial.ttf", 30)
        small = ImageFont.truetype("C:/Windows/Fonts/arial.ttf", 20)
    except OSError:
        font = small = ImageFont.load_default()
    gap, top, bottom = 24, 50, 64
    hmin = min(q.size[1] for q in panels)
    panels = [q.crop((0, 0, q.size[0], hmin)) for q in panels]
    pw2, ph2 = panels[0].size
    out = Image.new("RGB", (n * pw2 + (n + 1) * gap, top + ph2 + bottom), (24, 24, 24))
    d = ImageDraw.Draw(out)
    for k, (p, lab) in enumerate(zip(panels, o.labels)):
        x = gap + k * (pw2 + gap)
        out.paste(p, (x, top))
        d.text((x, 10), lab, fill=(255, 255, 255), font=font)
    d.text((gap, top + ph2 + 10),
           "One line of the far valley and bay per frame, time running down, the camera's pan taken out, changes x16.",
           fill=(220, 220, 220), font=small)
    d.text((gap, top + ph2 + 34),
           "Flat grey = calm. Horizontal lines = the haze pumping. Slanted streaks = a pattern travelling (the wind).",
           fill=(220, 220, 220), font=small)
    out.save(o.out)
    print(out.size)


if __name__ == "__main__":
    main()
