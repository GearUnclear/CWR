"""12 consecutive frames of a crop at 2x, and their 11 camera-compensated differences amplified.

usage: python consec.py outdir t0 x0 y0 w h gain label=clip.mkv [label=clip.mkv ...]
Writes <outdir>/consec_<t0>_<label>_frames.png (4x3 grid of 2x crops, frame numbers burnt in) and
<outdir>/consec_<t0>_diffs.png: one row per clip, 11 differences (frame t+1 minus frame t shifted by
the camera's track at this place, luma, x gain, 2x).
"""
import sys, os
import numpy as np
from scipy import ndimage
from PIL import Image, ImageDraw
sys.path.insert(0, os.path.dirname(__file__))
from cache import decode
from common import SCRATCH

out, t0, x0, y0, w, h, gain = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4]), int(sys.argv[5]), int(sys.argv[6]), float(sys.argv[7])
clips = [s.split("=", 1) for s in sys.argv[8:]]
os.makedirs(out, exist_ok=True)
tr = np.load(f"{SCRATCH}/camtrack.npy", allow_pickle=True).item()
d = tr["reg"]["bay"]  # consecutive shift (dy, dx) per frame
PAD = 8
rows = []
for lab, path in clips:
    rgb = decode(path, y0 - PAD, y0 + h + PAD, x0 - PAD, x0 + w + PAD, t0, t0 + 12, rgb=True)
    frames = []
    for i in range(12):
        im = Image.fromarray(rgb[i, PAD:-PAD, PAD:-PAD]).resize((w * 2, h * 2), Image.LANCZOS)
        ImageDraw.Draw(im).text((4, 4), f"{lab} f{t0 + i}", fill=(255, 255, 255))
        frames.append(im)
    grid = Image.new("RGB", (w * 2 * 4, h * 2 * 3))
    for i, im in enumerate(frames):
        grid.paste(im, ((i % 4) * w * 2, (i // 4) * h * 2))
    grid.save(f"{out}/consec_{t0}_{lab}_frames.png")
    Y = rgb.astype(np.float32) @ np.array([0.2126, 0.7152, 0.0722], np.float32)
    diffs = []
    for i in range(11):
        sh = ndimage.shift(Y[i], d[t0 + i + 1], order=3, mode="nearest")
        D = (Y[i + 1] - sh)[PAD:-PAD, PAD:-PAD]
        img = np.clip(D * gain + 128, 0, 255).astype(np.uint8)
        im = Image.fromarray(img).resize((w * 2, h * 2), Image.NEAREST)
        ImageDraw.Draw(im).text((4, 4), f"{lab} {t0 + i + 1}-{t0 + i}", fill=255)
        diffs.append(im)
    rows.append(diffs)
# diffs sheet: per clip a row of the first 6 diffs, then a row of the next 5
W = w * 2
sheet = Image.new("L", (W * 6, h * 2 * 2 * len(rows)), 128)
for r, diffs in enumerate(rows):
    for i, im in enumerate(diffs):
        sheet.paste(im, ((i % 6) * W, (r * 2 + i // 6) * h * 2))
sheet.save(f"{out}/consec_{t0}_diffs.png")
print("ok", sheet.size)
