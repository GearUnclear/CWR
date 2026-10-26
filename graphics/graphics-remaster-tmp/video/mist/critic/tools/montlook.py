import subprocess, sys
import numpy as np
from PIL import Image
FF = "C:/ffmpeg/bin/ffmpeg.exe"
rows = []
for src in sys.argv[1:3]:
    r = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-ss", "20.40", "-i", src, "-frames:v", "6",
                        "-vf", "format=gray", "-f", "rawvideo", "-"], capture_output=True, check=True)
    x = np.frombuffer(r.stdout, np.uint8).reshape(-1, 1080, 1920).astype(np.float32)[:, 430:900, 900:1920]
    tiles = []
    for i in range(1, 5):
        d = x[i] - 0.5 * (x[i - 1] + x[i + 1])
        im = np.clip(128 + 20 * d, 0, 255).astype(np.uint8)
        tiles.append(np.pad(np.asarray(Image.fromarray(im).resize((510, 235), Image.BOX)), 3))
    rows.append(np.concatenate(tiles, 1))
Image.fromarray(np.concatenate(rows, 0)).save(sys.argv[3])
