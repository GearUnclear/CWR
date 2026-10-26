"""Contact sheet of capture shots, labelled, for the look check.

    python sheet.py <out.png> <cols> <width> <png> [<png> ...]

Each image is scaled to <width> px wide and labelled with its file stem.
"""
import sys
from pathlib import Path

from PIL import Image, ImageDraw


def main():
    out, cols, width = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
    files = [Path(p) for p in sys.argv[4:]]
    ims = []
    for f in files:
        im = Image.open(f).convert("RGB")
        h = round(im.height * width / im.width)
        ims.append((f.stem, im.resize((width, h), Image.LANCZOS)))
    h = max(im.height for _, im in ims)
    rows = (len(ims) + cols - 1) // cols
    sheet = Image.new("RGB", (cols * width, rows * (h + 18)), (20, 20, 20))
    d = ImageDraw.Draw(sheet)
    for i, (name, im) in enumerate(ims):
        x, y = (i % cols) * width, (i // cols) * (h + 18)
        d.text((x + 4, y + 3), name, fill=(230, 230, 230))
        sheet.paste(im, (x, y + 18))
    sheet.save(out)
    print(out, sheet.size)


if __name__ == "__main__":
    main()
