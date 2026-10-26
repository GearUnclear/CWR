#!/usr/bin/env python
"""fable-3: the per-sample swing of the two DISCONTINUOUS-ish terms a point sample sees in a far slice,
against the smooth ramps.

1. The haze's near-thinning ramp: hazeKeep = mix(0.06, 1, smoothstep(200, 3000, dist)) (fogWind.w, .z),
   hazeLobe keep = mix(0.1, 1, smoothstep(30, 800, dist)).  Per slice: the point sample's swing over the
   depth offset vs the slice mean, and the mid-point's error.
2. The mist top's noise: cloudNoise = 5 octaves of value noise, periods 4..64 per 256 tile, equalised,
   read at the 310 m scale (n2, also + p.y * 0.002) and the 1100 m scale (n1).  Along a ray the
   top moves by (n2 - 0.5) * 0.35 * depth and depth * mix(0.6, 1.35, n1); the banks by
   1.7 * smoothstep(0.2, 0.85, 0.6 n1 + 0.4 n2).  For slices of 40, 90, 190 m: the point sample's
   swing of the MIST density (ray at -8 deg, 30 m layer, eye 120 m up: the crossing slices), the
   mid-point error, the error of 2 lookups at 1/4 and 3/4 (det1n), 4 strata, vs the slice mean.
Run with plain python.
"""
import math
import numpy as np

rng = np.random.default_rng(7)


def smoothstep(a, b, x):
    t = np.clip((x - a) / (b - a), 0.0, 1.0)
    return t * t * (3 - 2 * t)


def value_noise_1d(n_points, x, period_cells, seed):
    """1-D value noise along x (in tile units, 1 tile = 256 texels) with `period_cells` cells per tile."""
    r = np.random.default_rng(seed)
    cells = int(period_cells)
    lattice = r.random(cells + 1)
    lattice[-1] = lattice[0]
    u = (x % 1.0) * cells
    i = np.floor(u).astype(int)
    f = u - i
    f = f * f * (3 - 2 * f)
    return lattice[i] * (1 - f) + lattice[i + 1] * f


def cloud_noise_1d(x):
    """5 octaves, periods 4, 8, 16, 32, 64 per tile, amplitudes 1, .5, .25, .125, .0625, rank-equalised
    (approximated by a global rank over a long sample)."""
    s = np.zeros_like(x)
    amp, per = 1.0, 4
    for o in range(5):
        s += amp * value_noise_1d(len(x), x, per, 100 + o)
        amp *= 0.5
        per *= 2
    return s


# A long ray sample: 20 km along the ray at 0.5 m steps, the noise equalised over it.
L = 20000.0
step = 0.5
t = np.arange(0, L, step)
raw2 = cloud_noise_1d(t / 310.0)
raw1 = cloud_noise_1d(t / 1100.0 + 0.3)
n2 = np.argsort(np.argsort(raw2)) / float(len(raw2))
n1 = np.argsort(np.argsort(raw1)) / float(len(raw1))


def mist_along(elev_deg, eye, layer, fall=18.0, sigma=2e-3, use_noise=True):
    y = eye + math.sin(math.radians(elev_deg)) * t
    if use_noise:
        top = layer * (0.6 + 0.75 * n1) + (n2 - 0.5) * 0.35 * layer
        banks = 1.0 + (1.7 * smoothstep(0.2, 0.85, 0.6 * n1 + 0.4 * n2) - 1.0) * 0.8  # fogMist.w = mistNoise 0.8
    else:
        top = layer * (0.6 + 0.75 * 0.5)
        banks = np.ones_like(t) * (1.0 + (1.7 * smoothstep(0.2, 0.85, 0.5) - 1.0) * 0.8)
    above = y - top
    shape = np.where(above <= 0, 1.0, np.exp(-np.maximum(above, 0) / fall))
    return sigma * shape * banks


