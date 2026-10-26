"""Held-camera check (real-time recordings, camera still): anything that changes in the fog is an artefact.
Per region: CHG = rms of consecutive frame differences band-passed 2-16 px (blotches), SLOW = rms of the
difference between frames 8 apart (2-16 px band) (slow boil / drift), B60 = rms over time of 60x30 px block
means (high-passed in time), and travel = NCC of consecutive band-passed differences, best |lag| >= 3 px
minus the core (|lag| <= 1.5), with its lag.
usage: python held.py label=clip.mkv ...
"""
import sys, os
import numpy as np
from scipy import ndimage
sys.path.insert(0, os.path.dirname(__file__))
from cache import decode
from common import bandpass

REG = {"farhill": (500, 950, 495, 560), "farvalley": (1150, 1290, 520, 585), "valley": (1140, 1250, 600, 700),
       "bay": (1450, 1880, 640, 740), "haze": (1200, 1900, 470, 555)}
L = 16


def ncc(X, Y):
    h, w = X.shape
    H, W = h + L, w + L
    c = np.fft.irfft2(np.conj(np.fft.rfft2(X, (H, W))) * np.fft.rfft2(Y, (H, W)), (H, W))
    c = np.roll(np.roll(c, L, 0), L, 1)[:2 * L + 1, :2 * L + 1]
    return c / np.sqrt((X * X).sum() * (Y * Y).sum() + 1e-12)


ly, lx = np.mgrid[-L:L + 1, -L:L + 1]
rr = np.hypot(ly, lx)
for spec in sys.argv[1:]:
    lab, path = spec.split("=", 1)
    a = decode(path, 460, 750, 0, 1920).astype(np.float32)  # rgb False -> luma float16 -> float32
    T = a.shape[0]
    a = a[10:]
    print(f"== {lab} ({T} frames, using {a.shape[0]})")
    for k, (x0, x1, y0, y1) in REG.items():
        S = a[:, y0 - 460:y1 - 460, x0:x1]
        D = S[1:] - S[:-1]
        Db = np.stack([bandpass(d, 1.0, 8.0) for d in D])
        chg = Db.std()
        D8 = np.stack([bandpass(d, 1.0, 8.0) for d in (S[8:] - S[:-8])])
        win = np.outer(np.hanning(Db.shape[1]), np.hanning(Db.shape[2]))
        C = np.zeros((2 * L + 1, 2 * L + 1))
        for i in range(len(Db) - 1):
            X = Db[i] * win; Y = Db[i + 1] * win
            C += ncc(X - X.mean(), Y - Y.mean())
        C /= len(Db) - 1
        core = C[rr <= 1.5].max()
        j = np.argmax(np.where(rr >= 3, C, -9))
        hh, ww = (S.shape[1] // 30) * 30, (S.shape[2] // 60) * 60
        B = S[:, :hh, :ww].reshape(S.shape[0], hh // 30, 30, ww // 60, 60).mean((2, 4))
        Bh = B - ndimage.uniform_filter1d(B, 15, axis=0, mode="nearest")
        print(f"  {k:9s} CHG {chg:.3f} SLOW8 {D8.std():.3f} B60 {Bh.std():.3f} | travel {max(0, C.flat[j] - max(core, 0)):.3f} at ({lx.flat[j]},{ly.flat[j]}) core {core:+.3f}", flush=True)
