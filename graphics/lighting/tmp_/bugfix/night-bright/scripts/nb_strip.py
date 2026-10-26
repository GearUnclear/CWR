"""python nb_strip.py out.png "Title" label1=path1 label2=path2 ... [--scale 0.75]"""
import sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont


def font(sz):
    for n in ("arialbd.ttf", "arial.ttf"):
        p = Path("C:/Windows/Fonts") / n
        if p.exists():
            return ImageFont.truetype(str(p), sz)
    return ImageFont.load_default()


def main():
    args = sys.argv[1:]
    scale = 1.0
    if "--scale" in args:
        i = args.index("--scale")
        scale = float(args[i + 1])
        del args[i:i + 2]
    out, title, pairs = args[0], args[1], args[2:]
    panels = []
    for p in pairs:
        label, path = p.split("=", 1)
        im = Image.open(path).convert("RGB")
        if scale != 1.0:
            im = im.resize((int(im.size[0] * scale), int(im.size[1] * scale)), Image.LANCZOS)
        d = ImageDraw.Draw(im)
        f = font(18)
        b = d.textbbox((10, 10), label, font=f)
        d.rectangle([b[0] - 5, b[1] - 3, b[2] + 5, b[3] + 3], fill=(0, 0, 0))
        d.text((10, 10), label, font=f, fill=(255, 255, 255))
        panels.append(im)
    w, h = panels[0].size
    strip = Image.new("RGB", (w * len(panels) + 6 * (len(panels) - 1), h + 34), (24, 24, 24))
    d = ImageDraw.Draw(strip)
    d.text((10, 6), title, font=font(20), fill=(255, 255, 255))
    for i, im in enumerate(panels):
        strip.paste(im, (i * (w + 6), 34))
    strip.save(out)


if __name__ == "__main__":
    main()
