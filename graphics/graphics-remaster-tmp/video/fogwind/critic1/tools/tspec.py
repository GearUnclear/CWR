"""Temporal spectra of the fog at fixed SCENE points (scene-stabilised rows), per region and band.

For each row segment, each scene point's 128-frame series (linear trend removed, Hann window) is
Fourier transformed; the power is averaged over points and reported as rms code values per band of
temporal frequency at 60 fps: 0.5-2 Hz (slow swell), 2-6 Hz (pumping, swimming), 6-15 Hz, 15-30 Hz
(fizz, encode). Also the same for the segment MEAN (a whole-region pump) and for 3 px and 24 px
spatial bands of the series (fine grain vs blotches).
usage: python tspec.py label... (prints a table)
"""
import sys
import numpy as np
from scipy import ndimage
from common import load, Y0, SCRATCH

SEG = {  # name: (row y, track, screen x0, x1 at the window centre)
    "haze520": (520, "farvalley", 1300, 1850),
    "farhill520": (520, "farhill", 520, 940),
    "farvalley600": (600, "farvalley", 1200, 1480),
    "bay660": (660, "bay", 1500, 1850),
    "bay700": (700, "bay", 1480, 1850),
    "lowmist790": (790, "lowmist", 1560, 1850),
}
WINS = [(72, 200), (200, 328), (328, 456)]
BANDS = [(0.5, 2), (2, 6), (6, 15), (15, 30)]


def track(name):
    tr = np.load(f"{SCRATCH}/camtrack.npy", allow_pickle=True).item()
    return np.cumsum(tr["reg"][name], 0)


def seg_series(a, y, c, x0, x1, t0, t1):
    tc = (t0 + t1) // 2
    xs = np.arange(x0, x1, dtype=np.float64) - c[tc, 1]  # scene x at frame 0
    out = np.zeros((t1 - t0, len(xs)), np.float32)
    for i, t in enumerate(range(t0, t1)):
        sy = y + (c[t, 0] - c[tc, 0])
        yi = int(np.floor(sy)); fy = sy - yi
        row = (1 - fy) * a[t, yi - Y0].astype(np.float32) + fy * a[t, yi + 1 - Y0].astype(np.float32)
        out[i] = np.interp(xs + c[t, 1], np.arange(1920), row)
    return out


def spec(S):
    n = S.shape[0]
    t = np.arange(n)
    # remove a linear trend per point
    A = np.vstack([t, np.ones(n)]).T
    coef, *_ = np.linalg.lstsq(A, S, rcond=None)
    S = S - A @ coef
    w = np.hanning(n)[:, None]
    F = np.fft.rfft(S * w, axis=0)
    P = (np.abs(F) ** 2) / (w ** 2).sum()  # per-bin power such that sum over bins ~ variance * 2
    f = np.fft.rfftfreq(n, 1 / 60.0)
    res = []
    for lo, hi in BANDS:
        m = (f >= lo) & (f < hi)
        res.append(np.sqrt(2 * P[m].sum(0).mean() / n))  # Parseval: band variance = (2/n) sum |F|^2 / sum w^2
    return res


labs = sys.argv[1:]
print("rms code values per temporal band " + " ".join(f"{lo}-{hi}Hz" for lo, hi in BANDS) + " | region MEAN 2-6Hz | 24px-blur 2-6Hz | fine(<3px) 6-30Hz")
for name, (y, tn, x0, x1) in SEG.items():
    c = track(tn)
    print(f"--- {name}")
    for lab in labs:
        a = load(lab)
        acc = []
        for (t0, t1) in WINS:
            S = seg_series(a, y, c, x0, x1, t0, t1)
            b = spec(S)
            m = spec(S.mean(1, keepdims=True))
            blur = ndimage.gaussian_filter1d(S, 8.0, axis=1)
            bb = spec(blur)
            fine = S - ndimage.gaussian_filter1d(S, 1.0, axis=1)
            fb = spec(fine)
            acc.append(b + [m[1], bb[1], np.sqrt(fb[2] ** 2 + fb[3] ** 2)])
        acc = np.array(acc).mean(0)
        print(f"  {lab:5s} " + " ".join(f"{v:.3f}" for v in acc[:4]) + f" | {acc[4]:.3f} | {acc[5]:.3f} | {acc[6]:.3f}", flush=True)
