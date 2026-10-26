"""d2 RMS of block means at the froxel cell scale (12 px at 1080p) and 2 and 5 cells, in the mist band
(y 45-78 %, x 15-100 %), so a per-cell residue the 60 px bandmetric blocks average away still shows.
usage: python cellscale.py NAME=MKV:FROM:TO ...   (0-based frame range of that mkv; plain python)"""
import subprocess, sys
import numpy as np
FF = "C:/ffmpeg/bin/ffmpeg.exe"
W, H = 960, 540
for arg in sys.argv[1:]:
    name, rest = arg.split("=", 1)
    mkv, a, b = rest.rsplit(":", 2)
    a, b = int(a), int(b)
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", mkv, "-vf",
                        f"select='between(n\\,{a}\\,{b - 1})',scale={W}:{H}:flags=area,format=gray", "-vsync", "0",
                        "-f", "rawvideo", "-"], capture_output=True, check=True)
    x = np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W).astype(np.float32)
    y0, y1, x0, x1 = int(0.45 * H), int(0.78 * H), int(0.15 * W), W
    out = [name, f"frames {len(x)}"]
    for bs, lab in ((6, "1 cell"), (12, "2 cells"), (30, "5 cells")):
        r = x[:, y0:y1, x0:x1]
        hh, ww = (r.shape[1] // bs) * bs, (r.shape[2] // bs) * bs
        blk = r[:, :hh, :ww].reshape(len(r), hh // bs, bs, ww // bs, bs).mean(axis=(2, 4))
        d2 = blk[1:-1] - 0.5 * (blk[:-2] + blk[2:])
        out.append(f"{lab}: rms {np.sqrt((d2 ** 2).mean()):.3f} p99 {np.percentile(np.abs(d2), 99):.2f} over1 {(np.abs(d2) > 1).mean() * 100:.2f}%")
    print(" | ".join(out), flush=True)
