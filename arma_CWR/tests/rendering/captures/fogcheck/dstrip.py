"""Consecutive frame-difference strip: python dstrip.py out.png clip:LABEL ... [--first 50] [--n 4] [--box x,y,w,h] [--gain 24]
One row per clip: n consecutive (or --step k: k frames apart, for slow change) camera-compensated differences D_t (x gain around grey), crop at 1:1, then 2x."""
import sys, os
import numpy as np
from PIL import Image, ImageDraw
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fogmotion as fm
args = sys.argv[1:]
opt = {"--first": "50", "--n": "4", "--box": "1250,560,320,160", "--gain": "24", "--step": "1"}
for k in list(opt):
    if k in args:
        i = args.index(k); opt[k] = args[i + 1]; del args[i:i + 2]
out, specs = args[0], args[1:]
first, n, gain, step = int(opt["--first"]), int(opt["--n"]), float(opt["--gain"]), int(opt["--step"])
x, y, w, h = (int(v) for v in opt["--box"].split(","))
rows = []
for spec in specs:
    src, lab = spec.rsplit(":", 1)
    st = fm.decode(src, first, first + n * step + 1, (y - 40, y + h + 40, x - 40, x + w + 40))[::step]
    win = fm.hann(*st[0].shape)
    tiles = []
    for t in range(n):
        d = fm.phase_shift(st[t], st[t + 1], win)
        D = st[t + 1] - fm.warp(st[t], d)
        tiles.append(np.clip(128 + gain * D[40:-40, 40:-40], 0, 255).astype(np.uint8))
        tiles.append(np.zeros((h, 4), np.uint8))
    row = np.concatenate(tiles, axis=1)
    im = Image.fromarray(np.kron(row, np.ones((2, 2), np.uint8))).convert("RGB")
    ImageDraw.Draw(im).text((4, 2), f"{lab} D f{first}..{first + n * step} step {step} x{gain:g}", fill=(255, 255, 0))
    rows.append(np.asarray(im))
Image.fromarray(np.concatenate(rows, axis=0)).save(out)
