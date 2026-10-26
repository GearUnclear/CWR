"""Decode a clip's fog band to a float16 luma cache (critic1, independent of the swarm's kits).

usage: python cache.py <clip.mkv> <label> [--y0 400 --y1 840]
Writes <scratch>/<label>.npy : (T, y1-y0, 1920) float16 luma (Rec.709 weights, code values 0..255)
and <scratch>/<label>_rgbmean.npy : (T, y1-y0, 1920, 3) is NOT kept (too big); region RGB means are taken later from the mkv.
"""
import sys, subprocess, argparse, os
import numpy as np

SCRATCH = "C:/Users/Dane/AppData/Local/Temp/claude/D--Arma-CWA/f07c0602-9af9-49db-b0ed-5558491d5e03/scratchpad/cache"


def decode(path, y0, y1, x0=0, x1=1920, f0=None, f1=None, rgb=False):
    w, h = x1 - x0, y1 - y0
    vf = []
    if f0 is not None:
        vf.append(f"trim=start_frame={f0}:end_frame={f1},setpts=PTS-STARTPTS")
    vf.append(f"crop={w}:{h}:{x0}:{y0}")
    cmd = ["ffmpeg", "-v", "error", "-i", path, "-vf", ",".join(vf), "-fps_mode", "passthrough", "-f", "rawvideo", "-pix_fmt", "rgb24", "-"]
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, bufsize=10 ** 8)
    n = w * h * 3
    out = []
    while True:
        b = p.stdout.read(n)
        if len(b) < n:
            break
        a = np.frombuffer(b, np.uint8).reshape(h, w, 3)
        if rgb:
            out.append(a.copy())
        else:
            y = a[..., 0] * 0.2126 + a[..., 1] * 0.7152 + a[..., 2] * 0.0722
            out.append(y.astype(np.float16))
    p.wait()
    return np.stack(out)


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("clip")
    ap.add_argument("label")
    ap.add_argument("--y0", type=int, default=400)
    ap.add_argument("--y1", type=int, default=840)
    o = ap.parse_args()
    os.makedirs(SCRATCH, exist_ok=True)
    a = decode(o.clip, o.y0, o.y1)
    np.save(f"{SCRATCH}/{o.label}.npy", a)
    print(o.label, a.shape, float(a[0].astype(np.float32).mean()))
