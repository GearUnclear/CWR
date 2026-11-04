"""Mist-band flicker metric for the valley_mist shot (shared by the mist swarm).

usage: python bandmetric.py <clip.mkv | frames_dir> [--from N] [--to M] [--band r0,r1,c0,c1] [--json out.json]

Frames are scaled to 480x270 grey and cut into 15x15 px blocks (an 18 x 32 grid).
Per block: the signed temporal second difference d2(t) = Y(t) - (Y(t-1) + Y(t+1)) / 2
of the block mean, then its RMS over time. A steady pan gives a small d2; a block
that brightens and darkens from frame to frame gives a large one.

Prints:
  band   = mean block RMS over the mist band (default rows 8-13, cols 5-31: the
           valley, the mist and the bay below the horizon)
  sky    = the same over rows 0-5 (the sky)
  spikes = how many (block, frame) samples in the band have |d2| > 0.5/255
  worst  = the frames with the largest band |d2|
Reference values, full 570-frame clips as shot on 2026-10-19:
  ud (overhaul)  band 0.567  sky 0.051  spikes 25076   worst frames 328, 329, 472, 471, 540, 429, 326
  vanilla        band 0.106  sky 0.025  spikes 0
  ud frames 270-390 (--from 270 --to 390): band 0.526, spikes 5482
  Goal after the fix: the ud band close to vanilla (under about 0.15) with no spikes cluster.
Run with plain `python` (numpy is in the user site; `-I` hides it).
"""
import argparse, glob, json, os, shutil, subprocess, sys
import numpy as np

W, H, B = 480, 270, 15
# ffmpeg: the FFMPEG environment variable, else the one on PATH.
FF = os.environ.get("FFMPEG") or shutil.which("ffmpeg") or "ffmpeg"


def load(src, a, b):
    if os.path.isdir(src):
        files = sorted(glob.glob(os.path.join(src, "*.png")) + glob.glob(os.path.join(src, "*.bmp"))
                       + glob.glob(os.path.join(src, "*.jpg")))
        files = files[a:b]
        from PIL import Image
        arr = [np.asarray(Image.open(f).convert("L").resize((W, H), Image.BOX), np.float32) for f in files]
        return np.stack(arr)
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src,
                        "-vf", f"scale={W}:{H}:flags=area,format=gray", "-f", "rawvideo", "-"],
                       capture_output=True, check=True)
    x = np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W).astype(np.float32)
    return x[a:b]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("src")
    ap.add_argument("--from", dest="a", type=int, default=0)
    ap.add_argument("--to", dest="b", type=int, default=None)
    ap.add_argument("--band", default="8,13,5,31")
    ap.add_argument("--json", default="")
    o = ap.parse_args()
    x = load(o.src, o.a, o.b)
    gy, gx = H // B, W // B
    blk = x[:, :gy * B, :gx * B].reshape(len(x), gy, B, gx, B).mean(axis=(2, 4))
    d2 = blk[1:-1] - 0.5 * (blk[:-2] + blk[2:])
    rms = np.sqrt((d2 ** 2).mean(axis=0))
    r0, r1, c0, c1 = (int(v) for v in o.band.split(","))
    band = rms[r0:r1 + 1, c0:c1 + 1]
    sky = rms[0:6]
    bd2 = np.abs(d2[:, r0:r1 + 1, c0:c1 + 1])
    spikes = int((bd2 > 0.5).sum())
    per = bd2.reshape(len(bd2), -1).max(axis=1)
    worst = [(int(i) + 1 + o.a, round(float(per[i]), 2)) for i in np.argsort(per)[-8:][::-1]]
    res = dict(src=o.src, frames=int(len(x)), band=round(float(band.mean()), 3), band_max=round(float(band.max()), 3),
               sky=round(float(sky.mean()), 3), spikes=spikes, worst=worst)
    print(json.dumps(res))
    if o.json:
        with open(o.json, "w") as f:
            json.dump(dict(res, grid=(rms * 100).round(0).astype(int).tolist()), f)


if __name__ == "__main__":
    main()
