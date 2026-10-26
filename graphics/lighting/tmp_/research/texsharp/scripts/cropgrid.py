"""Crop the same region out of several shots of one view and lay them out 2x.

    python cropgrid.py <shots_dir> <view> <x0,y0,x1,y1> <out.png> label1 label2 ... [--scale 2] [--cols 2]
"""
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, str(Path(__file__).parent))
from compare import NAMES, font, label_box  # noqa: E402


def main():
    args = sys.argv[1:]
    scale = 2
    cols = 2
    if "--scale" in args:
        i = args.index("--scale"); scale = int(args[i + 1]); del args[i:i + 2]
    if "--cols" in args:
        i = args.index("--cols"); cols = int(args[i + 1]); del args[i:i + 2]
    shots, view, box, out = Path(args[0]), args[1], tuple(int(v) for v in args[2].split(",")), args[3]
    labels = args[4:]
    ims = []
    for l in labels:
        im = Image.open(shots / f"{view}__{l}.png").convert("RGB").crop(box)
        ims.append(im.resize((im.width * scale, im.height * scale), Image.NEAREST))
    w, h = ims[0].size
    rows = (len(ims) + cols - 1) // cols
    sheet = Image.new("RGB", (cols * w + (cols - 1) * 6, rows * h + (rows - 1) * 6 + 32), (20, 20, 20))
    d = ImageDraw.Draw(sheet)
    d.text((8, 5), f"{view}: crop {box} of the 960x640 frame, {scale}x nearest", font=font(18), fill=(230, 230, 230))
    for k, (l, im) in enumerate(zip(labels, ims)):
        r, c = divmod(k, cols)
        dd = ImageDraw.Draw(im)
        label_box(dd, (8, 6), NAMES.get(l, l), font(18))
        sheet.paste(im, (c * (w + 6), 32 + r * (h + 6)))
    sheet.save(out)
    print(out)


if __name__ == "__main__":
    main()
