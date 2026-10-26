"""cropcmp.py a.png b.png out.png x0 y0 x1 y1 [zoom] [gain]: a, b and their
difference (x gain) side by side for one crop, nearest-neighbour zoomed."""
import sys
import numpy as np
from PIL import Image

a = Image.open(sys.argv[1]).convert("RGB")
b = Image.open(sys.argv[2]).convert("RGB")
x0, y0, x1, y1 = (int(v) for v in sys.argv[4:8])
zoom = int(sys.argv[8]) if len(sys.argv) > 8 else 4
gain = float(sys.argv[9]) if len(sys.argv) > 9 else 16.0
ca, cb = a.crop((x0, y0, x1, y1)), b.crop((x0, y0, x1, y1))
d = np.abs(np.asarray(ca).astype(np.int16) - np.asarray(cb).astype(np.int16))
d = np.clip(d * gain, 0, 255).astype(np.uint8)
cd = Image.fromarray(d)
w, h = ca.size
out = Image.new("RGB", (w * 3 + 8, h), (255, 0, 255))
for i, im in enumerate((ca, cb, cd)):
    out.paste(im, (i * (w + 4), 0))
out = out.resize((out.width * zoom, out.height * zoom), Image.NEAREST)
out.save(sys.argv[3])
