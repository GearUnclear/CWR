import subprocess, sys
import numpy as np

FF = "C:/ffmpeg/bin/ffmpeg.exe"
V2 = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2"
clip = sys.argv[1]
src = f"{V2}/ud/{clip}/{clip}__ud.mkv"
w, h = 192, 108
p = subprocess.run([FF, "-v", "error", "-i", src, "-vf", f"scale={w}:{h}", "-f", "rawvideo",
                    "-pix_fmt", "rgb24", "-"], capture_output=True, check=True)
a = np.frombuffer(p.stdout, np.uint8).reshape(-1, h, w, 3).astype(np.float32)
lum = a.mean(axis=3)
mean = lum.mean(axis=(1, 2))
p99 = np.percentile(lum.reshape(len(lum), -1), 99, axis=1)
bright = (lum > 200).mean(axis=(1, 2))
warm = ((a[..., 0] - a[..., 2]) > 40).mean(axis=(1, 2))
print("frames", len(a))
for i in range(0, len(a), int(sys.argv[2]) if len(sys.argv) > 2 else 10):
    print(f"{i:4d} mean={mean[i]:6.1f} p99={p99[i]:6.1f} bright={bright[i]*100:5.2f}% warm={warm[i]*100:5.2f}%")
