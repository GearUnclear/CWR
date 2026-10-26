"""Crop the same box out of several shots of a view and lay them side by side,
labelled, at 1.5x. usage: python crop_cmp.py <run_dir> <view> x0,y0,x1,y1 label1,label2,... [out_name] [scale]"""
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

PRETTY = {
    "classic": "Classic",
    "remastered": "Remastered (current)",
    "rm_vbo": "Remastered + late VBO",
    "gbufonly": "+ G-buffer writes only",
    "gi": "+ G-buffer GI (r 6 m)",
    "gi_src1": "+ GI, whole-frame source",
    "gi_r3": "+ GI (r 3 m)",
    "giao": "+ ray AO only",
}


def main():
    run = Path(sys.argv[1])
    view = sys.argv[2]
    box = tuple(int(v) for v in sys.argv[3].split(","))
    labels = sys.argv[4].split(",")
    out_name = sys.argv[5] if len(sys.argv) > 5 else f"crop_{view}.png"
    scale = float(sys.argv[6]) if len(sys.argv) > 6 else 1.5
    w = int((box[2] - box[0]) * scale)
    h = int((box[3] - box[1]) * scale)
    gap, title = 6, 30
    sheet = Image.new("RGB", (len(labels) * w + (len(labels) - 1) * gap, h + title), (20, 20, 20))
    d = ImageDraw.Draw(sheet)
    fnt = ImageFont.truetype("C:/Windows/Fonts/arialbd.ttf", 17)
    for i, l in enumerate(labels):
        p = run / f"{view}__{l}.png"
        x0 = i * (w + gap)
        if p.exists():
            im = Image.open(p).convert("RGB").crop(box).resize((w, h), Image.LANCZOS)
            sheet.paste(im, (x0, title))
        d.text((x0 + 6, 6), PRETTY.get(l, l), font=fnt, fill=(235, 235, 235))
    sheet.save(run / out_name)
    print(run / out_name)


if __name__ == "__main__":
    main()
