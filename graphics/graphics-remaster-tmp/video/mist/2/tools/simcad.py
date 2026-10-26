"""Model: EMA (share s) of the fog's Weyl jitter, sampled at the capture cadence.
Prints the RMS of the second difference of the history residual W for
cadences D (rendered frames per captured shot), relative to D = 1 (real time at 60 fps)."""
import numpy as np
s = 0.12
N = 200000
n = np.arange(N, dtype=np.float64)
rng = np.random.default_rng(1)
def ema(j):
    w = np.zeros(N)
    acc = 0.0
    for i in range(N):
        acc = acc + s * (j[i] - acc)
        w[i] = acc
    return w
res = {}
J = {a: np.mod(a * n, 1.0) - 0.5 for a in (0.6180339887, 0.7548776662, 0.5698402910)}
Wz = ema(J[0.6180339887])
def d2rms(idx, w):
    x = w[idx]
    d2 = x[1:-1] - 0.5 * (x[:-2] + x[2:])
    return np.sqrt((d2 ** 2).mean())
base = None
for name, steps in [("D=1 (rt)", lambda m: np.ones(m, int)), ("D=7..10 (clip)", lambda m: rng.choice([7, 8, 8, 8, 8, 9, 10], m)),
                    ("D=8 fixed", lambda m: np.full(m, 8)), ("K=1", lambda m: rng.choice([7, 8, 8, 8, 8, 9, 10], m) + 1),
                    ("K=2", lambda m: rng.choice([7, 8, 8, 8, 8, 9, 10], m) + 2), ("K=4", lambda m: rng.choice([7, 8, 8, 8, 8, 9, 10], m) + 4),
                    ("K=8", lambda m: rng.choice([7, 8, 8, 8, 8, 9, 10], m) + 8), ("D=2", lambda m: np.full(m, 2)), ("D=3", lambda m: np.full(m, 3))]:
    m = N // 20
    idx = np.cumsum(steps(m))
    idx = idx[idx < N]
    r = d2rms(idx, Wz)
    base = base or r
    print(f"{name:16s} d2 rms of jz residual {r:.4f}  x{r / base:.2f} of real time   (residual rms {Wz[idx].std():.4f})")
# averaging M consecutive presented frames per shot (capture-only convergence)
for M in (4, 8, 16):
    idx = np.cumsum(rng.choice([7, 8, 8, 8, 8, 9, 10], N // 40) + M)
    idx = idx[idx < N]
    avg = np.array([Wz[i - M + 1:i + 1].mean() for i in idx])
    d2 = avg[1:-1] - 0.5 * (avg[:-2] + avg[2:])
    print(f"mean of {M:2d} frames d2 rms {np.sqrt((d2**2).mean()):.4f}  x{np.sqrt((d2**2).mean()) / base:.2f}")
