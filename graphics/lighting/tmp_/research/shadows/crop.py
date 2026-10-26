"""Tight zoomed crops of several shots of one view, stacked vertically or side by side.

    python crop.py <shots_dir> <view> <labels comma> <x0,y0,x1,y1> <scale> <out.png> [--h]
"""
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, str(Path(__file__).parent))
from collect import LABELS, font  # noqa: E402


def main():
    shots, view, labels, box, scale, out = sys.argv[1:7]
    horiz = "--h" in sys.argv
    x0, y0, x1, y1 = [int(v) for v in box.split(",")]
    scale = float(scale)
    ims = []
    for lab in labels.split(","):
        p = Path(shots) / f"{view}__{lab}.png"
        im = Image.open(p).convert("RGB").crop((x0, y0, x1, y1))
        im = im.resize((int(im.width * scale), int(im.height * scale)), Image.LANCZOS)
        d = ImageDraw.Draw(im)
        f = font(18)
        t = LABELS.get(lab, "+ " + lab)
        tb = d.textbbox((0, 0), t, font=f)
        d.rectangle((4, 4, 12 + tb[2], 10 + tb[3]), fill=(0, 0, 0))
        d.text((8, 6), t, font=f, fill=(255, 255, 255))
        ims.append(im)
    gap = 6
    if horiz:
        W = sum(i.width for i in ims) + gap * (len(ims) - 1)
        H = max(i.height for i in ims)
    else:
        W = max(i.width for i in ims)
        H = sum(i.height for i in ims) + gap * (len(ims) - 1)
    canvas = Image.new("RGB", (W, H), (20, 20, 20))
    pos = 0
    for i in ims:
        canvas.paste(i, (pos, 0) if horiz else (0, pos))
        pos += (i.width if horiz else i.height) + gap
    canvas.save(out)
    print(out, canvas.size)


if __name__ == "__main__":
    main()
