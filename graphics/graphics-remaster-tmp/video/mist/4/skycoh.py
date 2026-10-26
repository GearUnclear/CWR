"""Agent 4: the sky's coherent (frame-global) d2 vs its block d2, and the rock's, per clip."""
import subprocess, sys
import numpy as np
FF = "C:/ffmpeg/bin/ffmpeg.exe"
W, H, B = 480, 270, 15
a0 = 30
for item in sys.argv[1:]:
    name, src = item.split("=", 1)
    if src.endswith(".npy"):
        x = np.load(src).astype(np.float32)
        x = (0.299 * x[..., 0] + 0.587 * x[..., 1] + 0.114 * x[..., 2])
    else:
        p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-vf", f"scale={W}:{H}:flags=area,format=gray",
                            "-f", "rawvideo", "-"], capture_output=True, check=True)
        x = np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W).astype(np.float32)
    x = x[a0:]
    T = len(x)
    blk = x[:, :H // B * B, :W // B * B].reshape(T, H // B, B, W // B, B).mean(axis=(2, 4))
    lb = np.log2(np.maximum(blk, 1))
    d2 = blk[1:-1] - 0.5 * (blk[:-2] + blk[2:])
    l2 = lb[1:-1] - 0.5 * (lb[:-2] + lb[2:])
    sky = d2[:, 0:6]; skyc = sky.mean(axis=(1, 2))
    rock = d2[:, 15:18, 2:13]; rockc = rock.mean(axis=(1, 2))
    lsky = l2[:, 0:6].mean(axis=(1, 2)); lrock = l2[:, 15:18, 2:13].mean(axis=(1, 2))
    print("%-8s sky block rms %.3f coherent %.3f | rock block %.3f coherent %.3f | corr(sky coh, rock coh) %+.2f | log2 coh x1000 sky %.2f rock %.2f" % (
        name, np.sqrt((sky ** 2).mean()), np.sqrt((skyc ** 2).mean()), np.sqrt((rock ** 2).mean()), np.sqrt((rockc ** 2).mean()),
        np.corrcoef(skyc, rockc)[0, 1], np.sqrt((lsky ** 2).mean()) * 1000, np.sqrt((lrock ** 2).mean()) * 1000))
