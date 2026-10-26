"""Agent 4: vertical-curtain strength on a STATIC probe. Time-mean of frames a..b, region box,
column means (curtains are vertical streaks below the sun), high-pass along x (minus a 31 px
box average), rms in code values (luma). Also the time-mean region level.

python curtain4.py clip.mkv:label ... [--box 1150,520,450,120] [--from 15] [--to 120]
"""
import argparse, subprocess
import numpy as np
from scipy import ndimage

FF = "C:/ffmpeg/bin/ffmpeg.exe"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("clips", nargs="+")
    ap.add_argument("--box", action="append", default=[])
    ap.add_argument("--from", dest="a", type=int, default=15)
    ap.add_argument("--to", dest="b", type=int, default=120)
    o = ap.parse_args()
    boxes = o.box or ["1150,520,450,120", "700,500,450,120", "1300,440,600,80"]
    print("curtain rms (code values) per box: " + " | ".join(boxes))
    for c in o.clips:
        p, lab = c.rsplit(":", 1)
        sel = "select='between(n," + str(o.a) + "," + str(o.b - 1) + ")'"
        raw = subprocess.run([FF, "-v", "error", "-i", p, "-vf", sel, "-vsync", "0", "-f", "rawvideo",
                              "-pix_fmt", "gray", "-"], capture_output=True, check=True).stdout
        n = len(raw) // (1920 * 1080)
        fr = np.frombuffer(raw[: n * 1920 * 1080], np.uint8).reshape(n, 1080, 1920).astype(np.float32)
        m = fr.mean(axis=0)
        out = []
        for b in boxes:
            x, y, w, h = [int(v) for v in b.split(",")]
            col = m[y:y + h, x:x + w].mean(axis=0)
            hp = col - ndimage.uniform_filter1d(col, 31, mode="nearest")
            out.append(f"{np.sqrt((hp[15:-15] ** 2).mean()):.3f} (lvl {m[y:y + h, x:x + w].mean():.1f})")
        print(f"{lab:12s} " + " | ".join(out))


if __name__ == "__main__":
    main()
