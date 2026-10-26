"""Amplitude of the cell-scale specks left in the haze after the fix.

usage: python specks.py <name>=<clip.mkv> ...
Patch: native x 1100..1820, y 490..650 (the far haze / bay band where the d2 x20 sheets show specks).
Luma, 4x4 box (removes codec grain, keeps 12 px froxel cells), temporal d2 per pixel.
Prints |d2| percentiles (code values), share of samples over 0.5 and 1.0, and the rms.
"""
import subprocess, sys, json
import numpy as np

FF = "C:/ffmpeg/bin/ffmpeg.exe"
X0, Y0, W, H = 1100, 490, 720, 160
for p in sys.argv[1:]:
    name, src = p.split("=", 1)
    r = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-vf",
                        f"crop={W}:{H}:{X0}:{Y0},format=gray", "-f", "rawvideo", "-"], capture_output=True, check=True)
    x = np.frombuffer(r.stdout, np.uint8).reshape(-1, H, W).astype(np.float32)
    x = x.reshape(len(x), H // 4, 4, W // 4, 4).mean(axis=(2, 4))
    d = np.abs(x[1:-1] - 0.5 * (x[:-2] + x[2:]))
    print(name, json.dumps(dict(rms=round(float(np.sqrt((d ** 2).mean())), 3),
                                p50=round(float(np.percentile(d, 50)), 3), p99=round(float(np.percentile(d, 99)), 3),
                                p999=round(float(np.percentile(d, 99.9)), 3), max=round(float(d.max()), 3),
                                over05=round(float((d > 0.5).mean()), 5), over1=round(float((d > 1.0).mean()), 6))),
          flush=True)
