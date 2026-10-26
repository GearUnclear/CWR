"""Side-by-side x-t RESID crops (orig | f1 | new | van) for a row, labelled. usage: python xtsheet.py dir y xs0 xs1 t0 t1 [zoom]"""
import sys
from PIL import Image, ImageDraw

d, y, x0, x1, t0, t1 = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4]), int(sys.argv[5]), int(sys.argv[6])
z = int(sys.argv[7]) if len(sys.argv) > 7 else 1
labs = ["orig", "f1", "new", "van"]
names = {"orig": "ORIGINAL", "f1": "F1 d91 (wind)", "new": "NEW q2b", "van": "VANILLA"}
ims = []
for l in labs:
    im = Image.open(f"{d}/xt_resid_{l}_y{y}.png").crop((x0, t0, x1, t1))
    if z > 1:
        im = im.resize((im.width * z, im.height * z), Image.NEAREST)
    ims.append(im)
W = sum(i.width for i in ims) + 10 * (len(ims) - 1)
H = ims[0].height + 16
sheet = Image.new("L", (W, H), 255)
dr = ImageDraw.Draw(sheet)
x = 0
for l, im in zip(labs, ims):
    sheet.paste(im, (x, 16))
    dr.text((x + 3, 2), f"{names[l]} y{y} t{t0}-{t1}", fill=0)
    x += im.width + 10
sheet.save(f"{d}/sheet_y{y}_x{x0}_{x1}_t{t0}_{t1}_z{z}.png")
print(sheet.size)
