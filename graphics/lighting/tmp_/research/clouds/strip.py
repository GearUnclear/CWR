"""Horizontal filmstrip of a capture sequence with a caption per frame.

usage: python strip.py OUT.png TITLE RUN_DIR VIEW MODE[:CAPTION] [MODE[:CAPTION] ...] [--scale s] [--gif OUT.gif]
"""
import sys
from pathlib import Path

from PIL import Image, ImageDraw

from compare3 import box, font


def main(argv):
    scale = 0.5
    gif = None
    args = []
    i = 0
    while i < len(argv):
        if argv[i] == "--scale":
            scale = float(argv[i + 1])
            i += 2
        elif argv[i] == "--gif":
            gif = Path(argv[i + 1])
            i += 2
        else:
            args.append(argv[i])
            i += 1
    out, title, run, view, modes = Path(args[0]), args[1], Path(args[2]), args[3], args[4:]
    frames = []
    for m in modes:
        mode, _, cap = m.partition(":")
        img = Image.open(run / f"{view}__{mode}.png").convert("RGB")
        img = img.resize((int(img.width * scale), int(img.height * scale)), Image.LANCZOS)
        d = ImageDraw.Draw(img)
        box(d, (8, 6), cap or mode, font(16))
        frames.append(img)
    w, h = frames[0].size
    cols = min(3, len(frames))
    rows = (len(frames) + cols - 1) // cols
    gap, title_h = 6, 36
    sheet = Image.new("RGB", (cols * w + (cols - 1) * gap, title_h + rows * h + (rows - 1) * gap), (24, 24, 24))
    ImageDraw.Draw(sheet).text((10, 8), title, font=font(20), fill=(235, 235, 235))
    for k, f in enumerate(frames):
        sheet.paste(f, ((k % cols) * (w + gap), title_h + (k // cols) * (h + gap)))
    sheet.save(out)
    print(out)
    if gif:
        frames[0].save(gif, save_all=True, append_images=frames[1:], duration=600, loop=0)
        print(gif)


if __name__ == "__main__":
    main(sys.argv[1:])
