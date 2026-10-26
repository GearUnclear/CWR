#!/usr/bin/env python
"""fable-3: a picture of the curtains. For each clip the time mean of frames a..b, a crop under the sun,
shown raw (2x) and high-passed along x (minus a 31 px box, x8, around 128) so fixed vertical streaks
stand out; clips stacked with labels.

python curtainview.py out.png clip.mkv:label ... [--box 1100,480,600,220] [--from 15] [--to 120]
"""
import argparse, subprocess
import numpy as np
from scipy import ndimage
from PIL import Image, ImageDraw

FF = "C:/ffmpeg/bin/ffmpeg.exe"


def mean_frame(p, a, b):
    sel = "select='between(n," + str(a) + "," + str(b - 1) + ")'"
    raw = subprocess.run([FF, "-v", "error", "-i", p, "-vf", sel, "-vsync", "0", "-f", "rawvideo",
                          "-pix_fmt", "gray", "-"], capture_output=True, check=True).stdout
    n = len(raw) // (1920 * 1080)
    fr = np.frombuffer(raw[: n * 1920 * 1080], np.uint8).reshape(n, 1080, 1920).astype(np.float32)
    return fr.mean(axis=0)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("clips", nargs="+")
    ap.add_argument("--box", default="1100,480,600,220")
    ap.add_argument("--from", dest="a", type=int, default=15)
    ap.add_argument("--to", dest="b", type=int, default=120)
    o = ap.parse_args()
    x, y, w, h = [int(v) for v in o.box.split(",")]
    rows = []
    for c in o.clips:
        p, lab = c.rsplit(":", 1)
        m = mean_frame(p, o.a, o.b)[y:y + h, x:x + w]
        hp = m - ndimage.uniform_filter1d(m, 31, axis=1, mode="nearest")
        raw = np.clip(m, 0, 255).astype(np.uint8)
        st = np.clip(128 + 8 * hp, 0, 255).astype(np.uint8)
        img = Image.fromarray(np.concatenate([raw, st], axis=1)).resize((2 * 2 * w, 2 * h), Image.NEAREST)
        d = ImageDraw.Draw(img)
        d.text((6, 6), lab + "  (left: time mean 2x, right: x high-pass x8)", fill=255)
        rows.append(np.asarray(img))
    Image.fromarray(np.concatenate(rows, axis=0)).save(o.out)
    print("wrote", o.out)


if __name__ == "__main__":
    main()
