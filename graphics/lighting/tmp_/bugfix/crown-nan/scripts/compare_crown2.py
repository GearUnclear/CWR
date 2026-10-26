"""Before/after sheets and numbers for the crown-nan fix, from the run2_* captures.

Shots per view (crown_nan_abel.test.sqf): <view>__remastered (first frame),
<view>__dbg3 (lighting normal), <view>__late (a plain Remastered frame 30
frames after the debug view). The evidence uses the late frame: at
forest_edge_golden the first frame draws the conifers with a lower LOD whose
leaves have valid normals (see NOTES.md).

NaN mask: magenta pixels of run2_nanpaint_before (the shipped shader with one
line that paints a fragment magenta when its lighting normal is NaN or inf),
same shot. Numbers are display luma (0-255, Rec. 709 weights).
"""
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

root = Path(sys.argv[1])
views = ["forest_edge_golden", "woods_mixed_noon", "woods_pine2_evening", "town_day"]
W = np.array([0.2126, 0.7152, 0.0722], np.float32)


def path(run, k, shot):
    off = {"remastered": 0, "dbg3": 1, "late": 2}[shot]
    v = views[k]
    return root / run / "tri" / "crown_nan_abel" / f"{3 * k + off:03d}_{v}__{shot}.png"


def load(p):
    return np.asarray(Image.open(p).convert("RGB")).astype(np.float32)


def label(img, text):
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, 8 + 6 * len(text), 16], fill=(0, 0, 0))
    d.text((4, 2), text, fill=(255, 255, 255))
    return img


def magenta(a):
    return (a[..., 0] > a[..., 1] + 80) & (a[..., 2] > a[..., 1] + 80)


rows = [
    "| view | shot | NaN-normal px in the shipped shader | their luma before | after | mean abs luma change on them | px changed > 4 levels | mean abs change elsewhere |",
    "|---|---|---|---|---|---|---|---|",
]
for k, v in enumerate(views):
    for shot in ("remastered", "late"):
        m = magenta(load(path("run2_nanpaint_before", k, shot)))
        b = load(path("run2_before", k, shot))
        a = load(path("run2_after", k, shot))
        lb, la = b @ W, a @ W
        d = np.abs(la - lb)
        n = int(m.sum())
        changed = int((np.abs(a - b).max(axis=2) > 4).sum())
        if n:
            rows.append(f"| {v} | {shot} | {n} ({100.0 * n / m.size:.1f}%) | {lb[m].mean():.1f} | {la[m].mean():.1f} | {d[m].mean():.2f} | {changed} | {d[~m].mean():.3f} |")
        else:
            rows.append(f"| {v} | {shot} | 0 | - | - | - | {changed} | {d.mean():.3f} |")

    # Sheets from the late frame.
    m = magenta(load(path("run2_nanpaint_before", k, "late")))
    bi = Image.open(path("run2_before", k, "late")).convert("RGB")
    ai = Image.open(path("run2_after", k, "late")).convert("RGB")
    w, h = bi.size
    sheet = Image.new("RGB", (w * 2, h * 2))
    sheet.paste(label(bi.copy(), "BEFORE (branch tip)"), (0, 0))
    sheet.paste(label(ai.copy(), "AFTER (fix)"), (w, 0))
    sheet.paste(label(Image.open(path("run2_before", k, "dbg3")).convert("RGB"), "BEFORE lighting normal (debug 3): NaN crowns black"), (0, h))
    sheet.paste(label(Image.open(path("run2_after", k, "dbg3")).convert("RGB"), "AFTER lighting normal (debug 3)"), (w, h))
    sheet.save(root / f"{v}__before_after.png")

    ys, xs = np.nonzero(m)
    if len(xs):
        pad = 16
        box = (
            int(max(0, np.percentile(xs, 1) - pad)),
            int(max(0, np.percentile(ys, 1) - pad)),
            int(min(w, np.percentile(xs, 99) + pad)),
            int(min(h, np.percentile(ys, 99) + pad)),
        )
        cw, ch = box[2] - box[0], box[3] - box[1]
        s = min(2.0, 900.0 / cw, 520.0 / ch)
        size = (max(1, int(cw * s)), max(1, int(ch * s)))
        strip = Image.new("RGB", (size[0] * 3, size[1]))
        parts = [
            ("BEFORE", path("run2_before", k, "late")),
            ("AFTER", path("run2_after", k, "late")),
            ("shipped shader: NaN normal (magenta)", path("run2_nanpaint_before", k, "late")),
        ]
        for i, (t, p) in enumerate(parts):
            im = Image.open(p).convert("RGB").crop(box).resize(size, Image.LANCZOS)
            strip.paste(label(im, t), (i * size[0], 0))
        strip.save(root / f"{v}__crop.png")

    # Signed change x8 over a dim copy of the BEFORE frame.
    b = load(path("run2_before", k, "late"))
    a = load(path("run2_after", k, "late"))
    dl = (a @ W) - (b @ W)
    out = np.repeat(((b @ W) * 0.25)[..., None], 3, axis=2)
    out[..., 0] += np.clip(dl * 8.0, 0, 255)
    out[..., 2] += np.clip(-dl * 8.0, 0, 255)
    img = label(Image.fromarray(np.clip(out, 0, 255).astype(np.uint8)), "AFTER - BEFORE luma x8: red brighter, blue darker")
    img.save(root / f"{v}__diff_x8.png")

(root / "measure.md").write_text("\n".join(rows) + "\n")
print("\n".join(rows))
