"""Shimmer strips from the pan captures.

    python pan_strip.py <run_dir> <view> <out.png> --variants panH-rem panH-best ... [--crop x0,y0,x1,y1] [--zoom N]
                        [--gif out.gif]

For each variant: the 4 pan frames (heading stepped by a sub-pixel 0.025 deg,
sim clock re-pinned, so only the camera moves) side by side, then the
amplified absolute difference of consecutive frames. A well-filtered image
changes little under a sub-pixel move; aliasing (crawling edges, popping
leaves) shows as bright speckle. The mean frame-to-frame difference (0-255
scale) of the crop is printed on each row as the shimmer number.
"""
import argparse
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont


def font(size):
    for name in ("arialbd.ttf", "arial.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            pass
    return ImageFont.load_default()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("view")
    ap.add_argument("out")
    ap.add_argument("--variants", nargs="+", required=True)
    ap.add_argument("--labels", nargs="*", default=None)
    ap.add_argument("--crop", default="")
    ap.add_argument("--zoom", type=int, default=1)
    ap.add_argument("--frames", type=int, default=4)
    ap.add_argument("--gain", type=float, default=4.0)
    ap.add_argument("--gif", default="")
    a = ap.parse_args()
    run = Path(a.run)
    rows = []
    gif_frames = []
    stats = []
    for vi, var in enumerate(a.variants):
        frames = []
        for i in range(a.frames):
            im = Image.open(run / f"{a.view}__{var}-{i}.png").convert("RGB")
            if a.crop:
                im = im.crop(tuple(map(int, a.crop.split(","))))
            frames.append(np.asarray(im).astype(np.float32))
        diffs = [np.abs(frames[i + 1] - frames[i]).mean(axis=2) for i in range(len(frames) - 1)]
        shimmer = float(np.mean([d.mean() for d in diffs]))
        stats.append((var, shimmer))
        tiles = [Image.fromarray(f.astype(np.uint8)) for f in frames[:2]]
        tiles += [Image.fromarray(np.clip(d * a.gain, 0, 255).astype(np.uint8)).convert("RGB") for d in diffs[:2]]
        if a.zoom > 1:
            tiles = [t.resize((t.width * a.zoom, t.height * a.zoom), Image.NEAREST) for t in tiles]
        label = (a.labels[vi] if a.labels else var)
        rows.append((label, shimmer, tiles))
        gif_frames.append([Image.fromarray(f.astype(np.uint8)) for f in frames])
    tw, th = rows[0][2][0].size
    gap, head, lab = 6, 40, 30
    W = 4 * tw + 3 * gap
    H = head + len(rows) * (th + lab + gap)
    out = Image.new("RGB", (W, H), (24, 24, 24))
    d = ImageDraw.Draw(out)
    d.text((10, 8), f"{a.view}: sub-pixel pan (0.025 deg/frame). Cols: frame 0, frame 1, |f1-f0| x{a.gain:g}, |f2-f1| x{a.gain:g}",
           fill=(230, 230, 230), font=font(18))
    y = head
    for label, shimmer, tiles in rows:
        d.text((8, y + 4), f"{label}    mean frame-to-frame change {shimmer:.2f} / 255", fill=(255, 255, 255), font=font(18))
        y += lab
        for i, t in enumerate(tiles):
            out.paste(t, (i * (tw + gap), y))
        y += th + gap
    out.save(a.out)
    for var, sh in stats:
        print(f"{var}: shimmer {sh:.3f}")
    if a.gif:
        # variants side by side, animated over the pan frames
        n = len(gif_frames)
        fw, fh = gif_frames[0][0].size
        seq = []
        for i in range(a.frames):
            canvas = Image.new("RGB", (n * fw + (n - 1) * gap, fh + lab), (24, 24, 24))
            dd = ImageDraw.Draw(canvas)
            for vi in range(n):
                canvas.paste(gif_frames[vi][i], (vi * (fw + gap), lab))
                dd.text((vi * (fw + gap) + 6, 6), rows[vi][0], fill=(255, 255, 255), font=font(16))
            if a.zoom > 1:
                canvas = canvas.resize((canvas.width * a.zoom, canvas.height * a.zoom), Image.NEAREST)
            seq.append(canvas)
        seq += seq[-2:0:-1]
        seq[0].save(a.gif, save_all=True, append_images=seq[1:], duration=180, loop=0)
        print("gif", a.gif)
    print(a.out)


if __name__ == "__main__":
    main()
