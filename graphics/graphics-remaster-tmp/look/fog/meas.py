"""meas.py img [name x0 y0 x1 y1]...: mean RGB and luminance std of boxes."""
import sys
import numpy as np
from PIL import Image
a = np.asarray(Image.open(sys.argv[1]).convert("RGB")).astype(float)
args = sys.argv[2:]
for i in range(0, len(args), 5):
    n = args[i]; x0, y0, x1, y1 = map(int, args[i + 1:i + 5])
    b = a[y0:y1, x0:x1]
    l = b @ np.array([0.2126, 0.7152, 0.0722])
    print(f"{n:12s} mean ({b[...,0].mean():.0f},{b[...,1].mean():.0f},{b[...,2].mean():.0f}) lum std {l.std():.1f} min {l.min():.0f} max {l.max():.0f}")
