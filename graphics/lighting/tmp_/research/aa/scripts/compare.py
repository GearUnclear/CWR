"""Labelled side-by-side comparison panels.

    python compare.py <out.png> --title "..." [--crop x0,y0,x1,y1] [--zoom N]
                      [--cols N] [--timings run/timings.csv ...]
                      <image>=<label> ...

Each panel gets its label top-left and, when the image's shot label is in one
of the timing CSVs, "GPU frame X ms | composite Y ms" bottom-left.
"""
import argparse
import csv
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


def font(size):
    for name in ("arialbd.ttf", "arial.ttf", "DejaVuSans-Bold.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            pass
    return ImageFont.load_default()


def tag(draw, xy, text, f, anchor_bottom=False):
    x, y = xy
    bb = draw.textbbox((0, 0), text, font=f)
    w, h = bb[2] - bb[0], bb[3] - bb[1]
    if anchor_bottom:
        y -= h + 10
    draw.rectangle([x, y, x + w + 10, y + h + 10], fill=(0, 0, 0))
    draw.text((x + 5, y + 5 - bb[1]), text, fill=(255, 255, 255), font=f)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("items", nargs="+")
    ap.add_argument("--title", default="")
    ap.add_argument("--crop", default="")
    ap.add_argument("--zoom", type=int, default=1)
    ap.add_argument("--cols", type=int, default=0)
    ap.add_argument("--timings", action="append", default=[])
    ap.add_argument("--note", default="")
    a = ap.parse_args()

    times = {}
    for t in a.timings:
        with open(t, encoding="utf-8") as f:
            for r in csv.DictReader(f):
                times[r["label"]] = r

    panels = []
    for it in a.items:
        path, label = it.split("=", 1)
        im = Image.open(path).convert("RGB")
        if a.crop:
            x0, y0, x1, y1 = map(int, a.crop.split(","))
            im = im.crop((x0, y0, x1, y1))
        if a.zoom > 1:
            im = im.resize((im.width * a.zoom, im.height * a.zoom), Image.NEAREST)
        shot = Path(path).stem
        t = times.get(shot)
        ms = ""
        if t and t.get("frame"):
            ms = f"GPU frame {t['frame']} ms"
            if t.get("composite"):
                ms += f" | composite {t['composite']} ms"
        panels.append((im, label, ms))

    n = len(panels)
    cols = a.cols or n
    rows = (n + cols - 1) // cols
    pw, ph = panels[0][0].size
    gap, head = 8, 44 if a.title else 0
    foot = 30 if a.note else 0
    W = cols * pw + (cols - 1) * gap
    H = head + rows * ph + (rows - 1) * gap + foot
    out = Image.new("RGB", (W, H), (24, 24, 24))
    d = ImageDraw.Draw(out)
    f_lab, f_ms, f_title = font(20), font(15), font(22)
    if a.title:
        d.text((10, 10), a.title, fill=(235, 235, 235), font=f_title)
    for i, (im, label, ms) in enumerate(panels):
        x = (i % cols) * (pw + gap)
        y = head + (i // cols) * (ph + gap)
        out.paste(im, (x, y))
        tag(d, (x + 6, y + 6), label, f_lab)
        if ms:
            tag(d, (x + 6, y + ph - 4), ms, f_ms, anchor_bottom=True)
    if a.note:
        d.text((10, H - foot + 6), a.note, fill=(200, 200, 200), font=font(15))
    out.save(a.out)
    print(a.out, out.size)


if __name__ == "__main__":
    main()
