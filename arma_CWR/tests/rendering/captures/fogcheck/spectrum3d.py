#!/usr/bin/env python
"""3D (y, x, t) power spectrum of the fog's fluctuation on a STATIC-camera clip (fogwind, agent 1).

usage: python spectrum3d.py <static.mkv> [--from 15] [--to 0] [--region y0,y1,x0,x1 (fractions)] [--top 12] [--png out.png]

The per-pixel temporal mean is removed (static camera: what remains is the fog's own change plus the
animated props). The 3D FFT's strongest components are listed with their spatial frequency (cycles per
froxel cell of 12 px, x right, y DOWN), temporal frequency (cycles/frame) and the phase velocity
v = nu k / |k|^2 (cells/frame; the direction a wave crest travels). Coherent drift = peaks off nu = 0
lying on one plane nu = -k.v through the origin.
Also prints the fraction of the fluctuation energy in |nu| < 0.05 (slow: drift / swim) and |nu| >= 0.05.
"""
import argparse, os, sys
import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fogmotion as fm

CELL = 12.0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("clip")
    ap.add_argument("--from", dest="a", type=int, default=15)
    ap.add_argument("--to", dest="b", type=int, default=0)
    ap.add_argument("--region", default="0.47,0.70,0.66,0.97")
    ap.add_argument("--top", type=int, default=12)
    ap.add_argument("--png", default="")
    o = ap.parse_args()
    y0, y1, x0, x1 = (float(v) for v in o.region.split(","))
    box = (int(y0 * 1080), int(y1 * 1080), int(x0 * 1920), int(x1 * 1920))
    x = fm.decode(o.clip, o.a, o.b, box)
    T, H, W = x.shape
    f = x - x.mean(axis=0, keepdims=True)
    # remove each frame's spatial mean (global pumping) to look at patterns
    glob_rms = float(np.sqrt((f.mean(axis=(1, 2)) ** 2).mean()))
    f = f - f.mean(axis=(1, 2), keepdims=True)
    win = np.hanning(T)[:, None, None] * np.hanning(H)[None, :, None] * np.hanning(W)[None, None, :]
    F = np.fft.fftn(f * win)
    P = np.abs(F) ** 2
    nu = np.fft.fftfreq(T)
    ky = np.fft.fftfreq(H) * CELL
    kx = np.fft.fftfreq(W) * CELL
    NU, KY, KX = np.meshgrid(nu, ky, kx, indexing="ij")
    tot = P.sum()
    # ignore the lowest spatial frequencies (< 1/10 cell^-1: illumination / props) for the drift census
    kk = np.hypot(KX, KY)
    sel = kk > 0.08
    slow = P[sel & (np.abs(NU) < 0.05) & (np.abs(NU) > 0)].sum() / P[sel].sum()
    still = P[sel & (NU == 0)].sum() / P[sel].sum()
    print(f"{o.clip}: frames {T}, region {H}x{W}, fluct rms {np.sqrt((f ** 2).mean()):.3f}, global (frame mean) rms {glob_rms:.3f}")
    print(f"  energy share at k > 0.08 cyc/cell: nu=0 {still:.3f}, 0<|nu|<0.05 (slow drift) {slow:.3f}, |nu|>=0.05 {1 - slow - still:.3f}")
    # energy-weighted velocity census over nu != 0, k > 0.08
    m = sel & (NU != 0)
    w = P[m]
    vx = NU[m] * KX[m] / kk[m] ** 2
    vy = NU[m] * KY[m] / kk[m] ** 2
    # positive frequency convention: numpy fft exp(-i(kx x + w t)) -> a wave travelling +x has kx, nu of opposite sign
    vx, vy = -vx, -vy
    order = np.argsort(P[m])[::-1][:o.top]
    print("  top components (k in cycles/cell, nu in cycles/frame, phase velocity in cells/frame):")
    idx = np.argwhere(m)
    for i in order:
        t, iy, ix = idx[i]
        print(f"    kx {kx[ix]:+.3f} ky {ky[iy]:+.3f} |k| {np.hypot(kx[ix], ky[iy]):.3f} nu {nu[t]:+.3f}  v=({vx[i]:+.2f},{vy[i]:+.2f})  P/tot {P[t, iy, ix] / tot:.4f}")
    # energy-weighted histogram of vx (cells/frame) for |nu| > 0
    hist, edges = np.histogram(np.clip(vx, -3, 3), bins=24, range=(-3, 3), weights=w)
    hist = hist / hist.sum()
    print("  energy-weighted phase velocity vx histogram (cells/frame):")
    print("   " + " ".join(f"{(edges[i] + edges[i + 1]) / 2:+.2f}:{hist[i]:.2f}" for i in range(len(hist)) if hist[i] > 0.02))
    if o.png:
        from PIL import Image
        # kx-nu plane, summed over ky
        S = np.fft.fftshift(P.sum(axis=1), axes=(0, 1))
        S = np.log10(S + 1e-9)
        S = (S - np.percentile(S, 50)) / (S.max() - np.percentile(S, 50) + 1e-9)
        img = (np.clip(S, 0, 1) * 255).astype(np.uint8)
        Image.fromarray(img).resize((img.shape[1], img.shape[0] * 3), Image.NEAREST).save(o.png)


if __name__ == "__main__":
    main()
