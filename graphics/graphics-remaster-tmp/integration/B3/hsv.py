"""Mean colour, hue and saturation of rectangles in capture shots.

    python hsv.py <png> <name>:<y0>:<y1>:<x0>:<x1> [...]
"""
import colorsys
import sys

import numpy as np
from PIL import Image

im = np.asarray(Image.open(sys.argv[1]).convert("RGB")).astype(float)
for spec in sys.argv[2:]:
    name, y0, y1, x0, x1 = spec.split(":")
    m = im[int(y0):int(y1), int(x0):int(x1)].reshape(-1, 3).mean(0)
    h, s, v = colorsys.rgb_to_hsv(*(m / 255.0))
    print(f"  {name:14s} rgb ({m[0]:6.1f} {m[1]:6.1f} {m[2]:6.1f})  hue {h * 360:5.1f}  sat {s:5.3f}  val {v * 255:5.1f}")
