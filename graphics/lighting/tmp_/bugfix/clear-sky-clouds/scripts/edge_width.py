import numpy as np
from noise_scale import build
fld = build(256, 1).astype(float)/255.0
for tile in (6000.0,):
    texel = tile/256
    gx = (np.roll(fld, -1, 1) - np.roll(fld, 1, 1)) / (2*texel)
    gz = (np.roll(fld, -1, 0) - np.roll(fld, 1, 0)) / (2*texel)
    g = np.hypot(gx, gz)
    for cover in (0.041, 0.1, 0.263, 0.3, 0.5, 0.9):
        edge = 0.08*min(1.0, 4*cover*(1-cover))
        thr = 1-cover
        near = np.abs(fld - thr) < 0.01
        gm = np.median(g[near])
        print(f"tile {tile:.0f} cover {cover:.3f}: edge +-{edge:.3f}, median |grad| at threshold {gm*1000:.3f} per km -> soft edge width {2*edge/gm:.0f} m")
