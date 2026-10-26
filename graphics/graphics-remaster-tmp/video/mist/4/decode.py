"""Agent 4: decode a clip to a small uint8 RGB npy (480x270) for offline analysis.

usage: python decode.py <clip.mkv> <out.npy> [W H]
"""
import subprocess, sys
import numpy as np

FF = "C:/ffmpeg/bin/ffmpeg.exe"
src, out = sys.argv[1], sys.argv[2]
W = int(sys.argv[3]) if len(sys.argv) > 3 else 480
H = int(sys.argv[4]) if len(sys.argv) > 4 else 270
p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-vf", f"scale={W}:{H}:flags=area",
                    "-pix_fmt", "rgb24", "-f", "rawvideo", "-"], capture_output=True, check=True)
x = np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W, 3)
np.save(out, x)
print(x.shape, "->", out)
