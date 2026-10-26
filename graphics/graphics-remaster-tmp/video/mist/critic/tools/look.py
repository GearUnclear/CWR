"""Critic's look sheets for valley_mist.

usage: python look.py <clip.mkv> <tag> <outdir>
Writes:
  <tag>_win<F>_crop2x.png : 12 consecutive frames F-6..F+5 (0-based), crop x1000-1320 y540-720 at 2x, 4x3 grid,
                            labelled frame numbers in a strip above each tile (plain bars, no font)
  <tag>_win<F>_d2x20.png  : the same 12 frames, temporal d2 x20 of the whole mist band (x288-1920, y486-842) at 1/2 scale
  <tag>_xt.png            : x-t slice: each row = one frame (570 rows), columns = mean over y 600-700 of the band at
                            x288..1920 (1/2 scale), temporal high-pass (minus 9-frame box) x25 around grey 128
"""
import subprocess, sys, os
import numpy as np
from PIL import Image

FF = "C:/ffmpeg/bin/ffmpeg.exe"
WINS = (328, 471, 540)


def decode(src, vf, w, h, ch):
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-vf", vf, "-f", "rawvideo", "-"],
                       capture_output=True, check=True)
    a = np.frombuffer(p.stdout, np.uint8)
    return a.reshape(-1, h, w, ch) if ch > 1 else a.reshape(-1, h, w)


def box(x, k):
    pad = k // 2
    xp = np.concatenate([np.repeat(x[:1], pad, 0), x, np.repeat(x[-1:], pad, 0)], 0)
    c = np.cumsum(np.concatenate([np.zeros_like(xp[:1]), xp], 0), 0)
    return (c[k:] - c[:-k]) / k


def main():
    src, tag, out = sys.argv[1], sys.argv[2], sys.argv[3]
    os.makedirs(out, exist_ok=True)
    # colour crop for the 2x tiles
    cx, cy, cw, chh = 1000, 540, 320, 180
    col = decode(src, f"crop={cw}:{chh}:{cx}:{cy},format=rgb24", cw, chh, 3)
    # grey band for d2 and xt
    bx, by, bw, bh = 288, 486, 1632, 356
    g = decode(src, f"crop={bw}:{bh}:{bx}:{by},format=gray", bw, bh, 1).astype(np.float32)
    n = len(g)
    for F in WINS:
        fr = list(range(F - 6, F + 6))
        tiles = []
        for i in fr:
            t = np.asarray(Image.fromarray(col[i]).resize((cw * 2, chh * 2), Image.NEAREST))
            bar = np.zeros((10, cw * 2, 3), np.uint8)
            bar[:, : int((i - fr[0] + 1) / 12 * cw * 2)] = (200, 200, 0)
            tiles.append(np.concatenate([bar, t], 0))
        rows = [np.concatenate(tiles[r * 4:(r + 1) * 4], 1) for r in range(3)]
        Image.fromarray(np.concatenate(rows, 0)).save(f"{out}/{tag}_win{F}_crop2x.png")
        d2t = []
        for i in fr:
            d = g[i] - 0.5 * (g[i - 1] + g[i + 1])
            im = np.clip(128 + 20 * d, 0, 255).astype(np.uint8)
            im = np.asarray(Image.fromarray(im).resize((bw // 2, bh // 2), Image.BOX))
            d2t.append(np.pad(im, ((2, 2), (2, 2)), constant_values=0))
        rows = [np.concatenate(d2t[r * 3:(r + 1) * 3], 1) for r in range(4)]
        Image.fromarray(np.concatenate(rows, 0)).save(f"{out}/{tag}_win{F}_d2x20.png")
    xt = g[:, 114:214, :].mean(axis=1)  # y 600..700
    xt = xt.reshape(n, bw // 2, 2).mean(axis=2)
    hp = xt - box(xt, 9)
    Image.fromarray(np.clip(128 + 25 * hp, 0, 255).astype(np.uint8)).save(f"{out}/{tag}_xt.png")
    Image.fromarray(np.clip(xt, 0, 255).astype(np.uint8)).save(f"{out}/{tag}_xt_raw.png")
    print("ok", tag, n)


if __name__ == "__main__":
    main()
