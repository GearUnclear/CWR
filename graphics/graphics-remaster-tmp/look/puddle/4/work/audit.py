"""Audit sheet for the wet-ground terms (puddle agent 4).

python audit.py <shots dir (run-captures OutDir)> <out png> [<out txt>]
Reads <view>__<label>.png from the tri tree. Per term: the reference minus the
frame with that term off, as a heat map (display luminance x 4), and the term's
share of the linear luminance on non-puddle and on puddle pixels of the two
regions of interest (tree base, open street). The puddle mask is probe p8.
"""
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

ROIS = {
    "town_night_rain": [("tree base", 780, 610), ("open street", 330, 790)],
    "seam_b": [("tree base", 110, 560), ("open street", 640, 760)],
    "seam_d": [("tree base", 700, 640), ("open street", 300, 800)],
    "seam_day": [("tree base", 770, 640), ("open ground", 180, 820)],
}
TERMS = [
    ("p1", "film lamp streaks (B7)"),
    ("p2", "film sun/moon glint (B7)"),
    ("p3", "SSR on non-puddle (B6)"),
    ("p4", "surface specular (lane)"),
    ("p7", "forward sky env (B7)"),
    ("p5", "puddle terms (keep)"),
    ("bloom0", "camera bloom"),
    ("foglights0", "lights in the fog"),
]


def font(size):
    for f in ("arial.ttf", "segoeui.ttf"):
        try:
            return ImageFont.truetype(f, size)
        except OSError:
            pass
    return ImageFont.load_default()


def find(shots, view, label):
    hits = list(Path(shots).rglob(f"*_{view}__{label}.png"))
    return hits[0] if hits else None


def load(p):
    return np.asarray(Image.open(p).convert("RGB")).astype(np.float32)


def lum_disp(a):
    return a @ np.array([0.2126, 0.7152, 0.0722], np.float32)


def lum_lin(a):
    return ((a / 255.0) ** 2.2) @ np.array([0.2126, 0.7152, 0.0722], np.float32)


def heat(d):
    # d: display luminance difference (ref - off), 0..255; x4, black -> red -> yellow -> white
    x = np.clip(d * 4.0 / 255.0, 0, 1)
    r = np.clip(x * 3, 0, 1)
    g = np.clip(x * 3 - 1, 0, 1)
    b = np.clip(x * 3 - 2, 0, 1)
    neg = np.clip(-d * 4.0 / 255.0, 0, 1)  # the term darkened: blue
    return (np.stack([r, g, np.maximum(b, neg)], -1) * 255).astype(np.uint8)


MASKDIR = None


def masks(shots, view, shape):
    p = find(shots, view, "p8") or (find(MASKDIR, view, "v8") if MASKDIR else None)
    if p is None:
        return np.zeros(shape[:2], bool), np.zeros(shape[:2], bool)
    m = load(p)
    r, g, b = m[..., 0], m[..., 1], m[..., 2]
    pud = (r > g + 25) & (r > b + 25)
    damp = (g > r + 25) & (g > b + 25)
    return pud, damp


def stats(shots, view, out_lines):
    ref = load(find(shots, view, "p0"))
    pud, damp = masks(shots, view, ref.shape)
    res = {}
    for key, name in TERMS:
        p = find(shots, view, key)
        if p is None:
            continue
        off = load(p)
        dl = lum_lin(ref) - lum_lin(off)
        row = []
        for rname, x0, y0 in ROIS[view]:
            sl = (slice(y0, y0 + 180), slice(x0, x0 + 320))
            rl = lum_lin(ref)[sl]
            np_m = ~pud[sl] & ~damp[sl]
            pu_m = pud[sl]
            s_np = dl[sl][np_m].mean() / max(rl[np_m].mean(), 1e-6) if np_m.any() else float("nan")
            s_pu = dl[sl][pu_m].mean() / max(rl[pu_m].mean(), 1e-6) if pu_m.any() else float("nan")
            row.append((rname, s_np, s_pu, np_m.mean(), pu_m.mean()))
        res[key] = row
        out_lines.append(f"{view:16s} {name:28s} " + "  ".join(
            f"{r[0]}: non-puddle {100 * r[1]:6.1f}% puddle {100 * r[2]:6.1f}% (np {100 * r[3]:.0f}% px, pud {100 * r[4]:.0f}% px)"
            for r in row))
    return res, pud


def main():
    global MASKDIR
    shots = Path(sys.argv[1])
    out = Path(sys.argv[2])
    if len(sys.argv) > 4:
        MASKDIR = Path(sys.argv[4])
    views = [v for v in ("town_night_rain", "seam_b", "seam_day") if find(shots, v, "p0")]
    W, H = 480, 270
    cols = 4
    f, fs = font(18), font(14)
    lines = []
    blocks = []
    for v in views:
        res, pud = stats(shots, v, lines)
        ref = load(find(shots, v, "p0"))
        tiles = []
        for key, title in (("p0", "reference (all terms on)"), ("refl", "SSR reflection alone (debug 10)"),
                           ("p8", "puddle mask (red water, green damp)"), ("p6", "all non-puddle reflective terms off")):
            p = find(shots, v, key) or (find(MASKDIR, v, "v8") if (MASKDIR and key == "p8") else None)
            if p is not None:
                tiles.append((Image.open(p).convert("RGB"), title, None))
        for key, name in TERMS:
            p = find(shots, v, key)
            if p is None:
                continue
            d = lum_disp(ref) - lum_disp(load(p))
            r = res.get(key, [])
            sub = "  ".join(f"{x[0]}: {100 * x[1]:.0f}% / pud {100 * x[2]:.0f}%" for x in r)
            tiles.append((Image.fromarray(heat(d)), f"{name}: ref - off, x4", sub))
        rows = (len(tiles) + cols - 1) // cols
        blk = Image.new("RGB", (W * cols, 30 + rows * (H + 22)), (16, 16, 16))
        dr = ImageDraw.Draw(blk)
        dr.text((8, 5), f"{v}: what each term adds (heat = reference minus that term off; % = its share of the light, non-puddle / puddle pixels)",
                fill=(240, 240, 240), font=f)
        for i, (im, title, sub) in enumerate(tiles):
            x = (i % cols) * W
            y = 30 + (i // cols) * (H + 22)
            blk.paste(im.resize((W, H), Image.LANCZOS), (x, y))
            dr.rectangle((x, y, x + W - 1, y + 20), fill=(0, 0, 0))
            dr.text((x + 4, y + 2), title, fill=(255, 210, 0), font=fs)
            if sub:
                dr.text((x + 4, y + H + 3), sub, fill=(200, 230, 255), font=fs)
            s = W / 1920.0
            for j, (_, x0, y0) in enumerate(ROIS[v]):
                dr.rectangle((x + x0 * s, y + y0 * s, x + (x0 + 320) * s, y + (y0 + 180) * s),
                             outline=(255, 210, 0) if j == 0 else (0, 200, 255))
        blocks.append(blk)
    sheet = Image.new("RGB", (W * cols, sum(b.height for b in blocks)), (16, 16, 16))
    y = 0
    for b in blocks:
        sheet.paste(b, (0, y))
        y += b.height
    sheet.save(out)
    print(out)
    txt = "\n".join(lines)
    print(txt)
    if len(sys.argv) > 3:
        Path(sys.argv[3]).write_text(txt + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
