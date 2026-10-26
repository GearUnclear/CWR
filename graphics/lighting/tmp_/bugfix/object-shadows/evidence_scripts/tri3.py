import sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
d = Path(sys.argv[1]); out = Path(sys.argv[2])
files = [("sun visibility (all terms)", "000_pine_stand_noon__dbgsunvis.png"), ("cloudShadows=0", "001_pine_stand_noon__dbgsunvis_nocloud.png"), ("terrainShadows=0", "002_pine_stand_noon__dbgsunvis_noterrain.png")]
ims = [Image.open(d / f).convert("RGB") for _, f in files]
w, h = ims[0].size
s = 0.6
tw, th = int(w * s), int(h * s)
c = Image.new("RGB", (tw * 3 + 16, th + 30), (24, 24, 24))
dr = ImageDraw.Draw(c)
try:
    f = ImageFont.truetype("arial.ttf", 16)
except OSError:
    f = ImageFont.load_default()
for i, ((t, _), im) in enumerate(zip(files, ims)):
    c.paste(im.resize((tw, th)), (i * (tw + 8), 30))
    dr.text((i * (tw + 8) + 6, 6), "AFTER, pine_stand_noon: " + t, fill=(255, 255, 255), font=f)
c.save(out)
