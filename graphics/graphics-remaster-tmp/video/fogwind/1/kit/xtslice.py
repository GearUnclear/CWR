#!/usr/bin/env python
"""x-t / y-t slices and 2x crop videos of the fog (fogwind swarm, agent 1).

usage:
  python xtslice.py <outdir> <clip.mkv>:<label>[@first] [<clip2.mkv>:<label2>[@first] ...]  (@first: this clip's own first frame; --to - --from frames) [--from N] [--to M]
                    [--row 0.62] [--x0 0.30 --x1 0.97] [--col 0.85] [--y0 0.44 --y1 0.76]
                    [--box 1100,520,760,280] [--gain 16] [--novideo]

Writes into <outdir>:
  xt.png        per clip three panels, time going DOWN (one line per frame):
                  RAW   the row (mean of 4 rows at --row) as shot: the camera's pan draws lines
                        of one slope.
                  STAB  the same row shifted back by the accumulated camera shift: the scene is
                        vertical lines. Fog that moves with the scene is vertical too; any slanted
                        or wavy structure is motion the camera does not explain (wind / drift).
                  RESID the camera-compensated frame difference along the row, x gain around grey:
                        travelling bands = slanted stripes, pumping = whole horizontal lines,
                        grain = salt and pepper, calm = flat grey.
  yt.png        the same for a column (--col), y vertical... laid out with time going RIGHT.
  crop2x.mp4    the --box crop (x,y,w,h at 1080p) at 2x, clips stacked, labelled (60 fps).
  resid2x.mp4   the camera-compensated difference of the same crop, x gain, 2x, clips stacked.
Run with plain `python`.
"""
import argparse, os, subprocess, sys
import numpy as np
from scipy import ndimage
from PIL import Image, ImageDraw

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fogmotion as fm

FF = "C:/ffmpeg/bin/ffmpeg.exe"


def decode_box(src, a, b, x, y, w, h):
    return fm.decode(src, a, b, (y, y + h, x, x + w))


def cam_shifts(stack):
    T, h, w = stack.shape
    win = fm.hann(h, w)
    return np.array([fm.phase_shift(stack[t], stack[t + 1], win) for t in range(T - 1)])


def resid(stack, sh):
    return np.stack([stack[t + 1] - fm.warp(stack[t], tuple(sh[t])) for t in range(len(sh))])


