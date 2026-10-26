"""Quick flicker map: temporal second difference per tile, ud vs vanilla."""
import subprocess, sys
import numpy as np

W, H = 480, 270
FF = "C:/ffmpeg/bin/ffmpeg.exe"


def frames(path):
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", path,
                        "-vf", f"scale={W}:{H}:flags=area,format=gray", "-f", "rawvideo", "-"],
                       capture_output=True, check=True)
    a = np.frombuffer(p.stdout, np.uint8).reshape(-1, H, W).astype(np.float32)
    return a


def report(name, a):
    d2 = np.abs(a[2:] - 0.5 * (a[1:-1] * 2 - a[:-2] - a[2:]) * 0 - (a[:-2] + a[2:]) * 0.5 - (a[1:-1] - a[1:-1]))
    # second difference: f(t) - mean(f(t-1), f(t+1))
    d2 = np.abs(a[1:-1] - 0.5 * (a[:-2] + a[2:]))
    d1 = np.abs(a[1:] - a[:-1])
    gy, gx = 6, 8
    th, tw = H // gy, W // gx
    t2 = d2[:, :gy * th, :gx * tw].reshape(len(d2), gy, th, gx, tw).mean(axis=(2, 4))
    t1 = d1[:, :gy * th, :gx * tw].reshape(len(d1), gy, th, gx, tw).mean(axis=(2, 4))
    print(f"== {name}: frames {len(a)}  mean d1 {d1.mean():.3f}  mean d2 {d2.mean():.3f}")
    print("tile mean d2 (rows top->bottom):")
    for r in t2.mean(axis=0):
        print("  " + " ".join(f"{v:5.2f}" for v in r))
    print("tile mean d1:")
    for r in t1.mean(axis=0):
        print("  " + " ".join(f"{v:5.2f}" for v in r))
    per = d2.reshape(len(d2), -1).mean(axis=1)
    top = np.argsort(per)[-12:][::-1]
    print("worst frames by d2:", [(int(i) + 1, round(float(per[i]), 3)) for i in top])
    # luminance band: sky (top third) mean per frame, its d2
    sky = a[:, : H // 3].mean(axis=(1, 2))
    sd2 = np.abs(sky[1:-1] - 0.5 * (sky[:-2] + sky[2:]))
    print(f"top-third mean Y d2: mean {sd2.mean():.3f} max {sd2.max():.3f} at f{int(sd2.argmax()) + 1}")
    return t2


ud = frames(sys.argv[1])
va = frames(sys.argv[2])
t_ud = report("ud", ud)
t_va = report("vanilla", va)
r = t_ud.mean(axis=0) / np.maximum(t_va.mean(axis=0), 0.05)
print("ratio ud/vanilla d2 per tile:")
for row in r:
    print("  " + " ".join(f"{v:5.1f}" for v in row))
