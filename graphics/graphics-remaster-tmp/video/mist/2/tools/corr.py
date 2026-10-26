"""Per-frame band flicker of a clip joined with the per-shot engine log fields.

usage: python corr.py <clip.mkv> <frames.json> [--from A --to B]
Prints the correlation of the per-frame band |d2| (mean over the mist band blocks)
with dframe (engine frames rendered since the last shot) and the camera meter fields.
"""
import argparse, json, subprocess
import numpy as np

W, H, B = 480, 270, 15
FF = "C:/ffmpeg/bin/ffmpeg.exe"


def load(src):
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src,
                        "-vf", f"scale={W}:{H}:flags=area,format=gray", "-f", "rawvideo", "-"],
                       capture_output=True, check=True)
    return np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W).astype(np.float32)


ap = argparse.ArgumentParser()
ap.add_argument("clip")
ap.add_argument("frames")
ap.add_argument("--from", dest="a", type=int, default=0)
ap.add_argument("--to", dest="b", type=int, default=None)
o = ap.parse_args()
x = load(o.clip)
gy, gx = H // B, W // B
blk = x[:, :gy * B, :gx * B].reshape(len(x), gy, B, gx, B).mean(axis=(2, 4))
band = blk[:, 8:14, 5:32]
d1 = band[1:] - band[:-1]          # d1[i] = frame i+1 minus frame i (0-based)
d2 = band[1:-1] - 0.5 * (band[:-2] + band[2:])  # d2[i] for frame i+1
rows = json.load(open(o.frames))
n = min(len(rows), len(x))
f_d2 = np.zeros(n)
f_d2[1:n - 1] = np.abs(d2[:n - 2]).reshape(n - 2, -1).mean(axis=1)
f_d1 = np.zeros(n)
f_d1[1:n] = np.abs(d1[:n - 1]).reshape(n - 1, -1).mean(axis=1)
a, b = o.a, o.b if o.b else n
sel = slice(max(a, 1), min(b, n - 1))
print("frames", n, "window", sel)
print("mean band |d2| %.3f  mean band |d1| %.3f" % (f_d2[sel].mean(), f_d1[sel].mean()))
keys = ["dframe", "wall", "ev", "target", "pre", "mean", "high", "wb", "sky", "ring", "ringw", "free", "skytop", "gpu"]
for k in keys:
    v = np.array([r.get(k) if r.get(k) is not None else np.nan for r in rows[:n]], float)
    vv = v[sel]
    if np.all(np.isnan(vv)) or np.nanstd(vv) == 0:
        print(f"{k:8s} constant/na  {np.nanmean(vv) if not np.all(np.isnan(vv)) else 'na'}")
        continue
    m = ~np.isnan(vv)
    c2 = np.corrcoef(vv[m], f_d2[sel][m])[0, 1]
    c1 = np.corrcoef(vv[m], f_d1[sel][m])[0, 1]
    dv = np.abs(np.diff(v))
    dvs = np.zeros(n); dvs[1:] = dv
    dvs = dvs[sel]
    m2 = ~np.isnan(dvs)
    cd = np.corrcoef(dvs[m2], f_d1[sel][m2])[0, 1] if np.nanstd(dvs) > 0 else float('nan')
    print(f"{k:8s} mean {np.nanmean(vv):9.4f} std {np.nanstd(vv):8.4f}  corr(v,|d2|) {c2:+.3f} corr(v,|d1|) {c1:+.3f} corr(|dv|,|d1|) {cd:+.3f}")
# frames with dframe values: mean d1 by dframe
dfr = np.array([r.get("dframe", -1) for r in rows[:n]])
for v in sorted(set(dfr[sel])):
    mm = dfr[sel] == v
    print(f"dframe={v}: n={mm.sum()} mean|d1|={f_d1[sel][mm].mean():.3f} mean|d2|={f_d2[sel][mm].mean():.3f}")
top = np.argsort(f_d2)[-12:][::-1]
for i in top:
    r = rows[i]
    print(f"frame {i+1}: |d2| {f_d2[i]:.3f} |d1| {f_d1[i]:.3f} dframe {r.get('dframe')} ev {r.get('ev')} target {r.get('target')} mean {r.get('mean')} ring {r.get('ring')}")
