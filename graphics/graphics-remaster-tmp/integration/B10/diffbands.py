"""Per view and row band: max channel difference / pixels with any channel over
2 levels / over 8 levels, between <dirA>/<view>__<a>.png and
<dirB>/<view>__<b>.png.

    python diffbands.py <dirA> <a> <dirB> <b> [--md out.md] [--title T]
"""
import argparse
from pathlib import Path

import numpy as np
from PIL import Image

ap = argparse.ArgumentParser()
ap.add_argument("dir_a")
ap.add_argument("a")
ap.add_argument("dir_b")
ap.add_argument("b")
ap.add_argument("--md", default="")
ap.add_argument("--title", default="")
args = ap.parse_args()

BANDS = (("sky", 0, 422), ("horizon", 422, 591), ("ground", 591, 1080), ("whole", 0, 1080))


def img(p):
    return np.asarray(Image.open(p).convert("RGB")).astype(np.int16)


da, db = Path(args.dir_a), Path(args.dir_b)
rows = []
for pa in sorted(da.glob(f"*__{args.a}.png")):
    view = pa.name[: -len(f"__{args.a}.png")]
    pb = db / f"{view}__{args.b}.png"
    if not pb.exists():
        continue
    d = np.abs(img(pa) - img(pb)).max(axis=2)
    cells = [view]
    for _, r0, r1 in BANDS:
        x = d[r0:r1]
        cells.append(f"{int(x.max())} / {int((x > 2).sum()):,} / {int((x > 8).sum()):,}")
    rows.append(cells)

title = args.title or f"{da.name} __{args.a} against {db.name} __{args.b}"
lines = [f"# {title}", "", "max channel difference / pixels over 2 levels / pixels over 8 levels, per row band", "",
         "| view | sky | horizon | ground | whole |", "|---|---|---|---|---|"]
lines += ["| " + " | ".join(r) + " |" for r in rows]
text = "\n".join(lines)
print(text)
if args.md:
    Path(args.md).write_text(text + "\n", encoding="utf-8")
