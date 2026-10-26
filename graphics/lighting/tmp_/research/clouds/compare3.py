"""Labelled N-way comparison strips for the clouds research track.

usage: python compare3.py OUT.png TITLE  RUN_DIR:VIEW:MODE:LABEL  [RUN_DIR:VIEW:MODE:LABEL ...]
Each panel is <RUN_DIR>/<VIEW>__<MODE>.png with its GPU frame time from
<RUN_DIR>/timings.csv (the row for that view and mode). Optional --crop
x0,y0,x1,y1 crops every panel (pixels of the 960x640 capture) and --scale s.
"""
import csv
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


def font(size):
    for name in ("arialbd.ttf", "arial.ttf", "DejaVuSans-Bold.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            pass
    return ImageFont.load_default()


def gpu_ms(run_dir: Path, view: str, mode: str):
    f = run_dir / "timings.csv"
    if not f.exists():
        return None
    with f.open(newline="") as fh:
        for row in csv.DictReader(fh):
            if row["view"] == view and row["mode"] == mode:
                try:
                    return float(row["gpu_frame_ms"])
                except ValueError:
                    return None
    return None


def box(d, xy, text, fnt):
    x, y = xy
    l, t, r, b = d.textbbox((x, y), text, font=fnt)
    d.rectangle((l - 5, t - 3, r + 5, b + 3), fill=(0, 0, 0))
    d.text((x, y), text, font=fnt, fill=(255, 255, 255))


def main(argv):
    crop = None
    scale = 1.0
    args = []
    i = 0
    while i < len(argv):
        if argv[i] == "--crop":
            crop = tuple(int(v) for v in argv[i + 1].split(","))
            i += 2
        elif argv[i] == "--scale":
            scale = float(argv[i + 1])
            i += 2
        else:
            args.append(argv[i])
            i += 1
    out, title, specs = Path(args[0]), args[1], args[2:]
    panels = []
    for spec in specs:
        run, view, mode, label = spec.rsplit(":", 3)
        run = Path(run)
        img = Image.open(run / f"{view}__{mode}.png").convert("RGB")
        if crop:
            img = img.crop(crop)
        if scale != 1.0:
            img = img.resize((int(img.width * scale), int(img.height * scale)), Image.LANCZOS)
        panels.append((img, label, gpu_ms(run, view, mode)))
    w, h = panels[0][0].size
    gap, title_h = 8, 40
    sheet = Image.new("RGB", (w * len(panels) + gap * (len(panels) - 1), h + title_h), (24, 24, 24))
    d = ImageDraw.Draw(sheet)
    d.text((10, 8), title, font=font(22), fill=(235, 235, 235))
    big, small = font(20), font(14)
    for k, (img, label, ms) in enumerate(panels):
        x0 = k * (w + gap)
        sheet.paste(img, (x0, title_h))
        box(d, (x0 + 10, title_h + 8), label, big)
        if ms is not None:
            box(d, (x0 + 10, title_h + h - 26), f"GPU frame {ms:.2f} ms", small)
    out.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(out)
    print(out)


if __name__ == "__main__":
    main(sys.argv[1:])
