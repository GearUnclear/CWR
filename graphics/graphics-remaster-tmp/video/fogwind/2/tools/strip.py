"""Agent 2: consecutive-frame strips of a static probe's residual (frame - temporal mean), enhanced.

usage: python strip.py out.png <label>=<clip.mkv> [...] [--t0 50] [--n 6] [--gain 12] [--box x,y,w,h]
Each clip becomes one row of n consecutive frames, 2x crop of the box (default the valley mist and the bay).
Also writes out_raw.png: the same crops of the plain frames (gain 1) for the look.
"""
import argparse, subprocess
import numpy as np
from PIL import Image, ImageDraw

FF = "C:/ffmpeg/bin/ffmpeg.exe"


def decode(src, box):
    x, y, w, h = box
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-vf", f"crop={w}:{h}:{x}:{y},format=gray",
                        "-f", "rawvideo", "-"], capture_output=True, check=True)
    return np.frombuffer(p.stdout, np.uint8).reshape(-1, h, w).astype(np.float32)


ap = argparse.ArgumentParser()
ap.add_argument("out")
ap.add_argument("clips", nargs="+")
ap.add_argument("--t0", type=int, default=50)
ap.add_argument("--n", type=int, default=6)
ap.add_argument("--gain", type=float, default=12.0)
ap.add_argument("--box", default="1060,560,300,140")
o = ap.parse_args()
box = [int(v) for v in o.box.split(",")]
w, h = box[2] * 2, box[3] * 2
rows, raws = [], []
for c in o.clips:
    label, src = c.split("=", 1)
    x = decode(src, box)
    m = x[15:].mean(axis=0)
    rows.append((label, [(x[t] - m) * o.gain + 128 for t in range(o.t0, o.t0 + o.n)]))
    raws.append((label, [x[t] for t in range(o.t0, o.t0 + o.n)]))
for suffix, data in (("", rows), ("_raw", raws)):
    img = Image.new("L", (w * o.n, (h + 16) * len(data)), 0)
    d = ImageDraw.Draw(img)
    for r, (label, frames) in enumerate(data):
        d.text((4, r * (h + 16) + 2), f"{label}  frames {o.t0}..{o.t0 + o.n - 1}" + ("" if suffix else f"  (frame - mean) x{o.gain:g}"), fill=255)
        for i, f in enumerate(frames):
            im = Image.fromarray(np.clip(f, 0, 255).astype(np.uint8)).resize((w, h), Image.NEAREST)
            img.paste(im, (i * w, r * (h + 16) + 16))
    img.save(o.out.replace(".png", suffix + ".png"))
print("ok")
