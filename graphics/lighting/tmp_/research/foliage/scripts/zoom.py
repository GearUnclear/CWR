"""Zoomed crops of one region across labels of a view.

    python zoom.py <run_dir> <view> x0,y0,x1,y1 <label,label,...> <out.png> [scale]
"""
import sys
from pathlib import Path

from PIL import Image, ImageDraw

sys.path.insert(0, str(Path(__file__).parent))
from collect import font, label_box  # noqa: E402


def main():
    run, view, box, labels, out = Path(sys.argv[1]), sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5]
    scale = float(sys.argv[6]) if len(sys.argv) > 6 else 2.0
    box = tuple(int(v) for v in box.split(","))
    labs = labels.split(",")
    ims = []
    for lab in labs:
        p = run / f"{view}__{lab}.png"
        if not p.exists():
            continue
        im = Image.open(p).convert("RGB").crop(box)
        im = im.resize((int(im.width * scale), int(im.height * scale)), Image.LANCZOS)
        label_box(ImageDraw.Draw(im), (8, 6), {"remastered": "Remastered (current)", "classic": "Classic"}.get(lab, lab), font(18))
        ims.append(im)
    w, h = ims[0].size
    cols = min(len(ims), 3 if w < 700 else 2)
    rows = (len(ims) + cols - 1) // cols
    sheet = Image.new("RGB", (cols * w + (cols - 1) * 6, rows * h + (rows - 1) * 6), (20, 20, 20))
    for i, im in enumerate(ims):
        r, c = divmod(i, cols)
        sheet.paste(im, (c * (w + 6), r * (h + 6)))
    sheet.save(out)
    print(out)


if __name__ == "__main__":
    main()
