"""Agent 5: a numeric model of the fog history's residual under different jitter sequences.

A 2D lattice of cells (W x H) and 2 slices; each cell's medium inside its slice is a THIN LAYER
(g(u) = 1 for u in [a, a + w), else 0, u = depth position in the slice), the worst case for the depth
jitter. The history is an EMA (share 0.12) read at the cell centre (agent 6), or a leaky sum (read at the
jittered point, the shipped form, modelled as s_n = o_n + 0.88 s_{n-1} for a linear medium - here we only
model the EMA form for the sampled value). We print, per scheme:
  rms   per-cell residual rms (the cell's mottle)
  slow  share of the residual power below 0.05 cycles/frame (3 Hz at 60 fps): pump and wind live here
  blk   rms of the 2x2x2 block mean of the residual (what survives trilinear + column integration)
  blk_slow  the block mean's slow power share
usage: python model5.py
"""
import numpy as np

W, H, S, T = 32, 32, 2, 2048
ALPHA = 0.12
rng = np.random.default_rng(1)
import sys
if len(sys.argv) > 1 and sys.argv[1] == "smooth":
    _y, _x, _s = np.meshgrid(np.arange(H), np.arange(W), np.arange(S), indexing="ij")
    a = 0.45 + 0.4 * np.sin(2 * np.pi * _x / 40.0 + 0.3 * _s) * np.cos(2 * np.pi * _y / 50.0)
else:
    a = rng.uniform(0, 0.9, size=(H, W, S))
w = 0.1


def g(u):
    return ((u >= a) & (u < a + w)).astype(np.float64)


def ign(x, y):
    return np.mod(52.9829189 * np.mod(0.06711056 * x + 0.00583715 * y, 1.0), 1.0)


yy, xx, ss = np.meshgrid(np.arange(H), np.arange(W), np.arange(S), indexing="ij")
VDC = np.array([0, 4, 2, 6, 1, 5, 3, 7])
dz_hash = rng.uniform(0, 1, size=(H, W, S))


def offsets(scheme, t):
    if scheme == "global":  # frame-wide golden ratio (original bug)
        return np.full((H, W, S), np.mod(0.6180339887 * t, 1.0))
    if scheme == "ign":  # d91a08e0
        return np.mod(0.6180339887 * t + ign(xx, yy) + 0.618034 * ss, 1.0)
    if scheme == "white":  # agent 2's A: white hash rotation of the golden sequence
        return np.mod(0.6180339887 * t + dz_hash, 1.0)
    if scheme == "c8":
        iz = (t + (xx & 1) + 2 * (yy & 1) + 4 * (ss & 1)) & 7
        return (VDC[iz] + dz_hash) / 8.0
    if scheme == "c8z":
        iz = (t + (xx & 1) + 2 * (yy & 1) + 4 * (ss & 1)) & 7
        return (VDC[iz] + 0.5) / 8.0
    raise ValueError(scheme)


def run(scheme):
    h = None
    res = np.zeros((T, H, W, S))
    for t in range(T):
        x = g(offsets(scheme, t))
        h = x if h is None else (1 - ALPHA) * h + ALPHA * x
        res[t] = h
    res = res[256:]
    res = res - res.mean(axis=0)  # residual about each cell's own long-run mean
    blk = res.reshape(res.shape[0], H // 2, 2, W // 2, 2, S // 2, 2).mean(axis=(2, 4, 6))

    def slow(r):
        f = np.fft.rfftfreq(r.shape[0])
        p = (np.abs(np.fft.rfft(r, axis=0)) ** 2).reshape(len(f), -1).sum(axis=1)
        return p[(f > 0) & (f < 0.05)].sum() / max(p[f > 0].sum(), 1e-30)

    return np.sqrt((res ** 2).mean()), slow(res), np.sqrt((blk ** 2).mean()), slow(blk)


print(f"{'scheme':8s} {'rms':>7s} {'slow':>6s} {'blk':>7s} {'blk_slow':>8s}   (thin layer w={w}, EMA {ALPHA}, centre read)")
for sc in ["global", "ign", "white", "c8", "c8z"]:
    r, sl, b, bs = run(sc)
    print(f"{sc:8s} {r:7.4f} {sl:6.3f} {b:7.4f} {bs:8.3f}")
