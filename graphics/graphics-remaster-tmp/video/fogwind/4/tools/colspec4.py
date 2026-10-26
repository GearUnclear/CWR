import subprocess, sys
import numpy as np
from scipy import ndimage
FF = "C:/ffmpeg/bin/ffmpeg.exe"
x, y, w, h = [int(v) for v in sys.argv[1].split(",")]
for c in sys.argv[2:]:
    p, lab = c.rsplit(":", 1)
    raw = subprocess.run([FF, "-v", "error", "-i", p, "-vf", "select='between(n,15,119)'", "-vsync", "0", "-f", "rawvideo", "-pix_fmt", "gray", "-"], capture_output=True, check=True).stdout
    n = len(raw) // (1920 * 1080)
    m = np.frombuffer(raw[: n * 1920 * 1080], np.uint8).reshape(n, 1080, 1920).astype(np.float32).mean(0)
    col = m[y:y + h, x:x + w].mean(0)
    hp = col - ndimage.uniform_filter1d(col, 31, mode="nearest")
    hp = hp[15:-15] * np.hanning(len(hp) - 30)
    sp = np.abs(np.fft.rfft(hp)) ** 2
    f = np.fft.rfftfreq(len(hp))
    top = np.argsort(sp)[-4:][::-1]
    print(lab, "top periods px:", ", ".join(f"{1 / f[i]:.1f} ({sp[i] / sp.sum():.2f})" for i in top if f[i] > 0))
