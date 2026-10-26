"""Numpy mirror of SkyDensity.cpp's BuildSkyDensityMap and SkyShadowCoverage on
a decoded weather sky (a PNG of the game texture, research assets; game data,
never committed). Prints the shadow cover per elevation band and writes a
picture of the map.

    python density_mirror.py <sky.png> <out.png> [deck weight]
"""
import sys

import numpy as np
from PIL import Image

RING_E = [-15.0, 0.87, 29.5, 59.5, 89.0]
RING_V = [1.0, 0.9869, 0.5434, 0.1736, 0.036]


def smooth(a, b, x):
    t = np.clip((x - a) / (b - a), 0, 1)
    return t * t * (3 - 2 * t)


def elev_of_v(v):
    return np.interp(v, RING_V[::-1], RING_E[::-1])


def blur(p):
    k = np.array([1, 4, 6, 4, 1]) / 16.0
    t = sum(k[i] * np.roll(p, i - 2, axis=1) for i in range(5))
    pad = np.pad(t, ((2, 2), (0, 0)), mode="edge")
    return sum(k[i] * pad[i:i + p.shape[0]] for i in range(5))


img = np.asarray(Image.open(sys.argv[1]).convert("RGB")).astype(np.float64) / 255.0
deck_w = float(sys.argv[3]) if len(sys.argv) > 3 else 0.16
h, w, _ = img.shape
q = (img[..., 0] + img[..., 1]) / np.maximum(2 * img[..., 2], 1e-3)
qmin = q.min(axis=1, keepdims=True)
mx, mn = img.max(axis=2), img.min(axis=2)
sat = ((mx - mn) / np.maximum(mx, 1e-3)).mean(axis=1)
deck_row = 1 - smooth(0.08, 0.16, sat)
feature = blur(smooth(0.03, 0.30, q - qmin))
lum = img @ np.array([0.2126, 0.7152, 0.0722])
dark = blur(np.clip(1.2 - lum * 1.3, 0, 1))
r = np.round(feature * 255) / 255
g = np.round(np.repeat(deck_row[:, None], w, axis=1) * 255) / 255
b = np.round(dark * 255) / 255
deck = g * deck_w
dens = np.maximum(r, deck) * ((0.75 + 0.25 * b) * (1 - deck) + (0.5 + 0.5 * b) * deck)
cover = smooth(0.43, 0.67, dens)
v = (np.arange(h) + 0.5) / h
e = elev_of_v(v)
for lo, hi in ((-15, 0), (0, 5), (5, 10), (10, 15), (15, 30), (30, 60), (60, 90)):
    m = (e >= lo) & (e < hi)
    if m.any():
        print(f"elev {lo:4d}..{hi:3d}: rows {m.sum():3d}  feature mean {feature[m].mean():.3f}  "
              f"density max {dens[m].max():.3f}  cover mean {cover[m].mean():.3f}")
wgt = np.where(e >= 15, np.cos(np.radians(e)), 0)
print("coverage (>=15 deg, solid angle):", float((cover.mean(axis=1) * wgt).sum() / wgt.sum()))
out = np.stack([r, g, b], axis=2)
Image.fromarray((out * 255).astype(np.uint8)).save(sys.argv[2])
