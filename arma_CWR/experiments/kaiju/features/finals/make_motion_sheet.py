r"""Contact sheet of a finals motion run: idle, then the walk from the side, the front quarter, the legs
close, behind, head on, and passing a man.

  python make_motion_sheet.py <letter> [<run tag>]      (default tag <letter>_anim1)

Reads shots\<tag>\*.png under kaiju-notes\features\finals, writes move_<letter>.png there.
"""
import glob
import os
import sys

from PIL import Image, ImageDraw, ImageFont

NOTES = r"D:\Arma_CWA_worktrees\kaiju-notes\features\finals"
letter = sys.argv[1]
tag = sys.argv[2] if len(sys.argv) > 2 else letter + "_anim1"
fs = sorted(f for f in glob.glob(os.path.join(NOTES, "shots", tag, "*.png")) if not f.endswith("sheet.png"))
rows = [
    ("Idle, front quarter, every 3 s", [f for f in fs if "idle_front" in f], None),
    ("Walk, his right side from 75 m, every second", [f for f in fs if f.endswith("_side.png")][::2][:8], (360, 0, 1560, 1080)),
    ("Walk, front quarter from 80 m, every second", [f for f in fs if "_front_quarter" in f and "idle" not in f][::2][:8], (360, 0, 1560, 1080)),
    ("Walk, legs from 40 m, every second", [f for f in fs if "legs_side" in f][::2][:8], None),
    ("Walk, from behind and head on", [f for f in fs if "behind" in f][::2][:4] + [f for f in fs if "head_on" in f][::2][:4], (360, 0, 1560, 1080)),
    ("Walk, past a man standing 14 m from his track (camera at 1.7 m, fixed)", [f for f in fs if "passing" in f][::3][:8], None),
]
cw, cap = 480, 30
font = ImageFont.truetype(r"C:\Windows\Fonts\segoeui.ttf", 20)
strips = []
for title, names, crop in rows:
    ims = []
    for n in names:
        im = Image.open(n).convert("RGB")
        if crop:
            im = im.crop(crop)
        im.thumbnail((cw, cw), Image.LANCZOS)
        ims.append(im)
    if ims:
        strips.append((title, ims))
height = sum(cap + max(i.height for i in ims) for _t, ims in strips)
sheet = Image.new("RGB", (cw * 8, height), (18, 18, 20))
d = ImageDraw.Draw(sheet)
y = 0
for title, ims in strips:
    d.text((10, y + 3), title, fill=(235, 235, 235), font=font)
    y += cap
    for k, im in enumerate(ims):
        sheet.paste(im, (k * cw, y))
    y += max(i.height for i in ims)
out = os.path.join(NOTES, "move_%s.png" % letter)
sheet.save(out)
print("wrote", out, sheet.size)