def slice_stats(values, z0, z1, n_strata=4):
    i0, i1 = int(z0 / step), int(z1 / step)
    seg = values[i0:i1]
    mean = seg.mean()
    if mean <= 0:
        return None
    swing = (seg.max() - seg.min()) / mean
    rms = seg.std() / mean
    mid = (seg[len(seg) // 2] - mean) / mean
    two = (0.5 * (seg[len(seg) // 4] + seg[3 * len(seg) // 4]) - mean) / mean
    four = (np.mean([seg[int((k + 0.5) / 4 * len(seg))] for k in range(4)]) - mean) / mean
    return mean, swing, rms, mid, two, four


print("1. HAZE RAMPS (the clear haze sigma 6e-4 * exp(-y/1200) * hazeKeep): per slice, far slices")
dist = t
y_h = 100.0 + math.sin(math.radians(-3)) * t
haze = 6e-4 * np.exp(-np.maximum(y_h, -50) / 1200.0) * (0.06 + 0.94 * smoothstep(200.0, 3000.0, dist))
lobe = 6e-4 * np.exp(-np.maximum(y_h, -50) / 1200.0) * (0.1 + 0.9 * smoothstep(30.0, 800.0, dist))
print("   slice span (m)      haze: swing   rms   mid   2pt  4st  |  lobe keep: swing  mid")
for z0, z1 in [(80, 88), (181, 200), (398, 439), (587, 647), (864, 951), (1269, 1396), (1861, 2048), (2729, 3003)]:
    h = slice_stats(haze, z0, z1)
    l = slice_stats(lobe, z0, z1)
    print("   %5d..%5d (%3d)   %5.1f%% %5.1f%% %5.2f%% %5.2f%% %5.2f%%  |  %5.1f%% %5.2f%%" % (
        z0, z1, z1 - z0, 100 * h[1], 100 * h[2], 100 * h[3], 100 * h[4], 100 * h[5], 100 * l[1], 100 * l[3]))

print("\n2. MIST with the cloudNoise top and banks (ray -8 deg, eye 120 m over the floor, layer 35 m):")
print("   per slice: point-sample swing / rms vs the slice mean; errors of: mid tap, 2 taps at 1/4,3/4 (det1n),")
print("   4 strata.  'smooth' = the same ray with the noise replaced by 0.5 (the ramps only).")
for use_noise in (False, True):
    m = mist_along(-8.0, 120.0, 35.0, use_noise=use_noise)
    print("  %s" % ("WITH NOISE" if use_noise else "SMOOTH (noise = 0.5)"))
    print("   slice span (m)     mean/m    swing    rms    mid    2pt    4st")
    for z0, z1 in [(398, 439), (484, 533), (587, 647), (647, 712), (712, 784), (784, 864), (864, 951), (951, 1047)]:
        s = slice_stats(m, z0, z1)
        if s is None:
            continue
        print("   %5d..%5d (%3d)  %8.2e %6.0f%% %5.0f%% %6.1f%% %6.1f%% %6.1f%%" % (
            z0, z1, z1 - z0, s[0], 100 * s[1], 100 * s[2], 100 * s[3], 100 * s[4], 100 * s[5]))
# Across many random phases of the noise (different rays), the rms of each estimator's error for a 60 m slice
print("\n   over 200 random slice positions of 60 m in the crossing zone (ray -8 deg): rms error of each estimator")
m = mist_along(-8.0, 120.0, 35.0, use_noise=True)
errs = {"point": [], "mid": [], "2pt": [], "4st": []}
for _ in range(200):
    z0 = rng.uniform(450, 900)
    s = slice_stats(m, z0, z0 + 60)
    if s is None:
        continue
    seg = m[int(z0 / step):int((z0 + 60) / step)]
    u = rng.random()
    errs["point"].append((seg[int(u * len(seg))] - s[0]) / s[0])
    errs["mid"].append(s[3])
    errs["2pt"].append(s[4])
    errs["4st"].append(s[5])
for k, v in errs.items():
    print("   %-6s rms %5.1f%%  max %5.1f%%" % (k, 100 * float(np.sqrt(np.mean(np.square(v)))), 100 * float(np.max(np.abs(v)))))
print("\n   the same for the mist's lateral (cell) footprint at 600 m: a 12 px cell = %.1f m" % (600 * math.radians(0.668)))
seg_len = 600 * math.radians(0.668)
errs = {"point": [], "mid": [], "2pt": []}
for _ in range(400):
    z0 = rng.uniform(450, 900)
    i0, i1 = int(z0 / step), int((z0 + seg_len) / step)
    seg = m[i0:i1]
    mean = seg.mean()
    if mean <= 0:
        continue
    u = rng.random()
    errs["point"].append((seg[int(u * len(seg))] - mean) / mean)
    errs["mid"].append((seg[len(seg) // 2] - mean) / mean)
    errs["2pt"].append((0.5 * (seg[len(seg) // 4] + seg[3 * len(seg) // 4]) - mean) / mean)
for k, v in errs.items():
    print("   %-6s rms %5.1f%%  max %5.1f%%" % (k, 100 * float(np.sqrt(np.mean(np.square(v)))), 100 * float(np.max(np.abs(v)))))
