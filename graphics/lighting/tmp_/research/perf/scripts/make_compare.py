"""Labelled N-way comparison strip (PIL).

usage: make_compare.py <out.png> <title> <img1> <label1> <img2> <label2> [...]
A label may carry a second line after a '|' (e.g. "Remastered|GPU world 7.4 ms").
"""
import sys
from PIL import Image, ImageDraw, ImageFont

out = sys.argv[1]
title = sys.argv[2]
pairs = list(zip(sys.argv[3::2], sys.argv[4::2]))
ims = [Image.open(p).convert("RGB") for p, _ in pairs]
w, h = ims[0].size
gap, top = 8, 40
W = len(ims) * w + (len(ims) - 1) * gap
canvas = Image.new("RGB", (W, h + top), (24, 24, 24))
d = ImageDraw.Draw(canvas)
try:
    font = ImageFont.truetype("arialbd.ttf", 20)
    small = ImageFont.truetype("arialbd.ttf", 16)
except OSError:
    font = small = ImageFont.load_default()
d.text((10, 9), title, fill=(235, 235, 235), font=font)
for i, (im, (_, label)) in enumerate(zip(ims, pairs)):
    x = i * (w + gap)
    canvas.paste(im.resize((w, h)), (x, top))
    lines = [x.strip() for x in label.split("|")]
    tw = max(d.textlength(lines[0], font=font), 10)
    d.rectangle([x + 6, top + 6, x + 14 + tw, top + 34], fill=(0, 0, 0))
    d.text((x + 10, top + 9), lines[0], fill=(255, 255, 255), font=font)
    if len(lines) > 1:
        for j, extra in enumerate(lines[1:]):
            ew = d.textlength(extra, font=small)
            y = top + h - 30 - 24 * (len(lines) - 2 - j)
            d.rectangle([x + 6, y - 2, x + 14 + ew, y + 22], fill=(0, 0, 0))
            d.text((x + 10, y), extra, fill=(255, 255, 255), font=small)
canvas.save(out)
print("wrote", out, canvas.size)
