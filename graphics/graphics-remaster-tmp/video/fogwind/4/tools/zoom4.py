"""Agent 4: 2x crops of one frame of several clips, stacked, with a local contrast stretch option.
python zoom4.py out.png --frame 50 --box x,y,w,h [--stretch] clip:label ..."""
import argparse, subprocess
import numpy as np
from PIL import Image, ImageDraw
FF = "C:/ffmpeg/bin/ffmpeg.exe"
ap = argparse.ArgumentParser()
ap.add_argument("out"); ap.add_argument("clips", nargs="+")
ap.add_argument("--frame", type=int, default=50); ap.add_argument("--box", default="1150,490,450,160")
ap.add_argument("--stretch", action="store_true")
o = ap.parse_args()
x, y, w, h = [int(v) for v in o.box.split(",")]
rows = []
for c in o.clips:
    p, lab = c.rsplit(":", 1)
    raw = subprocess.run([FF, "-v", "error", "-i", p, "-vf", f"select='eq(n\,{o.frame})',crop={w}:{h}:{x}:{y}", "-vsync", "0",
                          "-f", "rawvideo", "-pix_fmt", "rgb24", "-"], capture_output=True, check=True).stdout
    im = np.frombuffer(raw[: w * h * 3], np.uint8).reshape(h, w, 3).astype(np.float32)
    if o.stretch:
        lo, hi = np.percentile(im, 1), np.percentile(im, 99)
        im = np.clip((im - lo) / max(hi - lo, 1) * 255, 0, 255)
    pim = Image.fromarray(im.astype(np.uint8)).resize((2 * w, 2 * h), Image.NEAREST)
    d = ImageDraw.Draw(pim); d.rectangle([0, 0, 8 * len(lab) + 8, 14], fill=(0, 0, 0)); d.text((4, 2), lab, fill=(255, 255, 0))
    rows.append(np.asarray(pim))
Image.fromarray(np.concatenate(rows, 0)).save(o.out); print("wrote", o.out)
