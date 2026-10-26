"""Motion-compensated shimmer metric for the sub-pixel pans.

    python shimmer_metric.py <run_dir> <view> --crop x0,y0,x1,y1 --variants a b c ...

A heading step of 0.025 deg moves the whole crop by a fraction of a pixel.
For each consecutive pair of pan frames the global (dx, dy) shift is measured
by upsampled phase correlation, frame i+1 is shifted back by it with a Fourier
(band-limited) shift, and the residual |f_i - shifted f_i+1| is what the move
could not explain: edges that crawl, leaves that pop, sparkle. Reported as the
mean residual (0-255) and the 99th percentile, plus the raw unaligned change.
A sharper image also raises the raw change; the residual is the fairer number.
"""
import argparse
from pathlib import Path

import numpy as np
from PIL import Image


def load(p, box):
    im = Image.open(p).convert("RGB").crop(box)
    return np.asarray(im).astype(np.float64)


def lum(a):
    return a @ np.array([0.2126, 0.7152, 0.0722])


def phase_shift(a, b, up=20):
    """Shift (dy, dx) such that b ~ a shifted by it, with 1/up px precision."""
    A = np.fft.fft2(a - a.mean())
    B = np.fft.fft2(b - b.mean())
    R = A * np.conj(B)
    R /= np.abs(R) + 1e-12
    r = np.fft.ifft2(R).real
    py, px = np.unravel_index(np.argmax(r), r.shape)
    h, w = a.shape
    py = py - h if py > h // 2 else py
    px = px - w if px > w // 2 else px
    best = (0.0, py, px)
    # refine on a local grid with matrix DFT
    ys = np.arange(-1.0, 1.0 + 1e-9, 1.0 / up) + py
    xs = np.arange(-1.0, 1.0 + 1e-9, 1.0 / up) + px
    ky = np.fft.fftfreq(h)
    kx = np.fft.fftfreq(w)
    Ey = np.exp(2j * np.pi * np.outer(ys, ky))
    Ex = np.exp(2j * np.pi * np.outer(kx, xs))
    corr = (Ey @ R @ Ex).real
    iy, ix = np.unravel_index(np.argmax(corr), corr.shape)
    return -ys[iy], -xs[ix]


def fourier_shift(img, dy, dx):
    h, w = img.shape[:2]
    ky = np.fft.fftfreq(h)[:, None]
    kx = np.fft.fftfreq(w)[None, :]
    ph = np.exp(-2j * np.pi * (ky * dy + kx * dx))
    out = np.empty_like(img)
    for c in range(img.shape[2]):
        out[..., c] = np.fft.ifft2(np.fft.fft2(img[..., c]) * ph).real
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("view")
    ap.add_argument("--crop", required=True)
    ap.add_argument("--variants", nargs="+", required=True)
    ap.add_argument("--frames", type=int, default=4)
    a = ap.parse_args()
    box = tuple(map(int, a.crop.split(",")))
    m = 8  # ignore a margin where the circular Fourier shift wraps
    for var in a.variants:
        fr = [load(Path(a.run) / f"{a.view}__{var}-{i}.png", box) for i in range(a.frames)]
        res, p99, raw, shifts = [], [], [], []
        for i in range(a.frames - 1):
            dy, dx = phase_shift(lum(fr[i]), lum(fr[i + 1]))
            # b ~ a shifted by (dy, dx): move b back by it
            back = fourier_shift(fr[i + 1], -dy, -dx)
            d = np.abs(fr[i] - back).mean(axis=2)[m:-m, m:-m]
            res.append(d.mean())
            p99.append(np.percentile(d, 99))
            raw.append(np.abs(fr[i] - fr[i + 1]).mean(axis=2)[m:-m, m:-m].mean())
            shifts.append((dy, dx))
        sh = ", ".join(f"({s[1]:+.2f},{s[0]:+.2f})" for s in shifts)
        print(f"{a.view} {var:12s} residual {np.mean(res):.3f}  p99 {np.mean(p99):.2f}  raw {np.mean(raw):.3f}  shifts(dx,dy) {sh}")


if __name__ == "__main__":
    main()
