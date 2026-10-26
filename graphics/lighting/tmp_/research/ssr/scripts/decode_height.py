# Invert PoseidonTools terrain render colour ramp back to metres.
import sys
from PIL import Image
import numpy as np

def decode(path, minH, maxH):
    im = np.asarray(Image.open(path).convert('RGB')).astype(np.float32)
    r, g, b = im[..., 0], im[..., 1], im[..., 2]
    t = np.zeros(r.shape, np.float32)
    deep = (r == 30) & (g == 60) & (b == 180)
    # ramp 1: r=30+s*100 g=120+s*60, b=30+s*20
    s1 = (r - 30) / 100.0
    m1 = (~deep) & (g >= 119) & (b <= 51) & (r <= 131) & (np.abs(g - (120 + s1 * 60)) < 2.5)
    t[m1] = 0.01 + s1[m1] * 0.29
    # ramp 2: r=130+s*60, g=180-s*60, b=50-s*20
    s2 = (r - 130) / 60.0
    m2 = (~deep) & (~m1) & (r >= 129) & (r <= 191) & (np.abs(g - (180 - s2 * 60)) < 2.5)
    t[m2] = 0.3 + s2[m2] * 0.4
    s3 = (r - 190) / 65.0
    m3 = (~deep) & (~m1) & (~m2)
    t[m3] = 0.7 + np.clip(s3[m3], 0, 1) * 0.3
    h = minH + t * (maxH - minH)
    h[deep] = minH
    # image row z is flipped: image row 0 = north (z = h-1)
    return h[::-1, :]  # now index [zgrid, xgrid]

if __name__ == '__main__':
    h = decode(sys.argv[1], float(sys.argv[2]), float(sys.argv[3]))
    np.save(sys.argv[4], h)
    print(h.shape, h.min(), h.max(), (h < 0).mean())
