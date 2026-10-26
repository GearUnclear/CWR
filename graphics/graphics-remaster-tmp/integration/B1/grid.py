"""Labelled crops side by side.

    python grid.py <out.png> <x0,y0,x1,y1> <scale> <label=path> [<label=path> ...] [--cols N]
"""
import sys

from PIL import Image, ImageDraw, ImageFont


def main():
    args = [a for a in sys.argv[1:]]
    cols = 0
    if "--cols" in args:
        i = args.index("--cols")
        cols = int(args[i + 1])
        del args[i:i + 2]
    out, box, scale = args[0], tuple(int(v) for v in args[1].split(",")), float(args[2])
    items = [a.split("=", 1) for a in args[3:]]
    crops = []
    for label, path in items:
        im = Image.open(path).convert("RGB").crop(box)
        if scale != 1.0:
            im = im.resize((int(im.width * scale), int(im.height * scale)), Image.NEAREST)
        d = ImageDraw.Draw(im)
        try:
            font = ImageFont.truetype("arial.ttf", 18)
        except OSError:
            font = ImageFont.load_default()
        d.rectangle((0, 0, 8 + 10 * len(label), 26), fill=(0, 0, 0))
        d.text((4, 3), label, fill=(255, 255, 255), font=font)
        crops.append(im)
    cols = cols or len(crops)
    rows = (len(crops) + cols - 1) // cols
    w, h = crops[0].size
    sheet = Image.new("RGB", (cols * w + (cols - 1) * 4, rows * h + (rows - 1) * 4), (40, 40, 40))
    for i, im in enumerate(crops):
        sheet.paste(im, ((i % cols) * (w + 4), (i // cols) * (h + 4)))
    sheet.save(out)
    print(out, sheet.size)


main()
