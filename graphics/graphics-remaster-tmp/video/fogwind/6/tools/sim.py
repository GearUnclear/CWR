#!/usr/bin/env python
"""agent 6: toy model of the froxel injection's history, static camera, 2D cells x depth.
Field: smooth haze gradient + a soft mist top (tanh, 0.6 cell wide in depth) + a ridge-shadow
edge in x. Each frame every cell samples the field at its jittered point and blends with the
history (share 0.12) read either at the jittered point (as shipped: trilinear) or at the centre.
Output per variant: temporal rms of the displayed value (after optional tent) and the
travelling-pattern correlation (best lag != 0 of consecutive frame differences).
"""
import numpy as np
from scipy import ndimage

NX, NY, NZ = 96, 64, 8
T = 400
phi = 0.6180339887


def field(x, y, z):
    # x, y cells, z slices (continuous). smooth + mist top along a slanted surface + shadow edge
    smooth = 0.02 * x + 0.015 * y + 0.08 * z
    mist = 0.5 * (1 + np.tanh((z - 3.5 - 0.03 * y - 0.01 * x) / 0.3))
    shadow = 0.3 * (1 + np.tanh((x - 40.3 - 0.2 * z) / 0.25))
    return smooth + mist + shadow


def ign(px, py):
    return np.mod(52.9829189 * np.mod(px * 0.06711056 + py * 0.00583715, 1.0), 1.0)


X, Y = np.meshgrid(np.arange(NX), np.arange(NY), indexing="xy")  # (NY, NX)
cx, cy = X + 0.5, Y + 0.5


def run(variant):
    hist = None
    outs = []
    for n in range(T):
        jz = (phi * n) % 1.0
        jx = (0.7548776662 * n) % 1.0 - 0.5
        jy = (0.5698402910 * n) % 1.0 - 0.5
        cur = np.zeros((NZ, NY, NX))
        ox = np.zeros((NZ, NY, NX)); oy = np.zeros_like(ox); oz = np.zeros_like(ox)
        for s in range(NZ):
            if variant.startswith("d91"):
                g = ign(cx, cy)
                oz[s] = np.mod(jz + g + 0.618034 * s, 1.0) - 0.5
                ox[s] = np.mod(jx + 0.5 + ign(cx + 17, cy + 59), 1.0) - 0.5
                oy[s] = np.mod(jy + 0.5 + ign(cx + 83, cy + 29), 1.0) - 0.5
            elif variant.startswith("orig") or variant.startswith("C0"):
                oz[s] = jz - 0.5; ox[s] = jx; oy[s] = jy
            elif variant.startswith("S2"):
                px, py = X & 1, Y & 1
                kq = np.where(py == 0, px, 3 - px)
                a = 2 * np.pi * jz + 0.5 * np.pi * kq
                r = 0.408248
                ox[s] = r * np.cos(a); oy[s] = r * np.sin(a)
                oz[s] = r * np.cos(a + 0.5 * np.pi + 2.3999632 * s)
            cur[s] = field(X + ox[s], Y + oy[s], s + oz[s])
        if hist is None:
            hist = cur.copy()
        else:
            if variant in ("orig", "d91", "S2j"):  # history at the jittered point, trilinear, clamp
                coords = np.stack([np.arange(NZ)[:, None, None] + oz, Y[None] + oy, X[None] + ox])
                h = ndimage.map_coordinates(hist, coords, order=1, mode="nearest")
            else:
                h = hist
            hist = 0.88 * h + 0.12 * cur
        disp = hist
        if variant.endswith("t") and not variant.endswith("nt") or variant in ("S2", "S2j", "d91t"):
            k = np.array([1, 2, 1]) / 4.0
            disp = ndimage.convolve1d(ndimage.convolve1d(hist, k, axis=2, mode="nearest"), k, axis=1, mode="nearest")
        outs.append(disp.sum(0))  # integrated (sum over slices) per column
    o = np.array(outs[100:])
    d = np.diff(o, axis=0)
    rms = d[:, 2:-2, 2:-2].std()
    # travelling: corr of d[t] with d[t+1] shifted by lags
    best, bl = -1, None
    a0 = d[:-1, 4:-4, 4:-4]; b0 = d[1:, :, :]
    for dy in range(-3, 4):
        for dx in range(-3, 4):
            if dx == 0 and dy == 0:
                continue
            b = b0[:, 4 + dy:NY - 4 + dy, 4 + dx:NX - 4 + dx]
            c = (a0 * b).sum() / np.sqrt((a0 * a0).sum() * (b * b).sum() + 1e-30)
            if c > best:
                best, bl = c, (dx, dy)
    lowfreq = ndimage.uniform_filter(o - o.mean(0), size=(1, 8, 8))[:, 4:-4, 4:-4].std()
    return rms, best, bl, lowfreq, o.mean(0)


ref = None
for v in ["orig", "d91", "d91t", "C0", "S2nt", "S2j", "S2"]:
    rms, rho, lag, lf, mean = run(v)
    if ref is None:
        ref = mean
    print("%-5s frame-diff rms %.5f  travelling rho %.3f at %s  8-cell pump rms %.5f  mean vs orig %.4f" %
          (v, rms, rho, lag, lf, np.abs(mean - ref)[2:-2, 2:-2].mean()))
