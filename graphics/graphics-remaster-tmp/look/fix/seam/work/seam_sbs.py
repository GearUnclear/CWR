"""Before | after side by side, full frame on top (half size), a 2x crop of the
problem area below.

    python seam_sbs.py before.png after.png out.png x0,y0 [title]
The crop is 480 x 270 px from (x0, y0) of the 1920 x 1080 frames.
"""
import sys
from PIL import Image, ImageDraw, ImageFont

before, after, out = sys.argv[1], sys.argv[2], sys.argv[3]
x0, y0 = (int(v) for v in sys.argv[4].split(','))
title = sys.argv[5] if len(sys.argv) > 5 else ''
blabel = sys.argv[6] if len(sys.argv) > 6 else 'Before: look director final'
try:
    font = ImageFont.truetype('arial.ttf', 26)
    small = ImageFont.truetype('arial.ttf', 20)
except OSError:
    font = small = ImageFont.load_default()

B = Image.open(before).convert('RGB')
A = Image.open(after).convert('RGB')
assert B.size == (1920, 1080) and A.size == (1920, 1080), (B.size, A.size)
cw, ch = 480, 270
box = (x0, y0, x0 + cw, y0 + ch)
head = 44
sheet = Image.new('RGB', (1920, head + 540 + 540), (16, 16, 16))
d = ImageDraw.Draw(sheet)
d.text((12, 8), title, fill=(235, 235, 235), font=font)
for i, (im, label) in enumerate(((B, blabel), (A, 'After: seam fix'))):
    full = im.resize((960, 540), Image.LANCZOS)
    sheet.paste(full, (i * 960, head))
    fd = ImageDraw.Draw(sheet)
    # where the crop comes from
    fd.rectangle((i * 960 + x0 // 2, head + y0 // 2, i * 960 + (x0 + cw) // 2, head + (y0 + ch) // 2),
                 outline=(255, 210, 0), width=2)
    crop = im.crop(box).resize((960, 540), Image.LANCZOS)
    sheet.paste(crop, (i * 960, head + 540))
    fd.rectangle((i * 960, head, i * 960 + 470, head + 30), fill=(0, 0, 0))
    fd.text((i * 960 + 8, head + 4), label, fill=(255, 210, 0), font=small)
    fd.rectangle((i * 960, head + 540, i * 960 + 200, head + 570), fill=(0, 0, 0))
    fd.text((i * 960 + 8, head + 544), '2x crop', fill=(255, 210, 0), font=small)
d.line((960, head, 960, head + 1080), fill=(16, 16, 16), width=4)
sheet.save(out)
print(out, sheet.size)
