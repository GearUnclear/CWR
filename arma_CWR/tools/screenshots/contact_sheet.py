#!/usr/bin/env python3
"""Tile a shoot's PNGs into one contact sheet.

Reviewing a 40-shot run by opening 40 files is slow and you lose the comparison;
one sheet shows immediately which frames are blank, which are buried inside
geometry, and which are worth keeping.

    python contact_sheet.py tmp/showcase/sinai tmp/showcase/sheet.png [columns]

Requires Pillow.
"""

import glob
import os
import sys

from PIL import Image, ImageDraw

THUMB_W, THUMB_H = 480, 270  # 16:9, matching the 2560x1440 captures
LABEL_H = 16


def main() -> int:
    if len(sys.argv) < 3:
        print(__doc__)
        return 2

    src, out = sys.argv[1], sys.argv[2]
    cols = int(sys.argv[3]) if len(sys.argv) > 3 else 4

    files = sorted(glob.glob(os.path.join(src, "*.png")))
    if not files:
        print(f"no PNGs in {src}")
        return 1

    rows = (len(files) + cols - 1) // cols
    sheet = Image.new("RGB", (cols * THUMB_W, rows * (THUMB_H + LABEL_H)), (20, 20, 20))
    draw = ImageDraw.Draw(sheet)

    for i, path in enumerate(files):
        thumb = Image.open(path).convert("RGB").resize((THUMB_W, THUMB_H))
        x = (i % cols) * THUMB_W
        y = (i // cols) * (THUMB_H + LABEL_H)
        sheet.paste(thumb, (x, y))
        draw.text((x + 4, y + THUMB_H + 3), os.path.basename(path)[:-4], fill=(230, 230, 230))

    sheet.save(out)
    print(f"{out}  {sheet.size[0]}x{sheet.size[1]}  {len(files)} tiles")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
