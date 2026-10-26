"""Before | after (| no-shafts reference) comparison strips and crops."""
import sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

root = Path(sys.argv[1])

def font(s):
    for f in ("C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/arial.ttf"):
        try:
            return ImageFont.truetype(f, s)
        except OSError:
            pass
    return ImageFont.load_default()

def label(d, xy, text, f):
    x, y = xy
    b = d.textbbox((x, y), text, font=f)
    d.rectangle([b[0] - 4, b[1] - 3, b[2] + 4, b[3] + 3], fill=(0, 0, 0))
    d.text((x, y), text, font=f, fill=(240, 240, 240))

def strip(view, panels, title, out, crop=None, scale=1):
    ims = []
    for lab, p in panels:
        im = Image.open(p).convert("RGB")
        if crop:
            im = im.crop(crop)
        if scale != 1:
            im = im.resize((im.width * scale, im.height * scale), Image.LANCZOS)
        ims.append((lab, im))
    w, h = ims[0][1].size
    gap, th = 8, 34
    canvas = Image.new("RGB", (len(ims) * w + (len(ims) - 1) * gap, h + th), (24, 24, 24))
    d = ImageDraw.Draw(canvas)
    d.text((8, 6), title, font=font(18), fill=(235, 235, 235))
    for i, (lab, im) in enumerate(ims):
        x = i * (w + gap)
        canvas.paste(im, (x, th))
        label(d, (x + 10, th + 8), lab, font(17))
    canvas.save(root / out)
    print(root / out)

B, A = root / "before", root / "after"
views = {
    "lowsun_fp": "lowsun_fp, Abel, sun 12.7 deg, 35 deg left: Jeep 10 m, M113 16 m, soldier 7 m",
    "sinai_lowsun_fp": "sinai_lowsun_fp, Sinai + @LoBo, sun 12.2 deg: HMMWV 10 m, Zelda 16 m, rifleman 7 m",
    "sunrise_ridge": "sunrise_ridge, Abel, sun 5.1 deg, 4 deg right of centre",
}
for v, t in views.items():
    strip(v, [("BEFORE (branch tip)", B / f"{v}__remastered.png"), ("AFTER (fix)", A / f"{v}__remastered.png"),
              ("reference: shafts off", A / f"{v}_noshafts__remastered.png")],
          t + " | before | after | shafts off", f"cmp_{v}.png")
crops = {
    "lowsun_fp": (20, 260, 520, 460),
    "sinai_lowsun_fp": (20, 230, 520, 430),
    "sunrise_ridge": (300, 180, 800, 480),
}
for v, c in crops.items():
    strip(v, [("BEFORE", B / f"{v}__remastered.png"), ("AFTER", A / f"{v}__remastered.png")],
          f"{v} crop {c}, x2 | before | after", f"crop_{v}.png", crop=c, scale=2)
