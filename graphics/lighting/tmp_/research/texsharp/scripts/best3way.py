"""BEST_<view>.png: Classic | Remastered (current) | Remastered + texsharp, full
frames on top with the GPU frame ms of each shot, and the same crop of all
three enlarged 2x (nearest) below, so texture detail is judged at pixel level.

    python best3way.py <run_dir> <view> <label> <x0,y0,x1,y1> <out.png> [--title "..."]

<run_dir>/shots/<view>__<label>.png and metrics.json (compare.py) are read.
"""
import json
import sys
from pathlib import Path

from PIL import Image, ImageDraw

sys.path.insert(0, str(Path(__file__).parent))
from compare import NAMES, font, label_box  # noqa: E402


def main():
    args = sys.argv[1:]
    title = None
    if "--title" in args:
        i = args.index("--title"); title = args[i + 1]; del args[i:i + 2]
    run, view, label, box, out = Path(args[0]), args[1], args[2], tuple(int(v) for v in args[3].split(",")), args[4]
    labs = ["classic", "remastered", label]
    ms = {}
    mj = run / "metrics.json"
    if mj.exists():
        for r in json.loads(mj.read_text(encoding="utf-8")):
            if r["view"] == view:
                ms[r["label"]] = r.get("gpu_frame")
    ims = [Image.open(run / "shots" / f"{view}__{l}.png").convert("RGB") for l in labs]
    w, h = ims[0].size
    crops = [im.crop(box).resize(((box[2] - box[0]) * 2, (box[3] - box[1]) * 2), Image.NEAREST) for im in ims]
    cw, ch = crops[0].size
    gap = 6
    head = 36
    sheet_w = max(3 * w + 2 * gap, 3 * cw + 2 * gap)
    sheet = Image.new("RGB", (sheet_w, head + h + gap + 26 + ch), (20, 20, 20))
    d = ImageDraw.Draw(sheet)
    d.text((8, 6), title or f"{view}: Classic | Remastered (current) | {NAMES.get(label, label)}", font=font(20),
           fill=(230, 230, 230))
    fb, fs = font(20), font(15)
    for i, (l, im) in enumerate(zip(labs, ims)):
        p = im.copy()
        dd = ImageDraw.Draw(p)
        label_box(dd, (10, 8), NAMES.get(l, l), fb)
        if ms.get(l) is not None:
            label_box(dd, (10, h - 28), f"GPU frame {ms[l]:.2f} ms (shared GPU, noisy)", fs)
        dd.rectangle(box, outline=(255, 210, 0), width=2)
        sheet.paste(p, (i * (w + gap), head))
    y = head + h + gap
    d.text((8, y + 3), f"crop {box} (yellow box) at 2x nearest", font=font(16), fill=(230, 230, 230))
    for i, c in enumerate(crops):
        sheet.paste(c, (i * (cw + gap), y + 26))
    sheet.save(out)
    print(out)


if __name__ == "__main__":
    main()
