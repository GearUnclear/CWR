"""Invert PoseidonTools terrain render's colour ramp back to metres (AssetPreview.cpp PreviewTerrain)."""
import sys
import numpy as np
from PIL import Image

def decode(path, minH, maxH):
    im = np.asarray(Image.open(path).convert('RGB')).astype(np.float32)
    r, g, b = im[..., 0], im[..., 1], im[..., 2]
    t = np.zeros(r.shape, np.float32)
    # band 1: t in [0.01,0.3): r = 30 + s*100
    s1 = (r - 30) / 100.0
    t1 = 0.01 + s1 * 0.29
    # band 2: r = 130 + s*60, g = 180 - s*60
    s2 = (180 - g) / 60.0
    t2 = 0.3 + s2 * 0.4
    # band 3: r = 190 + s*65, b = 30 + s*225
    s3 = (b - 30) / 225.0
    t3 = 0.7 + s3 * 0.3
    sea = (r == 30) & (g == 60) & (b == 180)
    band1 = (~sea) & (r < 130) & (b < 51)
    band3 = (~sea) & (r >= 190) & (g >= 120) & (b >= 30) & ~((r < 190.5) & (g > 119))
    band2 = (~sea) & ~band1 & ~band3
    t = np.where(band1, t1, np.where(band2, t2, t3))
    t = np.where(sea, 0.0, t)
    h = minH + t * (maxH - minH)
    # image row 0 = north edge (z = h-1); flip so row index = z grid
    return h[::-1, :]

if __name__ == '__main__':
    h = decode(sys.argv[1], float(sys.argv[2]), float(sys.argv[3]))
    np.save(sys.argv[4], h)
    print(h.shape, h.min(), h.max())
