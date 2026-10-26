import sys
import numpy as np
from PIL import Image
a = np.asarray(Image.open(sys.argv[1]).convert("RGB"), dtype=np.int16)
b = np.asarray(Image.open(sys.argv[2]).convert("RGB"), dtype=np.int16)
d = np.abs(a - b)
print("mean abs diff", d.mean(), "max", d.max(), "pixels >8:", (d.max(-1) > 8).mean())
h = d.shape[0]
for name, (y0, y1) in {"far": (0.36, 0.48), "mid": (0.48, 0.62), "near": (0.62, 1.0)}.items():
    print(name, d[int(y0*h):int(y1*h)].mean())
