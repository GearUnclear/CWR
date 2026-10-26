"""Does the fog's global per-frame jitter explain the mist-band flicker?

usage: python jitfit.py <clip.mkv | frames_dir> <frames.json> [--from A] [--lag L0,L1] [--share 0.12]
For every shot the engine frame counter n (frames.json, from capFrameLog) is known.
The froxel injection uses ONE jitter for all cells per rendered frame,
jz = fract(0.618034 seq), jx = fract(0.754878 seq) - .5, jy = fract(0.569840 seq) - .5,
blended into the history with share 0.12. With seq = n - lag, the history's
jitter state is W_c(n) = sum_i s (1-s)^i (j_c(n-i) - mean). We regress the band
blocks' brightness residual (brightness minus a 9-frame running mean, so the pan
and slow drift are removed) on the 3 W series plus nonlinear terms, for each lag
in a range, and print the best lags and the explained share (R^2).
"""
import argparse, glob, json, os, subprocess
import numpy as np

W, H, B = 480, 270, 15
FF = "C:/ffmpeg/bin/ffmpeg.exe"


def load(src):
    if os.path.isdir(src):
        from PIL import Image
        files = sorted(glob.glob(os.path.join(src, "*.png")))
        return np.stack([np.asarray(Image.open(f).convert("L").resize((W, H), Image.BOX), np.float32) for f in files])
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src,
                        "-vf", f"scale={W}:{H}:flags=area,format=gray", "-f", "rawvideo", "-"],
                       capture_output=True, check=True)
    return np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W).astype(np.float32)


def frac(v):
    return v - np.floor(v)


ap = argparse.ArgumentParser()
ap.add_argument("src")
ap.add_argument("frames")
ap.add_argument("--from", dest="a", type=int, default=0)
ap.add_argument("--lag", default="-3000,3000")
ap.add_argument("--share", type=float, default=0.12)
ap.add_argument("--hist", type=int, default=60)
o = ap.parse_args()
x = load(o.src)
rows = json.load(open(o.frames))
n = min(len(x), len(rows))
x = x[:n]
fr = np.array([r["frame"] for r in rows[:n]], np.int64)
gy, gx = H // B, W // B
blk = x[:, :gy * B, :gx * B].reshape(n, gy, B, gx, B).mean(axis=(2, 4))
band = blk[:, 8:14, 5:32].reshape(n, -1)
k = 9
pad = np.pad(band, ((k // 2, k // 2), (0, 0)), mode="edge")
run = np.stack([pad[i:i + n] for i in range(k)]).mean(axis=0)
res = band - run
sel = np.arange(max(o.a, k), n - k)
Y = res[sel]
Yc = Y - Y.mean(axis=0)
tot = (Yc ** 2).sum()
s = o.share
wts = s * (1 - s) ** np.arange(o.hist)
l0, l1 = (int(v) for v in o.lag.split(","))
best = []
for lag in range(l0, l1 + 1):
    seq = fr[sel][:, None] - lag - np.arange(o.hist)[None, :]
    seq = np.maximum(seq, 0).astype(np.float64)
    jz = frac(0.6180339887 * seq) - 0.5
    jx = frac(0.7548776662 * seq) - 0.5
    jy = frac(0.5698402910 * seq) - 0.5
    feats = []
    for j in (jz, jx, jy):
        feats.append((j * wts).sum(axis=1))
        feats.append(((j ** 2 - 1 / 12) * wts).sum(axis=1))
    X = np.stack(feats, axis=1)
    X = np.concatenate([X - X.mean(axis=0), np.ones((len(X), 1))], axis=1)
    coef, *_ = np.linalg.lstsq(X, Yc, rcond=None)
    r2 = 1 - ((Yc - X @ coef) ** 2).sum() / tot
    best.append((r2, lag))
best.sort(reverse=True)
r2s = np.array([b[0] for b in best])
print(f"shots used {len(sel)}, blocks {Y.shape[1]}, lags {l0}..{l1}")
print("best lags (R^2, lag):", [(round(a, 3), b) for a, b in best[:8]])
print("R^2 over all lags: median %.3f p99 %.3f max %.3f" % (np.median(r2s), np.percentile(r2s, 99), r2s.max()))
