"""Zoomed crops of one region across labels, tiled with captions.

    python crop_grid.py OUT.png "Title" x0,y0,x1,y1 zoom cols PANEL [PANEL ...]

PANEL = run_dir|view|label|caption
"""
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


def font(size):
    for f in ("C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/arial.ttf"):
        try:
            return ImageFont.truetype(f, size)
        except OSError:
            pass
    return ImageFont.load_default()


def main():
    out, title, box, zoom, cols = sys.argv[1], sys.argv[2], tuple(int(v) for v in sys.argv[3].split(",")), float(sys.argv[4]), int(sys.argv[5])
    panels = [p.split("|") for p in sys.argv[6:]]
    w, h = int((box[2] - box[0]) * zoom), int((box[3] - box[1]) * zoom)
    rows = (len(panels) + cols - 1) // cols
    pad, top = 4, 34
    sheet = Image.new("RGB", (cols * w + (cols - 1) * pad, top + rows * h + (rows - 1) * pad), (20, 20, 20))
    d = ImageDraw.Draw(sheet)
    d.text((8, 4), title, fill=(235, 235, 235), font=font(20))
    f = font(16)
    for i, (run, view, label, cap) in enumerate(panels):
        im = Image.open(Path(run) / f"{view}__{label}.png").convert("RGB").crop(box).resize((w, h), Image.LANCZOS)
        x, y = (i % cols) * (w + pad), top + (i // cols) * (h + pad)
        sheet.paste(im, (x, y))
        tw = d.textlength(cap, font=f)
        d.rectangle((x + 4, y + 4, x + 12 + tw, y + 26), fill=(0, 0, 0))
        d.text((x + 8, y + 5), cap, fill=(255, 255, 255), font=f)
    sheet.save(out)
    print(out)


if __name__ == "__main__":
    main()
