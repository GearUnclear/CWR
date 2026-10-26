"""Agent 7: look check. Time-mean of two static clips (frames a..b), per-region mean colour difference
(code values, linear-ish sRGB codes) and a side-by-side PNG (mean A | mean B | |A-B| x8) of a crop.

python lookdiff.py A.mkv B.mkv out.png [--from 15] [--crop W:H:X:Y]
"""
import argparse, subprocess, json
import numpy as np
from PIL import Image

FF = "C:/ffmpeg/bin/ffmpeg.exe"
REGIONS = {
    "bay": (0.56, 0.70, 0.72, 0.97), "valley": (0.54, 0.64, 0.58, 0.68), "farhill": (0.45, 0.53, 0.25, 0.55),
    "haze": (0.47, 0.55, 0.62, 0.97), "slope": (0.56, 0.68, 0.30, 0.48), "sky": (0.05, 0.40, 0.05, 0.95),
}


def mean_rgb(src, a, b):
    p = subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-i", src, "-f", "rawvideo", "-pix_fmt", "rgb24", "-"],
                       capture_output=True, check=True)
    x = np.frombuffer(p.stdout, np.uint8).reshape(-1, 1080, 1920, 3)[a:b]
    acc = np.zeros((1080, 1920, 3), np.float64)
    for f in x:
        acc += f
    return acc / len(x)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("a"); ap.add_argument("b"); ap.add_argument("out")
    ap.add_argument("--from", dest="fa", type=int, default=15)
    ap.add_argument("--to", dest="fb", type=int, default=None)
    ap.add_argument("--crop", default="1000:400:900:420")
    o = ap.parse_args()
    A = mean_rgb(o.a, o.fa, o.fb); B = mean_rgb(o.b, o.fa, o.fb)
    res = {}
    for k, (y0, y1, x0, x1) in REGIONS.items():
        ra = A[int(y0 * 1080):int(y1 * 1080), int(x0 * 1920):int(x1 * 1920)]
        rb = B[int(y0 * 1080):int(y1 * 1080), int(x0 * 1920):int(x1 * 1920)]
        res[k] = dict(meanA=[round(v, 2) for v in ra.mean(axis=(0, 1))], dmean=[round(v, 2) for v in (rb - ra).mean(axis=(0, 1))],
                      dabs=round(float(np.abs(rb - ra).mean()), 3))
    print(json.dumps(res))
    w, h, x, y = (int(v) for v in o.crop.split(":"))
    ca = A[y:y + h, x:x + w]; cb = B[y:y + h, x:x + w]
    d = np.clip(np.abs(cb - ca) * 8, 0, 255)
    img = np.vstack([ca, cb, d])
    Image.fromarray(np.clip(img, 0, 255).astype(np.uint8)).save(o.out)


if __name__ == "__main__":
    main()
