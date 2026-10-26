"""Agent 7: map of low-frequency flicker (the kit's lowd2, per 40 px block of the camera-stabilised frame).
python lowmap.py out.png a.mkv b.mkv ... [--from 68 --to 120]
Each clip: frames stabilised to the first by the accumulated phase-correlation shift (full frame, y 300-900),
block means of 40 px, rms of the temporal 2nd difference, shown x60 (white = 4 codes).
"""
import sys, argparse, subprocess
import numpy as np
from PIL import Image, ImageDraw
sys.path.insert(0, 'D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/7')
from motion7 import phase_shift, warp, window

FF = "C:/ffmpeg/bin/ffmpeg.exe"
ap = argparse.ArgumentParser()
ap.add_argument('out'); ap.add_argument('clips', nargs='+')
ap.add_argument('--from', dest='a', type=int, default=68)
ap.add_argument('--to', dest='b', type=int, default=120)
o = ap.parse_args()
X0, Y0, W, H, B = 0, 380, 1920, 520, 40
rows = []
for c in o.clips:
    p = subprocess.run([FF, '-hide_banner', '-loglevel', 'error', '-i', c, '-vf', 'crop=%d:%d:%d:%d,format=gray' % (W, H, X0, Y0),
                        '-f', 'rawvideo', '-'], capture_output=True, check=True)
    v = np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W).astype(np.float32)[o.a:o.b]
    win = window(H, W)
    acc = np.zeros(2)
    st = [v[0]]
    for t in range(1, len(v)):
        acc += phase_shift(v[t - 1], v[t], win)[0]
        st.append(warp(v[t], -acc))
    st = np.stack(st)
    m = 64
    st = st[:, :, m:-m] if False else st
    gy, gx = H // B, W // B
    blk = st[:, :gy * B, :gx * B].reshape(len(st), gy, B, gx, B).mean(axis=(2, 4))
    d2 = blk[1:-1] - 0.5 * (blk[:-2] + blk[2:])
    rms = np.sqrt((d2 ** 2).mean(axis=0))
    print(c, 'mean block lowd2 %.3f, median %.3f' % (rms[:, 2:-6].mean(), np.median(rms[:, 2:-6])))
    img = np.kron(np.clip(rms * 60, 0, 255), np.ones((B // 4, B // 4)))
    rows.append((c, img, st[len(st) // 2][::4, ::4]))
out = Image.new('L', (W // 4 * 2 + 10, (H // 4 + 14) * len(rows)))
d = ImageDraw.Draw(out)
for i, (c, img, fr) in enumerate(rows):
    out.paste(Image.fromarray(fr.clip(0, 255).astype(np.uint8)), (0, i * (H // 4 + 14) + 14))
    out.paste(Image.fromarray(img.astype(np.uint8)), (W // 4 + 10, i * (H // 4 + 14) + 14))
    d.text((2, i * (H // 4 + 14)), c.split('/')[-1], fill=255)
out.save(o.out)
