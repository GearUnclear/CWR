"""Low-frequency flicker: block-mean luminance, signed temporal 2nd difference, RMS per block."""
import subprocess, sys
import numpy as np
from PIL import Image

W, H, B = 480, 270, 15  # 32 x 18 blocks
FF = "C:/ffmpeg/bin/ffmpeg.exe"


def frames(path):
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", path,
                        "-vf", f"scale={W}:{H}:flags=area,format=gray", "-f", "rawvideo", "-"],
                       capture_output=True, check=True)
    return np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W).astype(np.float32)


def blocks(a):
    gy, gx = H // B, W // B
    return a[:, :gy * B, :gx * B].reshape(len(a), gy, B, gx, B).mean(axis=(2, 4))


out = sys.argv[3]
res = {}
for name, path in (("ud", sys.argv[1]), ("vanilla", sys.argv[2])):
    b = blocks(frames(path))
    d2 = b[1:-1] - 0.5 * (b[:-2] + b[2:])
    rms = np.sqrt((d2 ** 2).mean(axis=0))
    res[name] = (b, d2, rms)
    print(f"{name}: block d2 rms mean {rms.mean():.3f} max {rms.max():.3f} at {np.unravel_index(rms.argmax(), rms.shape)}")
    # count of sign-alternating spikes > 0.5 per block
    spikes = (np.abs(d2) > 0.5).sum(axis=0)
    print(f"  blocks with >10 spikes (|d2|>0.5): {(spikes > 10).sum()}  total spikes {spikes.sum()}")
    per = np.abs(d2).reshape(len(d2), -1).max(axis=1)
    top = np.argsort(per)[-10:][::-1]
    print("  worst frames (max block |d2|):", [(int(i) + 1, round(float(per[i]), 2)) for i in top])

r_ud, r_va = res["ud"][2], res["vanilla"][2]
print("ud rms grid (x100):")
for row in r_ud:
    print(" ".join(f"{int(v * 100):3d}" for v in row))
print("vanilla rms grid (x100):")
for row in r_va:
    print(" ".join(f"{int(v * 100):3d}" for v in row))
# heat overlay on the ud mid frame
mid = Image.open(sys.argv[4]).convert("RGB").resize((960, 540))
heat = np.clip(r_ud / 1.0, 0, 1)
hm = Image.fromarray((heat * 255).astype(np.uint8)).resize((960, 540), Image.NEAREST)
red = Image.new("RGB", (960, 540), (255, 0, 0))
over = Image.composite(red, mid, hm.point(lambda v: int(v * 0.7)))
over.save(out)
np.save(out.replace(".png", "_d2.npy"), res["ud"][1])
