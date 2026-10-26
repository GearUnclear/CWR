"""Before/after figures for bug fix msaa-default.

Usage: python make_figures.py <run_before> <run_after> <out_dir>

Each run dir holds <view>__<mode>.png copied out by make_pairs.py. Writes, per
view and mode: a side-by-side full frame (before | after) and a crop of each
shown at 3x with nearest-neighbour scaling so stair steps stay visible, plus a
small text table of whole-frame and crop mean luma (to show MSAA moves edges,
not light).
"""

import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

# (view, crop box x0, y0, x1, y1) on the 960x640 frame
CROPS = {
    "forest_edge_golden": [(560, 20, 960, 320), (180, 140, 500, 460)],
    "sinai_lowsun_fp": [(0, 140, 420, 380), (300, 140, 620, 350), (590, 200, 850, 340)],
}
MODES = ["remastered", "classic"]


def label(img, text):
    d = ImageDraw.Draw(img)
    w = d.textlength(text) + 10
    d.rectangle([0, 0, w, 16], fill=(0, 0, 0))
    d.text((5, 2), text, fill=(255, 255, 255))
    return img


def side_by_side(a, b, gap=8):
    out = Image.new("RGB", (a.width + b.width + gap, max(a.height, b.height)), (30, 30, 30))
    out.paste(a, (0, 0))
    out.paste(b, (a.width + gap, 0))
    return out


def luma(img):
    a = np.asarray(img.convert("RGB"), dtype=np.float64)
    return float((0.2126 * a[..., 0] + 0.7152 * a[..., 1] + 0.0722 * a[..., 2]).mean())


def main():
    before, after, out = (Path(p) for p in sys.argv[1:4])
    out.mkdir(parents=True, exist_ok=True)
    rows = ["| image | mean luma before | mean luma after | after - before |", "|---|---|---|---|"]
    for view, crops in CROPS.items():
        for mode in MODES:
            name = f"{view}__{mode}.png"
            pb, pa = before / name, after / name
            if not (pb.exists() and pa.exists()):
                print("missing", name)
                continue
            ib, ia = Image.open(pb).convert("RGB"), Image.open(pa).convert("RGB")
            lb, la = luma(ib), luma(ia)
            rows.append(f"| {view} {mode} full | {lb:.2f} | {la:.2f} | {la - lb:+.2f} |")
            full = side_by_side(label(ib.copy(), f"BEFORE (branch tip, shipped default) {mode}"),
                                label(ia.copy(), f"AFTER (fix, new default 4x MSAA) {mode}"))
            full.save(out / f"{view}__{mode}__before_after.png")
            for k, box in enumerate(crops):
                cb, ca = ib.crop(box), ia.crop(box)
                s = 3
                cb = cb.resize((cb.width * s, cb.height * s), Image.NEAREST)
                ca = ca.resize((ca.width * s, ca.height * s), Image.NEAREST)
                rows.append(f"| {view} {mode} crop{k} {box} | {luma(ib.crop(box)):.2f} | {luma(ia.crop(box)):.2f} | "
                            f"{luma(ia.crop(box)) - luma(ib.crop(box)):+.2f} |")
                side_by_side(label(cb, f"BEFORE {mode} crop{k} x3"), label(ca, f"AFTER {mode} crop{k} x3")).save(
                    out / f"{view}__{mode}__crop{k}_x3.png")
    (out / "luma.md").write_text("\n".join(rows) + "\n", encoding="utf-8")
    print("\n".join(rows))


if __name__ == "__main__":
    main()
