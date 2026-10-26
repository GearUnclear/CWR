"""Lead's look sheets for the mist band: 2x crops and d2 x20 strips, before / after / vanilla.

usage: python look.py --row NAME=MKV:FIRST ... --frames 327,328,329 --out <prefix>
  MKV:FIRST  an mkv and the clip frame number (1-based) of its first frame
             (the real clip: 1; agent 6's probe and its reruns: 241).
Writes:
  <prefix>_crop2x_f<mid>.png  the bay and the far valley (x 1280-1920, y 560-800 of 1920x1080) at 2x, one row per mkv
  <prefix>_d2x20.png          the mist band (x 300-1920, y 420-900) as d2 = Y(t) - (Y(t-1) + Y(t+1)) / 2, x20
                              around grey, half size, one row per mkv, one column per frame
  <prefix>_full_f<mid>.png    the whole frame at half size, one row per mkv
Run with plain python (numpy and Pillow are in the user site).
"""
import argparse
import subprocess

import numpy as np
from PIL import Image, ImageDraw

FF = "C:/ffmpeg/bin/ffmpeg.exe"
W, H = 1920, 1080


def frames(mkv, first, nums):
    """RGB frames (float32) of the given clip frame numbers."""
    idx = [n - first for n in nums]
    lo, hi = min(idx), max(idx)
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", mkv, "-vf",
                        f"select='between(n\\,{lo}\\,{hi})'", "-vsync", "0", "-f", "rawvideo", "-pix_fmt", "rgb24", "-"],
                       capture_output=True, check=True)
    x = np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W, 3)
    return {n: x[i - lo].astype(np.float32) for n, i in zip(nums, idx)}


def label(img, text):
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, 8 * len(text) + 10, 18], fill=(0, 0, 0))
    d.text((5, 3), text, fill=(255, 255, 0))
    return img


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--row", action="append", required=True)
    ap.add_argument("--frames", default="327,328,329")
    ap.add_argument("--out", required=True)
    a = ap.parse_args()
    mids = [int(x) for x in a.frames.split(",")]
    need = sorted(set(mids + [m - 1 for m in mids] + [m + 1 for m in mids]))
    mid = mids[len(mids) // 2]
    rows = []
    for r in a.row:
        name, rest = r.split("=", 1)
        mkv, first = rest.rsplit(":", 1)
        rows.append((name, frames(mkv, int(first), need)))
    # 2x crop of the bay and the far valley
    cx0, cx1, cy0, cy1 = 1280, 1920, 560, 800
    tiles = []
    for name, fr in rows:
        c = fr[mid][cy0:cy1, cx0:cx1].clip(0, 255).astype(np.uint8)
        t = Image.fromarray(c).resize(((cx1 - cx0) * 2, (cy1 - cy0) * 2), Image.NEAREST)
        tiles.append(label(t, f"{name} f{mid} 2x"))
    sheet = Image.new("RGB", (tiles[0].width, sum(t.height for t in tiles)))
    y = 0
    for t in tiles:
        sheet.paste(t, (0, y))
        y += t.height
    sheet.save(f"{a.out}_crop2x_f{mid}.png")
    # d2 x20 of the band
    bx0, bx1, by0, by1 = 300, 1920, 420, 900
    tw, th = (bx1 - bx0) // 2, (by1 - by0) // 2
    sheet = Image.new("RGB", (tw * len(mids), th * len(rows)))
    for ri, (name, fr) in enumerate(rows):
        for ci, m in enumerate(mids):
            y0 = fr[m - 1].mean(axis=2)
            y1 = fr[m].mean(axis=2)
            y2 = fr[m + 1].mean(axis=2)
            d2 = (y1 - 0.5 * (y0 + y2))[by0:by1, bx0:bx1]
            g = (128 + 20 * d2).clip(0, 255).astype(np.uint8)
            t = Image.fromarray(g).resize((tw, th), Image.BOX).convert("RGB")
            sheet.paste(label(t, f"{name} f{m} d2x20 rms {np.sqrt((d2 ** 2).mean()):.2f}"), (ci * tw, ri * th))
    sheet.save(f"{a.out}_d2x20.png")
    # whole frame at half size
    tiles = [label(Image.fromarray(fr[mid].clip(0, 255).astype(np.uint8)).resize((W // 2, H // 2), Image.BOX),
                   f"{name} f{mid}") for name, fr in rows]
    sheet = Image.new("RGB", (W // 2, (H // 2) * len(tiles)))
    for i, t in enumerate(tiles):
        sheet.paste(t, (0, i * (H // 2)))
    sheet.save(f"{a.out}_full_f{mid}.png")
    print("wrote", f"{a.out}_crop2x_f{mid}.png", f"{a.out}_d2x20.png", f"{a.out}_full_f{mid}.png")


if __name__ == "__main__":
    main()
