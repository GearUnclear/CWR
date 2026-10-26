"""Build a BEST_<view>.png: Classic | Remastered (current) | Remastered + prototype,
full frames on top and 2x crops of one region below, GPU frame ms on each panel.

    python best.py <cmp_dir> <view> <variant> "<variant label>" <x0,y0,x1,y1> <out.png>

<cmp_dir> is a collect.py output folder (copied shots + timings.csv).
"""
import csv
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


def font(size):
    for f in ("arialbd.ttf", "arial.ttf", "DejaVuSans-Bold.ttf"):
        try:
            return ImageFont.truetype(f, size)
        except OSError:
            pass
    return ImageFont.load_default()


def label(im, title, ms, fb, fs):
    im = im.copy()
    d = ImageDraw.Draw(im)
    d.rectangle([6, 6, 16 + d.textlength(title, font=fb), 36], fill=(0, 0, 0))
    d.text((11, 8), title, fill=(255, 255, 255), font=fb)
    if ms:
        d.rectangle([6, im.height - 30, 16 + d.textlength(ms, font=fs), im.height - 6], fill=(0, 0, 0))
        d.text((11, im.height - 28), ms, fill=(255, 255, 255), font=fs)
    return im


def main():
    cmp_dir, view, var, vlabel, box, out = sys.argv[1:7]
    cmp_dir = Path(cmp_dir)
    box = tuple(int(c) for c in box.split(","))
    tim = {}
    with open(cmp_dir / "timings.csv") as fh:
        for r in csv.DictReader(fh):
            tim[(r["view"], r["mode"])] = r
    modes = [("classic", "Classic"), ("v0", "Remastered (current)"), (var, f"Remastered + {vlabel}")]
    fb, fs = font(20), font(15)
    tops, crops = [], []
    for m, t in modes:
        im = Image.open(cmp_dir / f"{view}__{m}.png").convert("RGB")
        r = tim.get((view, m))
        ms = f"GPU frame {float(r['gpu_frame_ms']):.2f} ms, world {float(r['gpu_world_ms']):.2f} ms" if r and r[
            "gpu_frame_ms"] else ""
        tops.append(label(im, t, ms, fb, fs))
        cw, ch = box[2] - box[0], box[3] - box[1]
        c = im.crop(box).resize((cw * 2, ch * 2), Image.LANCZOS)
        crops.append(label(c, f"{t}, 2x crop", "", fs, fs))
    W = sum(i.width for i in tops) + 12
    H = 40 + tops[0].height + 6 + crops[0].height
    sheet = Image.new("RGB", (W, H), (24, 24, 24))
    d = ImageDraw.Draw(sheet)
    d.text((10, 8), f"{view}: Classic | Remastered (current) | Remastered + {vlabel}   (crop box {box})",
           fill=(240, 240, 240), font=font(20))
    x = 0
    for i in tops:
        sheet.paste(i, (x, 40))
        x += i.width + 6
    x = 0
    for c in crops:
        sheet.paste(c, (x, 40 + tops[0].height + 6))
        x += c.width + 6
    # outline the crop region on the top panels
    for k in range(3):
        ox = k * (tops[0].width + 6)
        d.rectangle([ox + box[0], 40 + box[1], ox + box[2], 40 + box[3]], outline=(255, 210, 0), width=2)
    sheet.save(out)
    print("wrote", out)


if __name__ == "__main__":
    main()
