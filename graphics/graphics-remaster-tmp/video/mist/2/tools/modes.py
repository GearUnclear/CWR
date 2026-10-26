"""How many common modes drive the mist-band flicker?

usage: python modes.py <clip.mkv | frames_dir> [--from A] [--to B]
Block means (15x15 px at 480x270) of the mist band, temporal second difference d2
per block (removes the smooth pan), then an SVD over blocks: the share of the d2
energy in the first few components. A global per-frame driver (one jitter vector
for the whole froxel volume, an exposure step) puts most of it in 1-3 components;
independent local noise spreads it evenly (each component about 1/blocks).
Also prints the RMS of d2 for the band and the correlation of the first
component's time series with the band mean d2.
"""
import argparse, glob, os, subprocess
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


ap = argparse.ArgumentParser()
ap.add_argument("src")
ap.add_argument("--from", dest="a", type=int, default=0)
ap.add_argument("--to", dest="b", type=int, default=None)
ap.add_argument("--save", default="")
o = ap.parse_args()
x = load(o.src)[o.a:o.b]
gy, gx = H // B, W // B
blk = x[:, :gy * B, :gx * B].reshape(len(x), gy, B, gx, B).mean(axis=(2, 4))
band = blk[:, 8:14, 5:32].reshape(len(x), -1)
d2 = band[1:-1] - 0.5 * (band[:-2] + band[2:])
rms = np.sqrt((d2 ** 2).mean())
u, s, vt = np.linalg.svd(d2 - d2.mean(axis=0), full_matrices=False)
e = s ** 2 / (s ** 2).sum()
print(f"frames {len(x)} blocks {band.shape[1]} band d2 rms {rms:.3f}")
print("energy share of components 1..6:", " ".join(f"{v:.3f}" for v in e[:6]), " (uniform would be %.3f)" % (1 / band.shape[1]))
# sign-consistency of the first component over the band
v0 = vt[0]
print("first component: share of blocks with the majority sign %.2f" % max((v0 > 0).mean(), (v0 < 0).mean()))
# mean pairwise correlation of block d2 series
c = np.corrcoef(d2.T)
iu = np.triu_indices(c.shape[0], 1)
print("mean pairwise corr of block d2 %.3f" % np.nanmean(c[iu]))
if o.save:
    np.savez(o.save, u=u[:, :6], s=s[:6], vt=vt[:6])
