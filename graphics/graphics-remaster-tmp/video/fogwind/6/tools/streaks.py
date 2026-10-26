# agent 6: static cell-column streaks check: 20-frame mean, crop, high-pass (mean - gauss 6 px) x10, column profile power at the cell period
import sys, subprocess, numpy as np
from scipy import ndimage
from PIL import Image, ImageDraw
FF = "C:/ffmpeg/bin/ffmpeg.exe"; W, H = 1920, 1080
out = sys.argv[1]; x0, y0, x1, y1 = [int(v) for v in sys.argv[2].split(",")]
tiles = []
for spec in sys.argv[3:]:
    path, lab = spec.rsplit(":", 1)
    p = subprocess.run([FF, "-v", "error", "-i", path, "-vf", "crop=%d:%d:%d:%d,format=gray" % (x1 - x0, y1 - y0, x0, y0), "-f", "rawvideo", "-pix_fmt", "gray", "-"], capture_output=True, check=True)
    a = np.frombuffer(p.stdout, np.uint8).reshape(-1, y1 - y0, x1 - x0).astype(np.float32)
    m = a[-20:].mean(0)
    hp = m - ndimage.gaussian_filter(m, 6)
    prof = hp.mean(0)  # column profile (vertical streaks)
    f = np.abs(np.fft.rfft(prof - prof.mean()))
    n = len(prof); k12 = round(n / 12.0)
    print("%-10s hp rms %.3f  column-profile rms %.3f  power at 12px period %.3f (median %.3f)" % (lab, hp.std(), prof.std(), f[k12 - 1:k12 + 2].max(), np.median(f)))
    im = Image.fromarray(np.clip(128 + hp * 10, 0, 255).astype(np.uint8)).resize(((x1 - x0) * 2, (y1 - y0) * 2), Image.NEAREST)
    ImageDraw.Draw(im).text((4, 2), lab + " hp x10", fill=255)
    tiles.append(im)
big = Image.new("L", (tiles[0].width, sum(t.height + 4 for t in tiles)))
y = 0
for t in tiles:
    big.paste(t, (0, y)); y += t.height + 4
big.save(out)
