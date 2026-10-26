"""Region luma of each shot vs the first: python regv.py x0 x1 y0 y1 img..."""
import sys
import numpy as np
from PIL import Image
L = np.array([0.2126, 0.7152, 0.0722])
x0, x1, y0, y1 = map(int, sys.argv[1:5])
def st(p):
    a = np.asarray(Image.open(p).convert("RGB")).astype(float)[y0:y1, x0:x1]
    l = (a * L).sum(axis=2)
    return l.mean(), l.std(), a[..., 0].mean() / a[..., 2].mean()
b = st(sys.argv[5])
print(f"{sys.argv[5].split('__')[-1]:12s} luma {b[0]:6.1f} std {b[1]:5.1f} R/B {b[2]:.3f}")
for p in sys.argv[6:]:
    s = st(p)
    print(f"{p.split('__')[-1]:12s} luma {s[0]:6.1f} ({(s[0]-b[0])/b[0]*100:+.1f}%) std {s[1]:5.1f} ({(s[1]-b[1])/b[1]*100:+.1f}%) R/B {s[2]:.3f}")
