import sys, os
from PIL import Image, ImageDraw
import numpy as np
src, out, v, x0, y0, x1, y1 = sys.argv[1], sys.argv[2], sys.argv[3], *map(int, sys.argv[4:8])
tiles = []
for k in ("classic", "off", "on"):
    im = Image.open(os.path.join(src, f"{v}__{k}.png")).convert("RGB").crop((x0, y0, x1, y1))
    a = np.asarray(im).astype(float)
    m = a.reshape(-1,3).mean(0)
    print(k, "mean rgb", m.round(1), "g:r %.3f r:b %.3f" % (m[1]/m[0], m[0]/m[2]))
    d = ImageDraw.Draw(im); d.rectangle((0,0,60,16), fill=(0,0,0)); d.text((3,3), k, fill=(255,255,255))
    tiles.append(im)
w, h = tiles[0].size
s = Image.new("RGB", (w*3, h))
for i, t in enumerate(tiles): s.paste(t, (i*w, 0))
s.save(os.path.join(out, f"{v}_crop_{x0}_{y0}.png"))
