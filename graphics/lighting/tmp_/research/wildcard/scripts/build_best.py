"""Three-way (or more) labelled comparisons for the wildcard track.

    python build_best.py <out.png> <view> panel [panel ...] [--crop x0,y0,x1,y1]

Each panel is <run_dir>:<label>[:<title>], e.g.
    r3s:classic:Classic  r3s:remastered:"Remastered (shipped)"  r4:svm_full:"+ sky visibility maps"
Images are <run_dir>/<view>__<label>.png (collected by collect.py); the GPU frame
ms comes from that run's UDCAP shot line. With --crop a second row shows the
same crop of every panel at 2x.
"""
import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from collect import collect, font, panel  # noqa: E402
from PIL import Image, ImageDraw  # noqa: E402

ROOT = Path(r"D:\Arma_CWA_worktrees\lighting\tmp\research\wildcard")
_rows = {}


def rows_of(run):
    if run not in _rows:
        _, r = collect(ROOT / run)
        _rows[run] = r
    return _rows[run]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("view")
    ap.add_argument("panels", nargs="+")
    ap.add_argument("--crop", default="")
    ap.add_argument("--title", default="")
    a = ap.parse_args()
    pans = []
    crops = []
    crop = tuple(int(v) for v in a.crop.split(",")) if a.crop else None
    for spec in a.panels:
        parts = spec.split(":", 2)
        run, label = parts[0], parts[1]
        title = parts[2] if len(parts) > 2 else label
        img = ROOT / run / f"{a.view}__{label}.png"
        if not img.exists():
            print("missing", img)
            continue
        ms = rows_of(run).get((a.view, label), {}).get("frame")
        pans.append(panel(img, title, ms))
        if crop:
            c = Image.open(img).convert("RGB").crop(crop)
            w = crop[2] - crop[0]
            h = crop[3] - crop[1]
            sc = min(960 / w, 640 / h)
            crops.append(c.resize((int(w * sc), int(h * sc)), Image.LANCZOS))
    W, H, gap = 960, 640, 6
    n = len(pans)
    rows = 2 if crops else 1
    ch = crops[0].height if crops else 0
    sheet = Image.new("RGB", (n * W + (n - 1) * gap, 40 + H + (gap + ch if crops else 0)), (24, 24, 24))
    d = ImageDraw.Draw(sheet)
    d.text((10, 8), a.title or a.view, fill=(255, 255, 255), font=font(24))
    for i, p in enumerate(pans):
        sheet.paste(p, (i * (W + gap), 40))
        if crops:
            sheet.paste(crops[i], (i * (W + gap), 40 + H + gap))
    sheet.save(a.out)
    print("wrote", a.out, sheet.size)


if __name__ == "__main__":
    main()
