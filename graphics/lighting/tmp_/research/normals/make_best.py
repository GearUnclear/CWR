"""Three-way comparison sheets for the normals track (research only).

    python make_best.py <run_dir> <view> <x0,y0,x1,y1> <out.png> [--proto v6mean] [--scale 2]

Top row: Classic | Remastered (as shipped) | Remastered + prototype, full frame.
Bottom row: the same crop of each, scaled. GPU frame ms (triGpuTimes average
over the settle frames, shared GPU, noisy) on every full-frame panel.
"""
import argparse
from pathlib import Path

from PIL import Image, ImageDraw

import compare as cmp

TITLES = {
    "classic": "Classic",
    "cur": "Remastered (as shipped)",
    "v6mean": "+ albedo relief V6 (mean-preserving)",
    "v8cav": "+ albedo relief V8",
    "v9bold": "+ albedo relief x2 V9",
    "remastered": "Remastered + late-VB fix only",
}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("view")
    ap.add_argument("crop")
    ap.add_argument("out")
    ap.add_argument("--proto", default="v6mean")
    ap.add_argument("--scale", type=float, default=2.0)
    a = ap.parse_args()
    run = Path(a.run)
    _best, gpu = cmp.collect(run)
    crop = tuple(int(x) for x in a.crop.split(","))
    labels = ["classic", "cur", a.proto]
    fulls = [Image.open(run / "shots" / f"{a.view}__{l}.png").convert("RGB") for l in labels]
    crops = []
    for im in fulls:
        c = im.crop(crop)
        crops.append(c.resize((int(c.width * a.scale), int(c.height * a.scale)), Image.LANCZOS))
    W, H = fulls[0].size
    cw, ch = crops[0].size
    gap, title = 8, 36
    cols_w = max(W, cw)
    out = Image.new("RGB", (3 * cols_w + 2 * gap, title + H + gap + 26 + ch), (24, 24, 24))
    d = ImageDraw.Draw(out)
    d.text((10, 8), f"{a.view}: Classic | Remastered as shipped | + prototype (same view, same sim time). Bottom: crop {crop} x{a.scale:g}",
           font=cmp.font(19), fill=(235, 235, 235))
    for i, (lab, im, c) in enumerate(zip(labels, fulls, crops)):
        x = i * (cols_w + gap)
        out.paste(im, (x, title))
        cmp.label_box(d, (x + 8, title + 8), TITLES.get(lab, lab), cmp.font(16))
        g = gpu.get((a.view, lab), {}).get("frame")
        if g is not None:
            cmp.label_box(d, (x + 8, title + H - 28), f"GPU frame {g:.2f} ms (one sample, shared GPU)", cmp.font(15))
        y = title + H + gap
        d.text((x + 4, y + 2), "crop", font=cmp.font(15), fill=(200, 200, 200))
        out.paste(c, (x, y + 24))
    out.save(a.out)
    print(a.out)


if __name__ == "__main__":
    main()
