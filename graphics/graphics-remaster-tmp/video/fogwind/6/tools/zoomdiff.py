#!/usr/bin/env python
"""agent 6: 2x crop of one frame and the frame-to-frame difference x gain, per clip, stacked.
usage: python zoomdiff.py out.png --frame N --box x,y,w,h --gain G clip.mkv:LABEL ...
"""
import sys, subprocess, argparse
import numpy as np
from PIL import Image, ImageDraw

FF = "C:/ffmpeg/bin/ffmpeg.exe"
W, H = 1920, 1080


def grab(path, n, count):
    p = subprocess.run([FF, "-v", "error", "-i", path, "-vf", "select=gte(n\\,%d)" % n, "-vframes", str(count),
                        "-f", "rawvideo", "-pix_fmt", "rgb24", "-"], capture_output=True, check=True)
    a = np.frombuffer(p.stdout, np.uint8)
    return a.reshape(-1, H, W, 3).astype(np.float32)


ap = argparse.ArgumentParser()
ap.add_argument("out")
ap.add_argument("clips", nargs="+")
ap.add_argument("--frame", type=int, default=60)
ap.add_argument("--box", default="1180,500,560,220")
ap.add_argument("--gain", type=float, default=16)
o = ap.parse_args()
x, y, w, h = [int(v) for v in o.box.split(",")]
rows = []
for spec in o.clips:
    path, lab = spec.rsplit(":", 1)
    f = grab(path, o.frame, 3)
    c = f[0, y:y + h, x:x + w]
    d = (f[1] - f[0])[y:y + h, x:x + w].mean(2)
    d2 = (f[2] - 2 * f[1] + f[0])[y:y + h, x:x + w].mean(2)
    a = Image.fromarray(np.clip(c, 0, 255).astype(np.uint8)).resize((w * 2, h * 2), Image.NEAREST)
    b = Image.fromarray(np.clip(128 + d * o.gain, 0, 255).astype(np.uint8)).convert("RGB").resize((w * 2, h * 2), Image.NEAREST)
    row = Image.new("RGB", (w * 4 + 8, h * 2), (0, 0, 0))
    row.paste(a, (0, 0)); row.paste(b, (w * 2 + 8, 0))
    ImageDraw.Draw(row).text((6, 4), "%s f%d | diff f%d-f%d x%g (rms %.3f)" % (lab, o.frame, o.frame + 1, o.frame, o.gain, d.std()), fill=(255, 255, 0))
    rows.append(row)
big = Image.new("RGB", (rows[0].width, sum(r.height + 6 for r in rows)))
yy = 0
for r in rows:
    big.paste(r, (0, yy)); yy += r.height + 6
big.save(o.out)
print("wrote", o.out)
