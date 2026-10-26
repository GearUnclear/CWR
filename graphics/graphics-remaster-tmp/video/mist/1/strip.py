#!/usr/bin/env python
"""Agent 1: crop strip of consecutive frames (2x) plus the frame-to-frame difference (x8).

  python strip.py <out.png> <a.mkv>:<first>:<count>:<label> [<b.mkv>:<first>:<count>:<label> ...] [--box x,y,w,h]

One row per clip: the crops of `count` consecutive frames from `first` (0-based), each
scaled 2x; under it the signed difference to the previous frame, x8 around grey 128.
"""
import subprocess
import sys

import numpy as np
from PIL import Image, ImageDraw

FF = "C:/ffmpeg/bin/ffmpeg.exe"


def frames(src, first, count):
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-vf",
                        f"select='between(n\\,{first}\\,{first + count - 1})'", "-vsync", "0", "-f", "rawvideo",
                        "-pix_fmt", "rgb24", "-"], capture_output=True, check=True)
    return np.frombuffer(p.stdout, np.uint8).reshape(-1, 1080, 1920, 3)


def main():
    args = sys.argv[1:]
    box = (1000, 520, 480, 270)
    if "--box" in args:
        i = args.index("--box")
        box = tuple(int(v) for v in args[i + 1].split(","))
        del args[i:i + 2]
    out, specs = args[0], args[1:]
    x, y, w, h = box
    s = 1  # crops are shown at 1:1 per frame; the whole sheet is then scaled 2x
    rows = []
    for spec in specs:
        src, first, count, label = spec.rsplit(":", 3)
        fr = frames(src, int(first), int(count))[:, y:y + h, x:x + w].astype(np.int16)
        top = np.concatenate(list(fr), axis=1)
        d = [np.full_like(fr[0], 128)] + [np.clip(128 + 8 * (fr[i] - fr[i - 1]), 0, 255) for i in range(1, len(fr))]
        bot = np.concatenate(d, axis=1)
        img = Image.fromarray(np.concatenate([top, bot], axis=0).astype(np.uint8))
        ImageDraw.Draw(img).text((4, 4), f"{label} frames {first}+", fill=(255, 0, 0))
        rows.append(img)
    W = max(r.width for r in rows)
    sheet = Image.new("RGB", (W, sum(r.height for r in rows)), (0, 0, 0))
    yy = 0
    for r in rows:
        sheet.paste(r, (0, yy))
        yy += r.height
    sheet = sheet.resize((sheet.width * 2, sheet.height * 2), Image.LANCZOS)
    sheet.save(out)
    print(out, sheet.size)


if __name__ == "__main__":
    main()
