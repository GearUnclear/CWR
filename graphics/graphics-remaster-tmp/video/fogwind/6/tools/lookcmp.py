#!/usr/bin/env python
"""agent 6: look comparison of static-probe clips at the same pose.
usage: python lookcmp.py <out_prefix> ref.mkv:LABEL other.mkv:LABEL ... [--frame N]
Per clip vs the first: mean of the last 20 frames (temporal mean = the look), per region
mean RGB difference (code values), and the sharpness (rms of mean image - gauss 3 px, gauss 8 px)
to see softening. Writes <prefix>_crops.png: 2x crops of the bay/valley/far hills of each clip's
mean image stacked, and <prefix>_diff.png: |mean - ref mean| x8.
"""
import sys, subprocess, json
import numpy as np
from scipy import ndimage
from PIL import Image, ImageDraw

FF = "C:/ffmpeg/bin/ffmpeg.exe"
W, H = 1920, 1080
REGIONS = {
    "bay": (0.56, 0.70, 0.72, 0.97),
    "valley": (0.54, 0.64, 0.58, 0.68),
    "farhill": (0.45, 0.53, 0.25, 0.55),
    "haze": (0.47, 0.55, 0.62, 0.97),
    "slope": (0.56, 0.68, 0.30, 0.48),
}


def frames(path):
    p = subprocess.run([FF, "-v", "error", "-i", path, "-f", "rawvideo", "-pix_fmt", "rgb24", "-"],
                       capture_output=True, check=True)
    a = np.frombuffer(p.stdout, np.uint8)
    n = a.size // (W * H * 3)
    return a[: n * W * H * 3].reshape(n, H, W, 3)


def main():
    args = [a for a in sys.argv[1:]]
    prefix = args.pop(0)
    clips = []
    for c in args:
        path, label = c.rsplit(":", 1)
        f = frames(path)
        mean = f[-20:].astype(np.float32).mean(0)
        clips.append((label, mean))
    ref = clips[0][1]
    out = {}
    for label, mean in clips:
        row = {}
        g = mean.mean(2)
        for r, (y0, y1, x0, x1) in REGIONS.items():
            sl = (slice(int(y0 * H), int(y1 * H)), slice(int(x0 * W), int(x1 * W)))
            d = (mean[sl] - ref[sl]).reshape(-1, 3).mean(0)
            hp3 = g - ndimage.gaussian_filter(g, 3)
            hp8 = ndimage.gaussian_filter(g, 3) - ndimage.gaussian_filter(g, 8)
            row[r] = dict(dRGB=[round(float(x), 2) for x in d], absd=round(float(np.abs(mean[sl] - ref[sl]).mean()), 2),
                          sharp3=round(float(hp3[sl].std()), 3), sharp8=round(float(hp8[sl].std()), 3))
        out[label] = row
        print(label)
        for r, v in row.items():
            print("   %-8s dRGB %-22s |d| %5.2f  sharp<3px %.3f  3-8px %.3f" % (r, v["dRGB"], v["absd"], v["sharp3"], v["sharp8"]))
    json.dump(out, open(prefix + "_look.json", "w"), indent=1)
    # crops: bay+haze (x 0.62-0.97, y 0.45-0.70) and far hills (x 0.25-0.55, y 0.43-0.60)
    boxes = [(0.62, 0.45, 0.97, 0.70), (0.25, 0.43, 0.60, 0.62)]
    tiles = []
    for label, mean in clips:
        parts = []
        for (x0, y0, x1, y1) in boxes:
            c = mean[int(y0 * H):int(y1 * H), int(x0 * W):int(x1 * W)]
            parts.append(Image.fromarray(np.clip(c, 0, 255).astype(np.uint8)))
        wsum = sum(p.width for p in parts) + 8
        hmax = max(p.height for p in parts)
        t = Image.new("RGB", (wsum, hmax + 18), (0, 0, 0))
        x = 0
        for p in parts:
            t.paste(p, (x, 18)); x += p.width + 8
        ImageDraw.Draw(t).text((4, 2), label, fill=(255, 255, 0))
        tiles.append(t)
    big = Image.new("RGB", (tiles[0].width, sum(t.height for t in tiles)))
    y = 0
    for t in tiles:
        big.paste(t, (0, y)); y += t.height
    big.save(prefix + "_crops.png")
    dt = []
    for label, mean in clips[1:]:
        d = np.clip(np.abs(mean - ref) * 8, 0, 255).astype(np.uint8)
        im = Image.fromarray(d).resize((W // 2, H // 2))
        ImageDraw.Draw(im).text((4, 2), label + " - " + clips[0][0] + " x8", fill=(255, 255, 0))
        dt.append(im)
    if dt:
        big = Image.new("RGB", (W // 2, (H // 2) * len(dt)))
        for i, im in enumerate(dt):
            big.paste(im, (0, i * (H // 2)))
        big.save(prefix + "_diff.png")


if __name__ == "__main__":
    main()