def label(img, text):
    im = Image.fromarray(img).convert("RGB")
    d = ImageDraw.Draw(im)
    d.rectangle([0, 0, 8 * len(text) + 8, 14], fill=(0, 0, 0))
    d.text((4, 2), text, fill=(255, 255, 0))
    return np.asarray(im)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("outdir")
    ap.add_argument("clips", nargs="+")
    ap.add_argument("--from", dest="a", type=int, default=0)
    ap.add_argument("--to", dest="b", type=int, default=0)
    ap.add_argument("--row", type=float, default=0.62)
    ap.add_argument("--x0", type=float, default=0.30)
    ap.add_argument("--x1", type=float, default=0.97)
    ap.add_argument("--col", type=float, default=0.85)
    ap.add_argument("--y0", type=float, default=0.44)
    ap.add_argument("--y1", type=float, default=0.76)
    ap.add_argument("--box", default="1100,520,760,280")
    ap.add_argument("--gain", type=float, default=16.0)
    ap.add_argument("--novideo", action="store_true")
    o = ap.parse_args()
    os.makedirs(o.outdir, exist_ok=True)
    FW, FH = 1920, 1080
    xt_rows, yt_cols = [], []
    for spec in o.clips:
        src, lab = spec.rsplit(":", 1) if ":" in spec[2:] else (spec, os.path.basename(spec))
        a0 = o.a
        if "@" in lab:
            lab, a0 = lab.split("@")[0], int(lab.split("@")[1])
        b0 = a0 + (o.b - o.a) if o.b else 0
        # x-t: a 120 px tall strip around the row (camera shift estimated on it)
        ry = int(o.row * FH)
        x0, x1 = int(o.x0 * FW), int(o.x1 * FW)
        st = decode_box(src, a0, b0, x0, ry - 60, x1 - x0, 120)
        sh = cam_shifts(st)
        line = st[:, 58:62, :].mean(axis=1)
        T, W = line.shape
        cum = np.concatenate([[0.0], np.cumsum(sh[:, 1])])
        stab = np.stack([ndimage.shift(line[t], -cum[t], order=3, mode="nearest") for t in range(T)])
        rs = resid(st, sh)[:, 58:62, :].mean(axis=1)
        def norm(v):
            lo, hi = np.percentile(v, 1), np.percentile(v, 99)
            return np.clip(255 * (v - lo) / (hi - lo + 1e-6), 0, 255).astype(np.uint8)
        raw8 = norm(line)
        stab8 = norm(stab)
        res8 = np.clip(128 + o.gain * rs, 0, 255).astype(np.uint8)
        res8 = np.concatenate([np.full((1, W), 128, np.uint8), res8])
        gap = np.zeros((T, 6), np.uint8)
        row = np.concatenate([raw8, gap, stab8, gap, res8], axis=1)
        xt_rows.append(label(np.repeat(row, 2, axis=0), f"{lab}  RAW | STAB | RESID x{o.gain:g}  rows y={ry}  frames {o.a}-{o.a + T - 1}"))
        # y-t
        cx = int(o.col * FW)
        y0, y1 = int(o.y0 * FH), int(o.y1 * FH)
        sc = decode_box(src, a0, b0, cx - 60, y0, 120, y1 - y0)
        shc = cam_shifts(sc)
        col = sc[:, :, 58:62].mean(axis=2)  # T x H
        cumy = np.concatenate([[0.0], np.cumsum(shc[:, 0])])
        stabc = np.stack([ndimage.shift(col[t], -cumy[t], order=3, mode="nearest") for t in range(T)])
        rc = resid(sc, shc)[:, :, 58:62].mean(axis=2)
        rc8 = np.concatenate([np.full((1, rc.shape[1]), 128, np.uint8),
                              np.clip(128 + o.gain * rc, 0, 255).astype(np.uint8)])
        gapc = np.zeros((6, T), np.uint8)
        colimg = np.concatenate([norm(col).T, gapc, norm(stabc).T, gapc, rc8.T], axis=0)
        yt_cols.append(label(np.repeat(colimg, 2, axis=1), f"{lab}  y-t col x={cx}: RAW / STAB / RESID (time ->)"))
        print(f"{lab}: camera x-t strip mean shift dx {sh[:, 1].mean():.2f} dy {sh[:, 0].mean():.2f} px/frame; "
              f"column strip dx {shc[:, 1].mean():.2f} dy {shc[:, 0].mean():.2f}")
        sys.stdout.flush()
    def stackv(imgs):
        w = max(i.shape[1] for i in imgs)
        pad = [np.pad(i, ((0, 8), (0, w - i.shape[1]), (0, 0))) for i in imgs]
        return np.concatenate(pad, axis=0)
    Image.fromarray(stackv(xt_rows)).save(os.path.join(o.outdir, "xt.png"))
    Image.fromarray(stackv(yt_cols)).save(os.path.join(o.outdir, "yt.png"))
    if o.novideo:
        return
    bx, by, bw, bh = (int(v) for v in o.box.split(","))
    crops, res = [], []
    labs = []
    for spec in o.clips:
        src, lab = spec.rsplit(":", 1) if ":" in spec[2:] else (spec, os.path.basename(spec))
        a0 = o.a
        if "@" in lab:
            lab, a0 = lab.split("@")[0], int(lab.split("@")[1])
        b0 = a0 + (o.b - o.a) if o.b else 0
        st = decode_box(src, a0, b0, bx, by, bw, bh)
        sh = cam_shifts(st)
        crops.append(st)
        r = resid(st, sh)
        res.append(np.concatenate([np.zeros((1, bh, bw), np.float32), r]))
        labs.append(lab)
    T = min(len(c) for c in crops)
    def write(path, frames_fn):
        h = sum(2 * bh for _ in crops) + 4 * (len(crops) - 1)
        p = subprocess.Popen([FF, "-hide_banner", "-loglevel", "error", "-y", "-f", "rawvideo", "-pix_fmt", "gray",
                              "-s", f"{2 * bw}x{h}", "-r", "60", "-i", "-", "-c:v", "libx264", "-crf", "12",
                              "-preset", "fast", "-pix_fmt", "yuv420p", path], stdin=subprocess.PIPE)
        for t in range(T):
            parts = []
            for k in range(len(crops)):
                f = frames_fn(k, t)
                f = np.kron(f, np.ones((2, 2), np.uint8))
                f = np.asarray(label(f, f"{labs[k]} f{o.a + t}"))[..., 0]
                parts.append(f)
                if k < len(crops) - 1:
                    parts.append(np.zeros((4, 2 * bw), np.uint8))
            p.stdin.write(np.concatenate(parts).tobytes())
        p.stdin.close()
        p.wait()
    write(os.path.join(o.outdir, "crop2x.mp4"), lambda k, t: np.clip(crops[k][t], 0, 255).astype(np.uint8))
    write(os.path.join(o.outdir, "resid2x.mp4"),
          lambda k, t: np.clip(128 + o.gain * res[k][t], 0, 255).astype(np.uint8))


if __name__ == "__main__":
    main()
