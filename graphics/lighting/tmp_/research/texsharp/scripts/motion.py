"""Motion check for the texsharp variants: the camera walks forward 0.25 m per
frame for 8 frames (abel_motion.test.sqf). Shimmer from boosted detail or
sharpening shows as frame-to-frame change in the distant bands, where the true
image barely moves.

    python motion.py <run_dir> [--crop x0,y0,x1,y1]

Writes <run_dir>/<view>__motion.gif (crop, 2x, variants side by side, 8
frames) and <run_dir>/motion.md (mean |frame i+1 - frame i| per band, 8-bit
units, and the ratio to the stock sequence).
"""
import argparse
import re
import shutil
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

VARS = ["mstock", "mrdtile", "mself", "mbest"]
NAMES = {"mstock": "Remastered (current)", "mrdtile": "detail kept K2=2 + anti-tile", "mself": "self-detail 1.5 (limited)",
         "mbest": "texsharp final (self-detail 1.5, detail kept, CAS 0.3 luma)"}
RE = re.compile(r"^(\d{3})_(.+)__(m[a-z]+)(\d)\.png$")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--crop", default="240,200,720,440")
    a = ap.parse_args()
    run = Path(a.run)
    frames = {}
    for p in (run / "tri").rglob("*.png"):
        m = RE.match(p.name)
        if m:
            key = (m.group(2), m.group(3), int(m.group(4)))
            if key not in frames or p.stat().st_mtime > frames[key].stat().st_mtime:
                frames[key] = p
    seqdir = run / "motion"
    seqdir.mkdir(exist_ok=True)
    for (v, var, i), p in frames.items():
        shutil.copy2(p, seqdir / f"{v}__{var}{i}.png")
    views = sorted({k[0] for k in frames})
    crop = tuple(int(x) for x in a.crop.split(","))
    fnt = ImageFont.truetype("C:/Windows/Fonts/arialbd.ttf", 16)
    md = ["| view | variant | far band | mid band | near band | far vs stock | mid vs stock |", "|---|---|---|---|---|---|---|"]
    for v in views:
        stats = {}
        for var in VARS:
            seq = [frames.get((v, var, i)) for i in range(8)]
            if not all(seq):
                continue
            ims = [np.asarray(Image.open(p).convert("L"), dtype=np.float32) for p in seq]
            h = ims[0].shape[0]
            bands = {"far": (0.36, 0.48), "mid": (0.48, 0.62), "near": (0.62, 1.0)}
            stats[var] = {b: float(np.mean([np.abs(ims[i + 1][int(y0 * h):int(y1 * h)] - ims[i][int(y0 * h):int(y1 * h)]).mean()
                                            for i in range(7)])) for b, (y0, y1) in bands.items()}
        base = stats.get("mstock")
        for var, s in stats.items():
            rf = s["far"] / base["far"] if base else float("nan")
            rm = s["mid"] / base["mid"] if base else float("nan")
            md.append(f"| {v} | {var} | {s['far']:.2f} | {s['mid']:.2f} | {s['near']:.2f} | {rf:.2f} | {rm:.2f} |")
        # GIF: variants side by side, crop at 2x
        gif = []
        for i in range(8):
            tiles = []
            for var in VARS:
                p = frames.get((v, var, i))
                if not p:
                    continue
                t = Image.open(p).convert("RGB").crop(crop)
                t = t.resize((t.width * 2, t.height * 2), Image.NEAREST)
                d = ImageDraw.Draw(t)
                box = d.textbbox((6, 6), NAMES[var], font=fnt)
                d.rectangle([box[0] - 4, box[1] - 3, box[2] + 4, box[3] + 3], fill=(0, 0, 0))
                d.text((6, 6), NAMES[var], font=fnt, fill=(255, 255, 255))
                tiles.append(t)
            if not tiles:
                continue
            w, hh = tiles[0].size
            cols = 2
            rows = (len(tiles) + 1) // 2
            sheet = Image.new("RGB", (cols * w + 6, rows * hh + 6 * (rows - 1)), (20, 20, 20))
            for k, t in enumerate(tiles):
                r, c = divmod(k, cols)
                sheet.paste(t, (c * (w + 6), r * (hh + 6)))
            gif.append(sheet)
        if gif:
            gif[0].save(run / f"{v}__motion.gif", save_all=True, append_images=gif[1:], duration=250, loop=0)
    (run / "motion.md").write_text("\n".join(md) + "\n", encoding="utf-8")
    print("\n".join(md))


if __name__ == "__main__":
    main()
