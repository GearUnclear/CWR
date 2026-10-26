"""x-t and y-t slices, scene-stabilised, with the temporal residual amplified.

x-t: one frame row per time step, shifted by the camera's track (vanilla, per region) so the scene
stands still: the scene is vertical lines, a pump is horizontal stripes, anything travelling is a
slant. y-t: the column under one SCENE point (it follows the pan) over time: rising or sinking
mist is a slant. RESID = value minus its 15-frame centred mean at the same scene point, x gain.
usage: python xt.py outdir label...
"""
import sys, os
import numpy as np
from scipy import ndimage
from PIL import Image
from common import load, Y0, SCRATCH

ROWS = {520: "farvalley", 560: "farvalley", 600: "farvalley", 660: "bay", 700: "bay", 790: "lowmist"}
COLS = {"farhill": (700, 470, 600), "farvalley": (1250, 480, 720), "bay": (1650, 560, 780)}
GAIN = 16
T0, T1 = 0, 569


def track(name):
    tr = np.load(f"{SCRATCH}/camtrack.npy", allow_pickle=True).item()
    d = tr["reg"][name]
    return np.cumsum(d, 0)  # content position shift vs frame 0 (dy, dx)


def tmean(a, n=15):
    return ndimage.uniform_filter1d(a, n, axis=0, mode="nearest")


out = sys.argv[1]
os.makedirs(out, exist_ok=True)
for lab in sys.argv[2:]:
    a = load(lab)
    T = a.shape[0]
    for y, reg in ROWS.items():
        c = track(reg)
        W = 1920 + 460
        canvas = np.full((T, W), np.nan, np.float32)
        xs = np.arange(W).astype(np.float64)  # scene x at frame 0 (content moves left, so new scene enters from the right)
        for t in range(T):
            row = a[t, y - Y0].astype(np.float32)
            # screen x of this scene point at frame t
            sx = xs + c[t, 1]
            ok = (sx >= 0) & (sx <= 1919)
            canvas[t, ok] = np.interp(sx[ok], np.arange(1920), row)
        m = tmean(np.nan_to_num(canvas, nan=0.0))
        w = tmean((~np.isnan(canvas)).astype(np.float32))
        mean = np.where(w > 0.99, m / np.maximum(w, 1e-6), np.nan)
        res = canvas - mean
        img = np.clip(np.nan_to_num(res, nan=0.0) * GAIN + 128, 0, 255).astype(np.uint8)
        raw = np.clip(np.nan_to_num(canvas, nan=0.0), 0, 255).astype(np.uint8)
        np.save(f"{SCRATCH}/xt_{lab}_{y}.npy", res.astype(np.float16))
        Image.fromarray(img).save(f"{out}/xt_resid_{lab}_y{y}.png")
        Image.fromarray(raw).save(f"{out}/xt_raw_{lab}_y{y}.png")
    for name, (x0, ya, yb) in COLS.items():
        c = track(name)
        H = yb - ya
        canvas = np.zeros((H, T), np.float32)
        for t in range(T):
            sx = x0 + (c[t, 1] - c[328, 1])
            sy = np.arange(ya, yb) + (c[t, 0] - c[328, 0])
            if sx < 1 or sx > 1918:
                canvas[:, t] = np.nan
                continue
            canvas[:, t] = ndimage.map_coordinates(a[t].astype(np.float32), [sy - Y0, np.full(H, sx)], order=1)
        m = ndimage.uniform_filter1d(np.nan_to_num(canvas), 15, axis=1, mode="nearest")
        res = canvas - m
        img = np.clip(np.nan_to_num(res) * GAIN + 128, 0, 255).astype(np.uint8)
        Image.fromarray(img).save(f"{out}/yt_resid_{lab}_{name}.png")
    print("done", lab, flush=True)
