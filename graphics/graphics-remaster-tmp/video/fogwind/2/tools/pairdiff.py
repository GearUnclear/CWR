"""Agent 2: inter-clip differences for a deterministic moving probe shot with two shader overrides.
The staging, camera and clock are the same shot for shot, so a - b holds only what the fog does differently
(the residuals of both, plus any non-determinism). For each clip pair prints the rms of (a-b) per frame
in a box and its frame-to-frame change, and writes a strip of n consecutive (a-b) frames (x gain) and an x-t
slice of (a-b) along a row.
usage: python pairdiff.py out_prefix a.mkv b.mkv [--box x,y,w,h] [--row y] [--t0 50] [--n 3] [--gain 12]
"""
import argparse, subprocess
import numpy as np
from PIL import Image, ImageDraw

FF = "C:/ffmpeg/bin/ffmpeg.exe"


def dec(src):
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-vf", "format=gray", "-f", "rawvideo", "-"],
                       capture_output=True, check=True)
    return np.frombuffer(p.stdout, np.uint8).reshape(-1, 1080, 1920)


ap = argparse.ArgumentParser()
ap.add_argument("out")
ap.add_argument("a")
ap.add_argument("b")
ap.add_argument("--box", default="560,380,420,300")
ap.add_argument("--row", type=int, default=520)
ap.add_argument("--t0", type=int, default=50)
ap.add_argument("--n", type=int, default=3)
ap.add_argument("--gain", type=float, default=12.0)
ap.add_argument("--from", dest="f0", type=int, default=15)
o = ap.parse_args()
A, B = dec(o.a), dec(o.b)
n = min(len(A), len(B))
x, y, w, h = (int(v) for v in o.box.split(","))
D = A[o.f0:n].astype(np.float32) - B[o.f0:n].astype(np.float32)
box = D[:, y:y + h, x:x + w]
from scipy import ndimage
b6 = ndimage.uniform_filter(box, size=(1, 6, 6))
print("box a-b: rms %.3f  mean %.3f  | 6px-blurred rms %.3f, its frame-to-frame change rms %.3f" % (
    box.std(), box.mean(), b6.std(), np.diff(b6, axis=0).std()))
img = Image.new("L", (w * 2 * o.n, h * 2 + 16), 0)
d = ImageDraw.Draw(img)
d.text((4, 2), f"(a - b) x{o.gain:g}, frames {o.t0}..{o.t0 + o.n - 1}; a={o.a.split('/')[-1]} b={o.b.split('/')[-1]}", fill=255)
for i in range(o.n):
    f = D[o.t0 - o.f0 + i, y:y + h, x:x + w] * o.gain + 128
    img.paste(Image.fromarray(np.clip(f, 0, 255).astype(np.uint8)).resize((w * 2, h * 2), Image.NEAREST), (i * w * 2, 16))
img.save(o.out + "_strip.png")
xt = D[:, o.row - 1:o.row + 2, :].mean(axis=1)
Image.fromarray(np.clip(xt * o.gain + 128, 0, 255).astype(np.uint8)).resize((1920, xt.shape[0] * 2), Image.NEAREST).save(o.out + "_xt.png")
