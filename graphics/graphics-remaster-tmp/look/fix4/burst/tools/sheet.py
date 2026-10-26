"""sheet.py out.png scale crop(x0,y0,x1,y1 or 'full') label=path ... : stacks crops in rows of N"""
import sys
from PIL import Image, ImageDraw
out = sys.argv[1]; scale = float(sys.argv[2]); crop = sys.argv[3]; cols = int(sys.argv[4])
items = [a.split("=", 1) for a in sys.argv[5:]]
tiles = []
for label, p in items:
    im = Image.open(p).convert("RGB")
    if crop != "full":
        x0, y0, x1, y1 = [int(v) for v in crop.split(",")]
        im = im.crop((x0, y0, x1, y1))
    im = im.resize((int(im.width * scale), int(im.height * scale)), Image.NEAREST if scale >= 2 else Image.LANCZOS)
    canvas = Image.new("RGB", (im.width, im.height + 16), (20, 20, 20))
    canvas.paste(im, (0, 16))
    ImageDraw.Draw(canvas).text((4, 2), label, fill=(255, 255, 0))
    tiles.append(canvas)
w = max(t.width for t in tiles); h = max(t.height for t in tiles)
rows = (len(tiles) + cols - 1) // cols
sheet = Image.new("RGB", (w * cols + 4 * (cols - 1), h * rows + 4 * (rows - 1)), (0, 0, 0))
for i, t in enumerate(tiles):
    sheet.paste(t, ((i % cols) * (w + 4), (i // cols) * (h + 4)))
sheet.save(out)
print(out, sheet.size)
