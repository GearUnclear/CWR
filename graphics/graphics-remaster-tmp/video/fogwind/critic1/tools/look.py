"""Look check: time-mean RGB (frames 30-560) of each clip at 480x270, region means, and a difference
image NEW minus ORIGINAL and NEW minus F1 (x8, grey = equal) to see where the look moved.
usage: python look.py outdir orig=... f1=... new=...
"""
import sys, subprocess
import numpy as np
from PIL import Image

out = sys.argv[1]
clips = dict(s.split("=", 1) for s in sys.argv[2:])
W, H = 480, 270
M = {}
for lab, path in clips.items():
    p = subprocess.run(["ffmpeg", "-v", "error", "-i", path, "-vf", f"scale={W}:{H}:flags=area", "-fps_mode", "passthrough",
                        "-f", "rawvideo", "-pix_fmt", "rgb24", "-"], capture_output=True, check=True)
    a = np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W, 3)[30:561].astype(np.float32)
    M[lab] = a.mean(0)
    np.save(f"{out}/tmean_{lab}.npy", M[lab])
# region boxes at 1/4 scale of the full frame (whole-clip means; the pan moves content, same for all ud clips)
R = {"sky_sun": (300, 480, 0, 100), "haze": (300, 480, 110, 140), "farhill": (120, 240, 120, 140),
     "valley": (280, 330, 145, 175), "bay": (360, 470, 155, 190), "nearhill": (60, 180, 150, 180), "rock": (100, 400, 220, 270)}
print("time-mean RGB per region, frames 30-560")
for k, (x0, x1, y0, y1) in R.items():
    s = f"{k:9s}"
    for lab in M:
        c = M[lab][y0:y1, x0:x1].reshape(-1, 3).mean(0)
        s += f" | {lab} {c[0]:6.1f} {c[1]:6.1f} {c[2]:6.1f}"
    print(s)
for ref in [r for r in ("orig", "f1") if r in M]:
    D = (M["new"] - M[ref]).mean(2)
    print(f"new - {ref}: whole-frame mean |d| {np.abs(M['new'] - M[ref]).mean():.3f}, p99 {np.percentile(np.abs(M['new'] - M[ref]), 99):.3f}, max {np.abs(M['new'] - M[ref]).max():.2f}")
    Image.fromarray(np.clip(D * 8 + 128, 0, 255).astype(np.uint8)).resize((960, 540), Image.NEAREST).save(f"{out}/look_new_minus_{ref}_x8.png")
