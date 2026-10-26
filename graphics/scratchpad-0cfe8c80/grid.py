"""grid.py OUT.png COLS WIDTH img[:label] ... (optional crop via --crop x0,y0,x1,y1 in fractions)"""
import sys
from PIL import Image, ImageDraw

args = sys.argv[1:]
crop = None
if args and args[0] == "--crop":
    crop = [float(v) for v in args[1].split(",")]
    args = args[2:]
out, cols, width = args[0], int(args[1]), int(args[2])
items = []
for a in args[3:]:
    if "::" in a:
        p, lab = a.split("::", 1)
    else:
        p, lab = a, a.replace("\\", "/").split("/")[-1]
    im = Image.open(p).convert("RGB")
    if crop:
        W, H = im.size
        im = im.crop((int(crop[0] * W), int(crop[1] * H), int(crop[2] * W), int(crop[3] * H)))
    h = int(im.height * width / im.width)
    items.append((im.resize((width, h), Image.LANCZOS), lab))
rows = (len(items) + cols - 1) // cols
cellh = max(i.height for i, _ in items) + 18
sheet = Image.new("RGB", (cols * (width + 4), rows * cellh), (20, 20, 20))
d = ImageDraw.Draw(sheet)
for k, (im, lab) in enumerate(items):
    r, c = divmod(k, cols)
    x, y = c * (width + 4), r * cellh
    sheet.paste(im, (x, y + 16))
    d.text((x + 2, y + 2), lab[:80], fill=(240, 220, 120))
sheet.save(out)
print(out, sheet.size)
