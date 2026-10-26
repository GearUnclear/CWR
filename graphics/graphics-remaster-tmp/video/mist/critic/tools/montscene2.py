"""Scene 2 of the split montage, divider pause window (montage 19.1-20.6 s, clip frames 240-330, the
divider still at 0.52): the overhaul side right of x 0.65 W, mist band y 45-78%. Block d2 at 60 px
(bandmetric scale) and 20 px, old montage vs new montage. Also the last clean all-overhaul span 23.1-23.6 s."""
import subprocess, json, sys
import numpy as np
FF = "C:/ffmpeg/bin/ffmpeg.exe"
def load(src, t0, dur):
    r = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-ss", str(t0), "-i", src, "-t", str(dur),
                        "-vf", "format=gray", "-f", "rawvideo", "-"], capture_output=True, check=True)
    return np.frombuffer(r.stdout, np.uint8).reshape(-1, 1080, 1920)
def m(a, B):
    a = a.astype(np.float32); n, h, w = a.shape
    blk = a[:, :h // B * B, :w // B * B].reshape(n, h // B, B, w // B, B).mean(axis=(2, 4))
    d = blk[1:-1] - 0.5 * (blk[:-2] + blk[2:])
    return dict(rms=round(float(np.sqrt((d ** 2).mean())), 3), spikes=int((np.abs(d) > 0.5).sum()), n=int(n))
out = {}
for name, src in (("old", sys.argv[1]), ("new", sys.argv[2])):
    x = load(src, 19.1, 1.5)[:, 486:842, 1248:1920]
    y = load(src, 23.1, 0.5)[:, 486:842, 288:1920]
    out[name] = dict(pause_60=m(x, 60), pause_20=m(x, 20), allud_60=m(y, 60), allud_20=m(y, 20))
    print(name, json.dumps(out[name]), flush=True)
