import sys
from PIL import Image
r = sys.argv[1]; v7 = sys.argv[2]; out = sys.argv[3]
box = (1050, 0, 1750, 560)
tiles = []
for d, tag in ((v7, "v7"), (r, "r2")):
    for m in ("classic", "off", "on"):
        tiles.append(Image.open(f"{d}/combat_eye_k00__{m}.png").convert("RGB").crop(box))
w, h = tiles[0].size
sheet = Image.new("RGB", (w * 3, h * 2))
for i, t in enumerate(tiles):
    sheet.paste(t, ((i % 3) * w, (i // 3) * h))
sheet.save(out)
