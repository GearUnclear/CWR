"""Mirror of the froxel injection + history, to calibrate the unit test's bars.

Grid N x N cells, one slice. Medium f(x, y, z): x, y in cells (world-fixed), z in slices
relative to the slice centre. The camera pans in x by `pan` cells per frame, so the cell
(i, j) at frame t looks at world x = i + pan * t.
A scheme gives the tap offsets per cell and frame (list of (ox, oy, oz, weight)).
History: h_t(i) = 0.12 v_t(i) + 0.88 h_{t-1} reprojected (linear interpolation in x by pan).
Residual r_t = h_t - truth_t (truth = the cell mean of the medium at the current pose,
from a 4x4x4 midpoint quadrature), d_t = r_t - r_{t-1}.
pump   = std over frames of the 5x5 block mean of r_t
travel = best averaged correlation of d_t vs d_{t-1} at a nonzero lag minus that at lag 0
held   = max |d_t| with pan 0 (bit-still check)
"""
import numpy as np

N = 48
SH = 0.12


def fract(x):
    return x - np.floor(x)


def medium(x, y, z):
    g = 1.0 + 0.5 * np.sin(0.21 * x) * np.cos(0.17 * y)
    return (np.sin(2 * np.pi * (0.083 * x + 0.047 * y)) + 0.6 * np.cos(2 * np.pi * (0.031 * x - 0.058 * y))
            + g * (0.9 * z + 0.4 * z * z))


I, J = np.meshgrid(np.arange(N, dtype=float), np.arange(N, dtype=float), indexing='xy')


def ign(x, y):
    return fract(52.9829189 * fract(x * 0.06711056 + y * 0.00583715))


def truth(t, pan):
    acc = np.zeros((N, N))
    q = (np.arange(4) + 0.5) / 4 - 0.5
    for ox in q:
        for oy in q:
            for oz in q:
                acc += medium(I + pan * t + ox, J + oy, oz)
    return acc / 64


def scheme_framewide(t):
    jz = fract(0.6180339887 * t) - 0.5
    jx = fract(0.7548776662 * t) - 0.5
    jy = fract(0.5698402910 * t) - 0.5
    return [(np.full((N, N), jx), np.full((N, N), jy), np.full((N, N), jz), 1.0)]


def scheme_ign(t, s=40):
    jz = fract(0.6180339887 * t)
    jx = fract(0.7548776662 * t) - 0.5
    jy = fract(0.5698402910 * t) - 0.5
    px, py = I + 0.5, J + 0.5
    oz = fract(jz + ign(px, py) + 0.618034 * s) - 0.5
    ox = fract(jx + 0.5 + ign(px + 17, py + 59)) - 0.5
    oy = fract(jy + 0.5 + ign(px + 83, py + 29)) - 0.5
    return [(ox, oy, oz, 1.0)]


def scheme_white(t):
    rng = np.random.default_rng(12345)
    hz, hx, hy = rng.random((3, N, N))
    jz = fract(0.6180339887 * t)
    jx = fract(0.7548776662 * t)
    jy = fract(0.5698402910 * t)
    return [(fract(jx + hx) - 0.5, fract(jy + hy) - 0.5, fract(jz + hz) - 0.5, 1.0)]


TAPZ = [0.25, 0.75]
TAPXY = [(-0.25, -0.25), (0.25, 0.25)]


def scheme_fixed(t):
    return [(np.full((N, N), TAPXY[i][0]), np.full((N, N), TAPXY[i][1]), np.full((N, N), TAPZ[i] - 0.5), 0.5)
            for i in range(2)]


def run(scheme, pan, frames=300, warm=100, lag=3):
    h = None
    prev_r = None
    prev_d = None
    blocks = []
    lagsum = np.zeros((2 * lag + 1, 2 * lag + 1))
    nl = 0
    held = 0.0
    for t in range(frames):
        v = np.zeros((N, N))
        for ox, oy, oz, w in scheme(t):
            v += w * medium(I + pan * t + ox, J + oy, oz)
        if h is None:
            h = v.copy()
        else:
            # reproject: the cell i now sees world i + pan t; last frame's cell at i + pan
            xs = np.arange(N) + pan
            x0 = np.clip(np.floor(xs).astype(int), 0, N - 1)
            x1 = np.clip(x0 + 1, 0, N - 1)
            fx = xs - np.floor(xs)
            hp = h[:, x0] * (1 - fx) + h[:, x1] * fx
            h = hp + SH * (v - hp)
        r = h - truth(t, pan)
        if prev_r is not None:
            d = r - prev_r
            if t >= warm:
                held = max(held, np.abs(d[:, 4:-4]).max())
                if prev_d is not None:
                    c = lag + 1
                    for dy in range(-lag, lag + 1):
                        for dx in range(-lag, lag + 1):
                            a = d[c:N - c, c:N - c - 4]
                            b = prev_d[c + dy:N - c + dy, c + dx:N - c - 4 + dx]
                            a = a - a.mean()
                            b = b - b.mean()
                            den = np.sqrt((a * a).sum() * (b * b).sum())
                            lagsum[dy + lag, dx + lag] += 0.0 if den < 1e-24 else (a * b).sum() / den
                    nl += 1
            prev_d = d
        prev_r = r
        if t >= warm:
            blocks.append(r[20:25, 20:25].mean())
    blocks = np.array(blocks)
    lagm = lagsum / max(nl, 1)
    c0 = lagm[lag, lag]
    lagm2 = lagm.copy()
    lagm2[lag, lag] = -9
    k = np.unravel_index(np.argmax(lagm2), lagm2.shape)
    best = lagm2[k]
    return dict(pump=blocks.std(), travel=best - c0, best=best, c0=c0, lagbest=(k[1] - lag, k[0] - lag), held=held)


if __name__ == '__main__':
    for name, sc in [('framewide', scheme_framewide), ('ign', scheme_ign), ('white', scheme_white), ('fixed', scheme_fixed)]:
        for pan in (0.0, 0.075):
            r = run(sc, pan)
            print(f"{name:10s} pan {pan:5.3f} pump {r['pump']:.5f} travel {r['travel']:.3f} best {r['best']:.3f} at {r['lagbest']} c0 {r['c0']:.3f} held {r['held']:.2e}")
