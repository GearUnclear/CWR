"""Side-by-side crops of one view across suffixes (and runs).

  python crops.py <out.png> <view> x0 y0 x1 y1 scale <run>:<suffix> [<run>:<suffix> ...]

<run> is a capture run folder (its tri tree is searched); each crop is
scaled by `scale` (nearest from 2x up), panels 6 px apart.
"""
import sys
from pathlib import Path

from PIL import Image


def find(run, view, suf):
    for p in Path(run).rglob(f"*_{view}__{suf}.png"):
        return p
    raise SystemExit(f"missing {view}__{suf} in {run}")


def main():
    out, view = sys.argv[1], sys.argv[2]
    x0, y0, x1, y1 = (int(v) for v in sys.argv[3:7])
    sc = float(sys.argv[7])
    ims = []
    for spec in sys.argv[8:]:
        run, suf = spec.rsplit(":", 1)
        im = Image.open(find(run, view, suf)).convert("RGB").crop((x0, y0, x1, y1))
        im = im.resize((int(im.width * sc), int(im.height * sc)), Image.NEAREST if sc >= 2 else Image.LANCZOS)
        ims.append(im)
    W = sum(i.width for i in ims) + 6 * (len(ims) - 1)
    o = Image.new("RGB", (W, ims[0].height), (255, 0, 255))
    x = 0
    for i in ims:
        o.paste(i, (x, 0))
        x += i.width + 6
    o.save(out)
    print(out, o.size)


if __name__ == "__main__":
    main()
