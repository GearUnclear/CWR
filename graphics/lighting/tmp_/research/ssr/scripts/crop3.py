"""Zoomed three-way crop: python crop3.py <shots_dir> <view> <variant> x0 y0 x1 y1 <out.png> [scale]"""
import sys
from PIL import Image, ImageDraw, ImageFont
d, view, var = sys.argv[1], sys.argv[2], sys.argv[3]
x0, y0, x1, y1 = map(int, sys.argv[4:8]); out = sys.argv[8]; sc = float(sys.argv[9]) if len(sys.argv) > 9 else 2.0
labs = [("classic", "Classic"), ("remastered", "Remastered"), (var, "Remastered + SSR")]
ims = [Image.open(f"{d}/{view}__{m}.png").convert("RGB").crop((x0, y0, x1, y1)) for m, _ in labs]
w, h = int((x1 - x0) * sc), int((y1 - y0) * sc)
ims = [im.resize((w, h), Image.LANCZOS) for im in ims]
try: f = ImageFont.truetype("C:/Windows/Fonts/arialbd.ttf", 22)
except OSError: f = ImageFont.load_default()
c = Image.new("RGB", (w * 3 + 16, h + 34), (24, 24, 24)); dr = ImageDraw.Draw(c)
dr.text((8, 4), f"{view} crop ({x0},{y0})-({x1},{y1}) x{sc}", fill=(230, 230, 230), font=f)
for i, (im, (_, n)) in enumerate(zip(ims, labs)):
    c.paste(im, (i * (w + 8), 34)); dr.text((i * (w + 8) + 8, 40), n, fill=(255, 255, 255), font=f)
c.save(out); print(out)
