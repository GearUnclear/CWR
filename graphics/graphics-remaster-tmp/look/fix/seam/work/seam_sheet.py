import sys, os
from PIL import Image, ImageDraw, ImageOps
# usage: seam_sheet.py out.png x0,y0,x1,y1 scale autocontrast(0/1) cols files...
out = sys.argv[1]
box = tuple(int(v) for v in sys.argv[2].split(','))
sc = float(sys.argv[3])
ac = sys.argv[4] == '1'
cols = int(sys.argv[5])
files = sys.argv[6:]
tiles = []
for f in files:
    im = Image.open(f).convert('RGB').crop(box)
    im = im.resize((int(im.width * sc), int(im.height * sc)), Image.LANCZOS)
    if ac:
        im = ImageOps.autocontrast(im, cutoff=0.5)
    d = ImageDraw.Draw(im)
    label = os.path.basename(os.path.dirname(os.path.dirname(os.path.dirname(f)))) + '/' + os.path.basename(f)
    d.rectangle((0, 0, im.width, 16), fill=(0, 0, 0))
    d.text((4, 2), label, fill=(255, 255, 0))
    tiles.append(im)
rows = (len(tiles) + cols - 1) // cols
W, H = tiles[0].size
sheet = Image.new('RGB', (W * cols, H * rows))
for i, t in enumerate(tiles):
    sheet.paste(t, ((i % cols) * W, (i // cols) * H))
sheet.save(out)
print(sheet.size)
