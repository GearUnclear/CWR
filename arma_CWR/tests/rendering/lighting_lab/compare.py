"""Side-by-side BEFORE | AFTER comparison of two lighting-lab output directories.

  python compare.py BEFORE_DIR AFTER_DIR --out DIR [--labels BEFORE AFTER] [--diff] [--diff-gain 4]

For every PNG name present in both directories (contact_sheet.png excluded)
writes DIR/<name>.png = BEFORE | AFTER (| optional amplified |difference|),
plus DIR/compare_sheet.png with all pairs and DIR/compare.json with simple
per-image metrics (mean luma before/after, mean abs difference, PSNR).
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

LAB_DIR = Path(__file__).resolve().parent
if str(LAB_DIR) not in sys.path:
    sys.path.insert(0, str(LAB_DIR))

from run_lab import _font, label_image  # noqa: E402

SKIP = {"contact_sheet.png", "compare_sheet.png"}


def metrics(a: np.ndarray, b: np.ndarray) -> dict:
    fa = a.astype(np.float64) / 255.0
    fb = b.astype(np.float64) / 255.0
    w = np.array([0.299, 0.587, 0.114])
    mse = float(np.mean((fa - fb) ** 2))
    return {
        "luma_before": round(float((fa @ w).mean()), 4),
        "luma_after": round(float((fb @ w).mean()), 4),
        "mean_abs_diff": round(float(np.mean(np.abs(fa - fb))), 5),
        "max_abs_diff": round(float(np.max(np.abs(fa - fb))), 4),
        "psnr_db": None if mse == 0 else round(10 * np.log10(1.0 / mse), 2),
        "changed_pixel_frac": round(float(np.mean(np.any(a != b, axis=2))), 5),
    }


def pair_image(before: Image.Image, after: Image.Image, labels: tuple[str, str], name: str, diff: bool,
               gain: float) -> Image.Image:
    if after.size != before.size:
        after = after.resize(before.size, Image.LANCZOS)
    w, h = before.size
    panels = [label_image(before, f"{labels[0]}  {name}"), label_image(after, f"{labels[1]}  {name}")]
    if diff:
        a = np.asarray(before.convert("RGB"), dtype=np.float64)
        b = np.asarray(after.convert("RGB"), dtype=np.float64)
        d = np.clip(np.abs(a - b) * gain, 0, 255).astype(np.uint8)
        panels.append(label_image(Image.fromarray(d, "RGB"), f"|diff| x{gain:g}"))
    gap = 4
    out = Image.new("RGB", (len(panels) * w + (len(panels) - 1) * gap, h), (40, 40, 40))
    for i, p in enumerate(panels):
        out.paste(p, (i * (w + gap), 0))
    return out


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("before")
    ap.add_argument("after")
    ap.add_argument("--out", required=True)
    ap.add_argument("--labels", nargs=2, default=["BEFORE", "AFTER"])
    ap.add_argument("--diff", action="store_true", help="add an amplified |difference| panel")
    ap.add_argument("--diff-gain", type=float, default=4.0)
    ap.add_argument("--sheet-width", type=int, default=1600, help="width of one pair row in compare_sheet.png")
    args = ap.parse_args(argv)

    bdir, adir, odir = Path(args.before), Path(args.after), Path(args.out)
    odir.mkdir(parents=True, exist_ok=True)
    names = sorted(p.name for p in bdir.glob("*.png") if p.name not in SKIP and (adir / p.name).exists())
    only_b = sorted(p.name for p in bdir.glob("*.png") if p.name not in SKIP and not (adir / p.name).exists())
    only_a = sorted(p.name for p in adir.glob("*.png") if p.name not in SKIP and not (bdir / p.name).exists())
    if not names:
        print("no common PNG names", file=sys.stderr)
        return 1

    report = {"before": str(bdir.resolve()), "after": str(adir.resolve()), "labels": args.labels, "images": {},
              "only_before": only_b, "only_after": only_a}
    pairs = []
    for n in names:
        b = Image.open(bdir / n).convert("RGB")
        a = Image.open(adir / n).convert("RGB")
        stem = Path(n).stem
        pair = pair_image(b, a, tuple(args.labels), stem, args.diff, args.diff_gain)
        pair.save(odir / n)
        m = metrics(np.asarray(b), np.asarray(a.resize(b.size) if a.size != b.size else a))
        report["images"][stem] = m
        pairs.append((stem, pair))
        print(f"  {stem:22s} luma {m['luma_before']:.3f} -> {m['luma_after']:.3f}  "
              f"mean|d| {m['mean_abs_diff']:.4f}  changed {m['changed_pixel_frac']:.3f}")

    row_w = args.sheet_width
    rows = []
    for stem, p in pairs:
        h = round(row_w * p.size[1] / p.size[0])
        rows.append(p.resize((row_w, h), Image.LANCZOS))
    head = 34
    sheet = Image.new("RGB", (row_w, head + sum(r.size[1] for r in rows) + 2 * (len(rows) - 1)), (24, 24, 24))
    d = ImageDraw.Draw(sheet)
    d.text((8, 6), f"{args.labels[0]}: {bdir}    |    {args.labels[1]}: {adir}", fill=(230, 230, 230), font=_font(16))
    y = head
    for r in rows:
        sheet.paste(r, (0, y))
        y += r.size[1] + 2
    sheet.save(odir / "compare_sheet.png")
    (odir / "compare.json").write_text(json.dumps(report, indent=1))
    print(f"wrote {len(pairs)} pairs + compare_sheet.png to {odir}")
    if only_b or only_a:
        print(f"  unmatched: before-only {only_b}, after-only {only_a}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
