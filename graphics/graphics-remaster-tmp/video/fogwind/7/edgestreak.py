"""Agent 7: column streaks of the camera-compensated difference (rows 470-770 averaged), rms per column band.
Prints rms of the column-mean residual for x 1700-1900 (right edge) and x 1150-1650 (interior), frames 15-119."""
import sys, subprocess, numpy as np
sys.path.insert(0, 'D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/7')
from motion7 import phase_shift, warp, window
FF = "C:/ffmpeg/bin/ffmpeg.exe"
X, Y, W, H = 1100, 470, 800, 300
for c in sys.argv[1:]:
    p = subprocess.run([FF, '-hide_banner', '-loglevel', 'error', '-i', c, '-vf', 'crop=%d:%d:%d:%d,format=gray' % (W, H, X, Y), '-f', 'rawvideo', '-'], capture_output=True, check=True)
    v = np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W).astype(np.float32)[15:120]
    win = window(H, W)
    cols = []
    for t in range(1, len(v)):
        d = phase_shift(v[t - 1], v[t], win)[0]
        r = v[t] - warp(v[t - 1], d)
        cm = r[20:-20].mean(axis=0)
        cm = cm - np.convolve(cm, np.ones(61) / 61, mode='same')  # keep column-scale structure
        cols.append(cm)
    cols = np.array(cols)
    edge = np.sqrt((cols[:, 600:780] ** 2).mean()); mid = np.sqrt((cols[:, 50:550] ** 2).mean())
    print('%-28s right-edge cols rms %.3f  interior %.3f' % (c.split('/')[-1], edge, mid))
