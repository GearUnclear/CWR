"""Agent 7: per-frame mean of the camera-compensated difference in a box (signed), to find step events.
python events.py <mkv> <x,y,w,h> [--from 15 --to 120]
"""
import sys, argparse, subprocess
import numpy as np
sys.path.insert(0, 'D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/7')
from motion7 import phase_shift, warp, window

FF = "C:/ffmpeg/bin/ffmpeg.exe"
ap = argparse.ArgumentParser()
ap.add_argument('src'); ap.add_argument('box')
ap.add_argument('--from', dest='a', type=int, default=15)
ap.add_argument('--to', dest='b', type=int, default=120)
o = ap.parse_args()
x, y, w, h = (int(v) for v in o.box.split(','))
p = subprocess.run([FF, '-hide_banner', '-loglevel', 'error', '-i', o.src, '-vf', 'crop=%d:%d:%d:%d,format=gray' % (w + 64, h + 64, x - 32, y - 32),
                    '-f', 'rawvideo', '-'], capture_output=True, check=True)
v = np.frombuffer(p.stdout, np.uint8).reshape(-1, h + 64, w + 64).astype(np.float32)[o.a:o.b]
win = window(h + 64, w + 64)
vals = []
for t in range(1, len(v)):
    d = phase_shift(v[t - 1], v[t], win)[0]
    r = (v[t] - warp(v[t - 1], d))[32:-32, 32:-32]
    vals.append(r.mean())
vals = np.array(vals)
print('rms %.3f  max|.| %.3f' % (np.sqrt((vals ** 2).mean()), np.abs(vals).max()))
print(' '.join('%d:%+.2f' % (i + o.a + 1, s) for i, s in enumerate(vals) if abs(s) > 2.5 * np.sqrt((vals ** 2).mean())))
print(' '.join('%+.2f' % s for s in vals))
