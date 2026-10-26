import numpy as np
from noise_scale import build

def chords(mask, texel):
    runs_c, runs_g = [], []
    for row in mask:
        r = np.concatenate([row, row])  # wrap
        # find runs in the doubled row, keep those starting in first copy and not spanning whole
        changes = np.flatnonzero(np.diff(r.astype(int))) + 1
        if len(changes) == 0:
            continue
        starts = changes[:-1]
        ends = changes[1:]
        for s, e in zip(starts, ends):
            if s >= len(row):
                break
            (runs_c if r[s] else runs_g).append((e - s) * texel)
    return np.array(runs_c), np.array(runs_g)

def screen_mixed(f01, cover, tile_m, rng, n=3000, h=1.8, rmin=3.0, rmax=1500.0, half_fov=0.6):
    size = f01.shape[0]
    thr = 1 - cover
    mixed = 0
    near_edge = 0
    for _ in range(n):
        cx, cz = rng.uniform(0, tile_m, 2)
        yaw = rng.uniform(0, 2*np.pi)
        # flat-ground screen weighting: pdf ~ r * (1/r^3) = 1/r^2 in r -> inverse-cdf
        u = rng.uniform(0, 1, 600)
        r = 1.0 / (1.0/rmin - u*(1.0/rmin - 1.0/rmax))
        a = yaw + rng.uniform(-half_fov, half_fov, 600)
        ix = (((cx + r*np.cos(a)) % tile_m) / tile_m * size).astype(int) % size
        iz = (((cz + r*np.sin(a)) % tile_m) / tile_m * size).astype(int) % size
        f = (f01[iz, ix] >= thr).mean()
        if 0.1 < f < 0.9:
            mixed += 1
        # an edge within 400 m in the view cone (uniform in area)
        r2 = np.sqrt(rng.uniform((5/400)**2, 1, 300)) * 400
        a2 = yaw + rng.uniform(-half_fov, half_fov, 300)
        ix = (((cx + r2*np.cos(a2)) % tile_m) / tile_m * size).astype(int) % size
        iz = (((cz + r2*np.sin(a2)) % tile_m) / tile_m * size).astype(int) % size
        c = f01[iz, ix] >= thr
        if c.any() and (~c).any():
            near_edge += 1
    return mixed/n, near_edge/n

rng = np.random.default_rng(5)
configs = [("shipped: base period 4, 6 km tile", 4, 5, 6000.0),
           ("tile 3 km (kCloudScale 1/3000)", 4, 5, 3000.0),
           ("tile 2 km", 4, 5, 2000.0),
           ("base period 8, 6 km tile, 5 oct", 8, 5, 6000.0),
           ("base period 16, 6 km tile, 4 oct", 16, 4, 6000.0)]
for label, bp, octv, tile in configs:
    fld = build(256, 1, bp, octv).astype(float)/255.0
    texel = tile/256
    for cover in (0.263, 0.30, 0.5):
        m = fld >= 1-cover
        c, g = chords(m, texel)
        sm, ne = screen_mixed(fld, cover, tile, rng)
        print(f"{label:36s} cover {cover:.3f}: shadow chord mean {c.mean():5.0f} m, lit chord mean {g.mean():5.0f} m; "
              f"eye-level frame mixed {sm:.2f}; edge within 400 m in view {ne:.2f}")
    print()
