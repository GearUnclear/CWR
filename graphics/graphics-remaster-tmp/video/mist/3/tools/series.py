"""Per-frame block-mean series for a few regions of a clip (grey 480x270), with d2.
usage: python series.py clip.mkv [--from a --to b] [--csv out.csv]"""
import argparse, subprocess, numpy as np, json
FF = "C:/ffmpeg/bin/ffmpeg.exe"
W, H = 480, 270
REG = {  # name: (y0,y1,x0,x1) as fractions
    "bay": (0.60, 0.70, 0.72, 0.95),
    "valley": (0.55, 0.65, 0.50, 0.65),
    "farhill": (0.45, 0.50, 0.25, 0.55),
    "haze_r": (0.48, 0.55, 0.62, 0.95),
    "sky": (0.10, 0.25, 0.10, 0.60),
    "ground": (0.80, 0.95, 0.30, 0.60),
}
def load(src):
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-vf", f"scale={W}:{H}:flags=area,format=gray",
                        "-f", "rawvideo", "-"], capture_output=True, check=True)
    return np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W).astype(np.float32)
ap = argparse.ArgumentParser(); ap.add_argument("src"); ap.add_argument("--from", dest="a", type=int, default=0)
ap.add_argument("--to", dest="b", type=int, default=None); ap.add_argument("--csv", default="")
o = ap.parse_args()
x = load(o.src)[o.a:o.b]
out = {}
for k, (y0, y1, x0, x1) in REG.items():
    s = x[:, int(y0*H):int(y1*H), int(x0*W):int(x1*W)].mean(axis=(1, 2))
    d2 = s[1:-1] - 0.5*(s[:-2]+s[2:])
    out[k] = s
    print(f"{k:8s} mean {s.mean():6.2f} d2rms {np.sqrt((d2**2).mean()):.3f} d2max {np.abs(d2).max():.3f} at {int(np.argmax(np.abs(d2)))+2+o.a}")
if o.csv:
    with open(o.csv, "w") as f:
        f.write("frame," + ",".join(out) + "\n")
        for i in range(len(x)):
            f.write(f"{i+1+o.a}," + ",".join(f"{out[k][i]:.3f}" for k in out) + "\n")
