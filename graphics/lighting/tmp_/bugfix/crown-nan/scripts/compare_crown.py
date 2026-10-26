"""Before/after images and numbers for the crown-nan fix.

Mask = the zero-mesh-normal foliage pixels (pure red) of the BEFORE diagnostic
run. Numbers are display luma (0-255, Rec. 709 weights) of the presented frame.
"""
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

root = Path(sys.argv[1])
out = root
views = ["forest_edge_golden", "woods_mixed_noon", "woods_pine2_evening", "town_day"]


def shots(run):
    d = root / run / "tri" / "crown_nan_abel"
    res = {}
    for p in sorted(d.glob("*.png")):
        label = p.stem.split("_", 1)[1]
        res[label] = p
    return res


before, after = shots("run_before"), shots("run_after")
diag, check = shots("run_diag_before"), shots("run_check_after")
nanp = shots("run_nanprobe_before")


def load(p):
    return np.asarray(Image.open(p).convert("RGB")).astype(np.float32)


def luma(a):
    return a[..., 0] * 0.2126 + a[..., 1] * 0.7152 + a[..., 2] * 0.0722


def label(img, text):
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, 8 + 7 * len(text), 18], fill=(0, 0, 0))
    d.text((4, 3), text, fill=(255, 255, 255))
    return img


lines = ["| view | zero-mesh-normal px (diag, pure red) | NaN-normal px in the shipped shader (probe, pure blue) | NaN px luma before | after | mean abs change on NaN px | elsewhere | AFTER check: NaN px | repaired px |",
         "|---|---|---|---|---|---|---|---|---|"]
for v in views:
    b = load(before[f"{v}__remastered"])
    a = load(after[f"{v}__remastered"])
    dg = load(diag[f"{v}__dbg3"])
    ck = load(check[f"{v}__dbg3"])
    zmask = (dg[..., 0] > 200) & (dg[..., 1] < 40) & (dg[..., 2] < 40)
    npb = load(nanp[f"{v}__dbg3"])
    mask = (npb[..., 2] > 200) & (npb[..., 0] < 40) & (npb[..., 1] < 40)
    # Debug colours pass through the fog mix, so classify by channel dominance.
    nan_after = (ck[..., 1] > ck[..., 0] + 60) & (ck[..., 1] > ck[..., 2] + 60)
    repaired = (ck[..., 0] > ck[..., 1] + 25) & (ck[..., 0] > ck[..., 2] + 25)
    nan_before_any = (dg[..., 0] > dg[..., 1] + 60) & (dg[..., 0] > dg[..., 2] + 60)
    lb, la = luma(b), luma(a)
    d = np.abs(la - lb)
    n = int(mask.sum())
    lines.append(
        f"| {v} | {int(zmask.sum())} | {n} ({100.0 * n / mask.size:.1f}%) | {lb[mask].mean():.1f} | {la[mask].mean():.1f} | "
        f"{d[mask].mean():.1f} | {d[~mask].mean():.2f} | {int(nan_after.sum())} | {int(repaired.sum())} |"
    )
    # Side by side: BEFORE | AFTER (remastered), then the normals debug view.
    h, w = b.shape[:2]
    sheet = Image.new("RGB", (w * 2, h * 2), (0, 0, 0))
    sheet.paste(label(Image.open(before[f"{v}__remastered"]).convert("RGB"), "BEFORE (branch tip)"), (0, 0))
    sheet.paste(label(Image.open(after[f"{v}__remastered"]).convert("RGB"), "AFTER (fix)"), (w, 0))
    sheet.paste(label(Image.open(before[f"{v}__dbg3"]).convert("RGB"), "BEFORE: lighting normal (debug 3)"), (0, h))
    sheet.paste(label(Image.open(after[f"{v}__dbg3"]).convert("RGB"), "AFTER: lighting normal (debug 3)"), (w, h))
    sheet.save(out / f"{v}__before_after.png")
    # Crop around the zero-normal crowns, 2x.
    if n:
        ys, xs = np.nonzero(mask)
        x0, x1 = np.percentile(xs, 2), np.percentile(xs, 98)
        y0, y1 = np.percentile(ys, 2), np.percentile(ys, 98)
        pad = 20
        box = (int(max(0, x0 - pad)), int(max(0, y0 - pad)), int(min(w, x1 + pad)), int(min(h, y1 + pad)))
        cw, chh = box[2] - box[0], box[3] - box[1]
        s = min(2.0, 960.0 / cw)
        size = (int(cw * s), int(chh * s))
        crops = [
            ("BEFORE", before[f"{v}__remastered"]),
            ("AFTER", after[f"{v}__remastered"]),
            ("shipped shader: NaN normal (blue)", nanp[f"{v}__dbg3"]),
        ]
        strip = Image.new("RGB", (size[0] * 3, size[1]), (0, 0, 0))
        for i, (t, p) in enumerate(crops):
            im = Image.open(p).convert("RGB").crop(box).resize(size, Image.LANCZOS)
            strip.paste(label(im, t), (i * size[0], 0))
        strip.save(out / f"{v}__crop.png")
    # The two diagnostics side by side.
    dsheet = Image.new("RGB", (w * 2, h), (0, 0, 0))
    dsheet.paste(label(Image.open(diag[f"{v}__dbg3"]).convert("RGB"), "BEFORE diag: red = zero mesh normal (NaN)"), (0, 0))
    dsheet.paste(label(Image.open(check[f"{v}__dbg3"]).convert("RGB"), "AFTER check: red = repaired (sun term), green = NaN"), (w, 0))
    dsheet.save(out / f"{v}__diag.png")

(out / "measure.md").write_text("\n".join(lines) + "\n")
print("\n".join(lines))
