#!/usr/bin/env python
"""fable-3: one frame's 2x crop of the bay and far valley, unstretched, for several clips stacked.
python crop2x.py out.png frame clip.mkv:label ... [--box 1100,500,600,200]
"""
import argparse, subprocess
import numpy as np
from PIL import Image, ImageDraw

FF = "C:/ffmpeg/bin/ffmpeg.exe"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("frame", type=int)
    ap.add_argument("clips", nargs="+")
    ap.add_argument("--box", default="1100,500,600,200")
    o = ap.parse_args()
    x, y, w, h = [int(v) for v in o.box.split(",")]
    rows = []
    for c in o.clips:
        p, lab = c.rsplit(":", 1)
        raw = subprocess.run([FF, "-v", "error", "-i", p, "-vf", "select='eq(n,%d)'" % o.frame, "-vsync", "0",
                              "-frames:v", "1", "-f", "rawvideo", "-pix_fmt", "rgb24", "-"],
                             capture_output=True, check=True).stdout
        fr = np.frombuffer(raw[:1920 * 1080 * 3], np.uint8).reshape(1080, 1920, 3)
        img = Image.fromarray(fr[y:y + h, x:x + w]).resize((2 * w, 2 * h), Image.NEAREST)
        ImageDraw.Draw(img).text((6, 6), "%s frame %d, 2x, no stretch" % (lab, o.frame), fill=(255, 255, 0))
        rows.append(np.asarray(img))
    Image.fromarray(np.concatenate(rows, axis=0)).save(o.out)
    print("wrote", o.out)


if __name__ == "__main__":
    main()
