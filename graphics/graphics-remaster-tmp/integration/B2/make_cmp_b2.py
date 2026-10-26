"""Comparison sheets for the B2 lane.

    python make_cmp_b2.py <after run> <before run> <out dir>

Per view: cmp_<view>.png, a 2x2 grid at half size (classic | before step on
the base build / off | on of the B2 build), and crop_<view>.png, the hull or
crown region at full size in the same order, side by side.
"""
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

CROPS = {
    "lowsun_fp": (150, 380, 1600, 720),
    "lowsun_box": (330, 420, 1330, 660),
    "dawn_box": (330, 420, 1330, 660),
    "dawn_box5": (330, 420, 1330, 660),
    "forest_edge_golden": (500, 120, 1800, 520),
    "noon_fields": (0, 300, 1920, 800),
    "sinai_lowsun_fp": (150, 380, 1650, 720),
    "sinai_box": (330, 420, 1330, 660),
    "sinai_dusk_box": (330, 420, 1330, 660),
}


def font(size):
    try:
        return ImageFont.truetype("arial.ttf", size)
    except OSError:
        return ImageFont.load_default()


def label(im, text):
    d = ImageDraw.Draw(im)
    f = font(22)
    w = d.textlength(text, font=f)
    d.rectangle((6, 6, 18 + w, 36), fill=(0, 0, 0))
    d.text((12, 9), text, fill=(255, 255, 255), font=f)
    return im


def main():
    after, before, out = Path(sys.argv[1]), Path(sys.argv[2]), Path(sys.argv[3])
    out.mkdir(parents=True, exist_ok=True)
    for view, box in CROPS.items():
        paths = [("classic (original look)", after / f"{view}__classic.png"),
                 ("before B2 (base build)", before / f"{view}__on.png"),
                 ("B2 off (lowSunRig=0, crownCanopy=0)", after / f"{view}__off.png"),
                 ("B2 on", after / f"{view}__on.png")]
        if not all(p.exists() for _, p in paths):
            print("skip", view)
            continue
        ims = [(t, Image.open(p).convert("RGB")) for t, p in paths]
        w, h = ims[0][1].size
        grid = Image.new("RGB", (w, h), (0, 0, 0))
        for i, (t, im) in enumerate(ims):
            small = label(im.resize((w // 2, h // 2), Image.LANCZOS), t)
            grid.paste(small, ((i % 2) * (w // 2), (i // 2) * (h // 2)))
        grid.save(out / f"cmp_{view}.png")
        cw, ch = box[2] - box[0], box[3] - box[1]
        strip = Image.new("RGB", (cw * 2, ch * 2), (0, 0, 0))
        for i, (t, im) in enumerate(ims):
            strip.paste(label(im.crop(box), t), ((i % 2) * cw, (i // 2) * ch))
        strip.save(out / f"crop_{view}.png")
        print(view)


if __name__ == "__main__":
    main()
