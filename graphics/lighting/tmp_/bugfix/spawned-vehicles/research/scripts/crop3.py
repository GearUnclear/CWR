import sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
b, a, out = Path(sys.argv[1]), Path(sys.argv[2]), Path(sys.argv[3])
view, mode, box, scale = sys.argv[4], sys.argv[5], tuple(int(v) for v in sys.argv[6].split(',')), float(sys.argv[7])
panels = [("BEFORE spawned", b / f"{view}__{mode}.png"), ("AFTER spawned", a / f"{view}__{mode}.png"), ("AFTER placed", a / f"placed_{view}__{mode}.png")]
ims = []
for t, p in panels:
    im = Image.open(p).convert("RGB").crop(box)
    im = im.resize((int(im.width * scale), int(im.height * scale)), Image.LANCZOS)
    ims.append((t, im))
w, h = ims[0][1].size
sheet = Image.new("RGB", (w * len(ims), h + 34), (20, 20, 20))
d = ImageDraw.Draw(sheet)
try:
    f = ImageFont.truetype("C:/Windows/Fonts/segoeui.ttf", 20)
except OSError:
    f = ImageFont.load_default()
for i, (t, im) in enumerate(ims):
    sheet.paste(im, (i * w, 34))
    d.text((i * w + 8, 5), f"{t} ({mode})", fill=(255, 255, 255), font=f)
sheet.save(out)
print(out)
