"""d2 x20 images of a clip's mist region for frames a..b (0-based index in the clip file), plus a 2x crop of one frame.
usage: python d2look.py <clip.mkv> <first_index> <count> <out_prefix>"""
import subprocess, sys
import numpy as np
from PIL import Image
FF = "C:/ffmpeg/bin/ffmpeg.exe"
clip, a, cnt, out = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), sys.argv[4]
vf = "select='between(n," + str(a) + "," + str(a + cnt - 1) + ")',crop=960:360:900:500,format=rgb24"
p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", clip, "-vf", vf, "-vsync", "0", "-f", "rawvideo", "-"], capture_output=True, check=True)
x = np.frombuffer(p.stdout, np.uint8).reshape(-1, 360, 960, 3).astype(np.float32)
rows = []
for i in range(1, len(x) - 1):
    d = (x[i] - 0.5 * (x[i - 1] + x[i + 1])).mean(axis=2)
    rows.append(np.clip(128 + 20 * d, 0, 255).astype(np.uint8))
    print(f"index {a + i}: region d2 mean {d.mean():+.2f} rms {np.sqrt((d ** 2).mean()):.2f}")
Image.fromarray(np.concatenate(rows, axis=0)).save(out + "_d2x20.png")
mid = x[len(x) // 2][60:300, 240:720]
Image.fromarray(mid.astype(np.uint8)).resize((960, 480), Image.NEAREST).save(out + "_crop2x.png")
