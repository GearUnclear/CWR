"""Agent 2: the look. Temporal mean frame (frames --from..) of clips in RGB at 960x540; prints per-region mean RGB
and the difference against the first clip; writes a diff heat map (b - a) x gain + 128 per channel mean, and a
side-by-side of the mean frames.
usage: python lookdiff.py out_prefix ref.mkv other.mkv [...] [--from 15] [--gain 16]
"""
import argparse, subprocess
import numpy as np
from PIL import Image

FF = "C:/ffmpeg/bin/ffmpeg.exe"
REG = {"bay": (0.56, 0.70, 0.72, 0.97), "valley": (0.54, 0.64, 0.58, 0.68), "farhill": (0.45, 0.53, 0.25, 0.55),
       "haze": (0.47, 0.55, 0.62, 0.97), "slope": (0.56, 0.68, 0.30, 0.48), "sky": (0.05, 0.30, 0.0, 1.0),
       "sunarea": (0.30, 0.50, 0.55, 0.95)}


def meanframe(src, a):
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-vf", "scale=960:540:flags=area,format=rgb24",
                        "-f", "rawvideo", "-"], capture_output=True, check=True)
    x = np.frombuffer(p.stdout, np.uint8).reshape(-1, 540, 960, 3)[a:]
    return x.astype(np.float64).mean(axis=0)


ap = argparse.ArgumentParser()
ap.add_argument("out")
ap.add_argument("clips", nargs="+")
ap.add_argument("--from", dest="a", type=int, default=15)
ap.add_argument("--gain", type=float, default=16.0)
o = ap.parse_args()
ms = [meanframe(c, o.a) for c in o.clips]
for c, m in zip(o.clips, ms):
    print(c)
    for k, (y0, y1, x0, x1) in REG.items():
        r = m[int(y0 * 540):int(y1 * 540), int(x0 * 960):int(x1 * 960)].reshape(-1, 3).mean(axis=0)
        r0 = ms[0][int(y0 * 540):int(y1 * 540), int(x0 * 960):int(x1 * 960)].reshape(-1, 3).mean(axis=0)
        print("  %-8s rgb %6.1f %6.1f %6.1f   d vs first %+5.2f %+5.2f %+5.2f" % (k, *r, *(r - r0)))
    d = (m - ms[0]).mean(axis=2)
    print("  whole frame |d| mean %.3f  p99 %.2f  max %.2f" % (np.abs(d).mean(), np.percentile(np.abs(d), 99), np.abs(d).max()))
for i in range(1, len(ms)):
    d = (ms[i] - ms[0]).mean(axis=2)
    Image.fromarray(np.clip(d * o.gain + 128, 0, 255).astype(np.uint8)).save(f"{o.out}_diff{i}.png")
Image.fromarray(np.concatenate([m.astype(np.uint8) for m in ms], axis=0)).save(f"{o.out}_means.png")
