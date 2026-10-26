"""Offline hillshade + cast terrain shadows for planning views (50 m wrp grid)."""
import sys, math
import numpy as np
from PIL import Image

def cast(h, cell, az_deg, el_deg, maxd=6000.0):
    n = h.shape[0]
    az = math.radians(az_deg); el = math.radians(el_deg)
    dx, dz = math.sin(az), math.cos(az)
    tanS = math.tan(el)
    zz, xx = np.mgrid[0:n, 0:n].astype(np.float32)
    best = np.full(h.shape, -1e9, np.float32)
    t = cell * 0.5
    while t < maxd:
        sx = np.clip(xx + dx * t / cell, 0, n - 1)
        sz = np.clip(zz + dz * t / cell, 0, n - 1)
        x0 = np.floor(sx).astype(int); z0 = np.floor(sz).astype(int)
        x1 = np.minimum(x0 + 1, n - 1); z1 = np.minimum(z0 + 1, n - 1)
        fx = sx - x0; fz = sz - z0
        hh = (h[z0, x0] * (1 - fx) + h[z0, x1] * fx) * (1 - fz) + (h[z1, x0] * (1 - fx) + h[z1, x1] * fx) * fz
        best = np.maximum(best, (hh - h) / t)
        t += cell * 0.5
    return best < tanS

if __name__ == '__main__':
    h = np.load(sys.argv[1]); cell = 50.0
    az, el = float(sys.argv[2]), float(sys.argv[3])
    sh = cast(np.maximum(h, 0), cell, az, el)
    gz, gx = np.gradient(np.maximum(h, 0), cell)
    nrm = np.stack([-gx, np.ones_like(h), -gz], -1)
    nrm /= np.linalg.norm(nrm, axis=-1, keepdims=True)
    l = np.array([math.sin(math.radians(az)) * math.cos(math.radians(el)), math.sin(math.radians(el)), math.cos(math.radians(az)) * math.cos(math.radians(el))])
    nl = np.clip(nrm @ l, 0, 1) * sh
    img = (0.25 + 0.75 * nl / max(nl.max(), 1e-3))
    img = np.where(h < 0, 0.1, img)
    rgb = np.stack([img, img, img], -1)
    rgb[..., 2] = np.where(sh, rgb[..., 2], rgb[..., 2] + 0.25)
    out = (np.clip(rgb[::-1], 0, 1) * 255).astype(np.uint8)  # north up
    Image.fromarray(out).resize((768, 768), Image.NEAREST).save(sys.argv[4])
