"""Fine-scale temporal noise in the mist band (static camera): per-pixel d2 RMS at 960x540 grey,
and at 4 px blocks (the froxel cell is 12 px at 1920, 6 px at 960), over the band region.
usage: python finegrain.py clip.mkv [--from 15]"""
import argparse, subprocess, numpy as np
FF = "C:/ffmpeg/bin/ffmpeg.exe"; W, H = 960, 540
ap = argparse.ArgumentParser(); ap.add_argument("src"); ap.add_argument("--from", dest="a", type=int, default=15); o = ap.parse_args()
p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", o.src, "-vf", f"scale={W}:{H}:flags=area,format=gray",
                    "-f", "rawvideo", "-"], capture_output=True, check=True)
x = np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W).astype(np.float32)[o.a:]
y0, y1, x0, x1 = int(0.47*H), int(0.70*H), int(0.45*W), int(0.98*W)
b = x[:, y0:y1, x0:x1]
def d2rms(s): d = s[1:-1] - 0.5*(s[:-2]+s[2:]); return float(np.sqrt((d**2).mean()))
for blk in (1, 3, 6, 12, 30):
    hh, ww = (b.shape[1]//blk)*blk, (b.shape[2]//blk)*blk
    m = b[:, :hh, :ww].reshape(len(b), hh//blk, blk, ww//blk, blk).mean(axis=(2, 4))
    print(f"block {blk:2d}px@960: d2 rms {d2rms(m):.3f}")
