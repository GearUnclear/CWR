#!/usr/bin/env python
"""Screen-locked pattern ("curtains", cell-column streaks) check on a MOVING clip (fogwind, agent 1).

usage: python curtain.py clip.mkv[@first] [more ...] [--n 105] [--png out.png]

The camera pans (about 1 px/frame), so the time mean of the frames IN SCREEN COORDINATES smears the
scene by the pan length (about 100 px) while anything locked to the screen (the 12 px froxel cell
grid: vertical cell-column streaks, view-locked sampling patterns) survives the average.
Per region: `streak` = rms of the x-direction band-pass (DoG 1..8 px along x, i.e. 12 px-ish columns)
of the time-mean image (codes); `lat12` = the share of the time-mean's x-spectrum power at the 12 px
lattice period (and its harmonic 6 px) relative to its neighbours (1 = nothing special, >> 1 = grid).
Compare with vanilla (no fog grid) and the jitter-frozen run (known curtains).
--png writes the time-mean x-band-pass of each clip, x40 around grey, stacked.
"""
import argparse, os, sys
import numpy as np
from scipy import ndimage
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fogmotion as fm

REG = {"lowsky": (0.30, 0.44, 0.62, 1.0), "haze": (0.45, 0.55, 0.62, 0.97), "bay": (0.56, 0.70, 0.72, 0.97),
       "farhill": (0.45, 0.53, 0.25, 0.55)}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("clips", nargs="+")
    ap.add_argument("--n", type=int, default=105)
    ap.add_argument("--png", default="")
    o = ap.parse_args()
    tiles = []
    for spec in o.clips:
        src, first = (spec.rsplit("@", 1) + ["15"])[:2] if "@" in spec else (spec, "15")
        first = int(first)
        line = []
        row_imgs = []
        for name, (y0, y1, x0, x1) in REG.items():
            box = (int(y0 * 1080), int(y1 * 1080), int(x0 * 1920), int(x1 * 1920))
            x = fm.decode(src, first, first + o.n, box)
            mu = x.mean(axis=0)
            bp = ndimage.gaussian_filter1d(mu, 1.0, axis=1) - ndimage.gaussian_filter1d(mu, 8.0, axis=1)
            bp = bp[8:-8, 16:-16]
            streak = float(np.sqrt((bp ** 2).mean()))
            # x spectrum of the time mean (rows averaged in power)
            r = mu[8:-8, 16:-16] - mu[8:-8, 16:-16].mean(axis=1, keepdims=True)
            W = r.shape[1]
            P = (np.abs(np.fft.rfft(r * np.hanning(W)[None, :], axis=1)) ** 2).mean(axis=0)
            f = np.fft.rfftfreq(W)
            def peak(period):
                i = int(round(W / period))
                nb = np.r_[P[max(i - 8, 1):i - 2], P[i + 3:i + 9]]
                return float(P[i - 1:i + 2].max() / (np.median(nb) + 1e-12))
            line.append(f"{name} streak {streak:.3f} lat12 {peak(12.0):.1f} lat6 {peak(6.0):.1f}")
            row_imgs.append(np.clip(128 + 40 * bp, 0, 255).astype(np.uint8))
        print(f"{src}@{first}: " + " | ".join(line))
        sys.stdout.flush()
        if o.png:
            w = max(t.shape[1] for t in row_imgs)
            tiles.append(np.concatenate([np.pad(t, ((0, 4), (0, w - t.shape[1]))) for t in row_imgs], axis=0))
    if o.png:
        from PIL import Image
        w = max(t.shape[1] for t in tiles)
        Image.fromarray(np.concatenate([np.pad(t, ((0, 10), (0, w - t.shape[1]))) for t in tiles], axis=0)).save(o.png)


if __name__ == "__main__":
    main()
