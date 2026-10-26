import sys, subprocess, numpy as np
from PIL import Image, ImageDraw
sys.path.insert(0, 'D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/7')
from motion7 import phase_shift, warp, window
from scipy import ndimage
FF = "C:/ffmpeg/bin/ffmpeg.exe"
out, a, n = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
clips = sys.argv[4:]
X, Y, W, H = 1100, 470, 800, 300
rows = []
for c in clips:
    p = subprocess.run([FF, '-hide_banner', '-loglevel', 'error', '-i', c, '-vf', 'crop=%d:%d:%d:%d,format=gray' % (W, H, X, Y), '-f', 'rawvideo', '-'], capture_output=True, check=True)
    v = np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W).astype(np.float32)[a:a + n + 1]
    win = window(H, W)
    ds = []
    for t in range(1, len(v)):
        d = phase_shift(v[t - 1], v[t], win)[0]
        r = v[t] - warp(v[t - 1], d)
        r = ndimage.gaussian_filter(r, 3)
        ds.append(np.clip(128 + 40 * r, 0, 255)[::2, ::2])
    rows.append(np.hstack([np.pad(x, 2) for x in ds]))
img = np.vstack([np.pad(r, ((14, 0), (0, 0))) for r in rows])
im = Image.fromarray(img.astype(np.uint8))
dr = ImageDraw.Draw(im)
for i, c in enumerate(clips):
    dr.text((4, i * (H // 2 + 4 + 14)), c.split('/')[-1] + '  frames %d..%d, diff blurred 3px x40' % (a, a + n), fill=255)
im.save(out)
