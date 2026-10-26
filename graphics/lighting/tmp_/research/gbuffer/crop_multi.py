"""Crop the same box out of several shots (possibly from different runs) and
lay them side by side, labelled, scaled, with GPU frame ms when known.

usage: python crop_multi.py <out.png> <title> x0,y0,x1,y1 <scale> <run>|<view>__<label>|<caption> [...]
Each panel spec is "run_dir|shot_stem|caption". GPU numbers come from
<run_dir>/timings_gb.json (written by collect_gb.py) when present.
Also prints the mean display luminance of the crop per panel.
"""
import json
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont, ImageStat


def font(size):
    for name in ("arialbd.ttf", "arial.ttf"):
        p = Path("C:/Windows/Fonts") / name
        if p.exists():
            return ImageFont.truetype(str(p), size)
    return ImageFont.load_default()


def gpu_for(run, stem):
    j = Path(run) / "timings_gb.json"
    if not j.exists() or "__" not in stem:
        return None
    view, mode = stem.split("__", 1)
    for r in json.loads(j.read_text(encoding="utf-8")):
        if r.get("view") == view and r.get("mode") == mode:
            return r
    return None


def main():
    out, title, box, scale = sys.argv[1], sys.argv[2], tuple(int(v) for v in sys.argv[3].split(",")), float(sys.argv[4])
    specs = [s.split("|") for s in sys.argv[5:]]
    w = int((box[2] - box[0]) * scale)
    h = int((box[3] - box[1]) * scale)
    gap, top, cap_h = 6, 34, 30
    sheet = Image.new("RGB", (len(specs) * w + (len(specs) - 1) * gap, top + h), (20, 20, 20))
    d = ImageDraw.Draw(sheet)
    d.text((8, 6), title, font=font(20), fill=(235, 235, 235))
    for i, (run, stem, cap) in enumerate(specs):
        p = Path(run) / f"{stem}.png"
        x0 = i * (w + gap)
        if not p.exists():
            d.rectangle([x0, top, x0 + w, top + h], fill=(70, 0, 0))
            d.text((x0 + 8, top + 8), "MISSING " + p.name, font=font(16), fill=(255, 255, 255))
            print("missing", p)
            continue
        im = Image.open(p).convert("RGB").crop(box)
        lum = ImageStat.Stat(im.convert("L")).mean[0]
        sheet.paste(im.resize((w, h), Image.LANCZOS), (x0, top))
        r = gpu_for(run, stem)
        txt = cap
        b = d.textbbox((x0 + 8, top + 6), txt, font=font(16))
        d.rectangle([b[0] - 4, b[1] - 3, b[2] + 4, b[3] + 3], fill=(0, 0, 0))
        d.text((x0 + 8, top + 6), txt, font=font(16), fill=(255, 255, 255))
        if r and r.get("frame") is not None:
            g = f"GPU frame {r['frame']:.2f} ms (post {r.get('post') or 0:.2f})"
            b = d.textbbox((x0 + 8, top + h - cap_h), g, font=font(14))
            d.rectangle([b[0] - 4, b[1] - 3, b[2] + 4, b[3] + 3], fill=(0, 0, 0))
            d.text((x0 + 8, top + h - cap_h), g, font=font(14), fill=(255, 255, 255))
        print(f"{cap}: mean display luma {lum:.1f}")
    sheet.save(out)
    print(out)


if __name__ == "__main__":
    main()
