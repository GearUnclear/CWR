"""Collect a texsharp capture run and build comparison images + a metrics table.

    python compare.py <run_dir> [--best <label>] [--crop x0,y0,x1,y1] [--views a,b]

<run_dir>/tri holds Trident's output tree (NNN_<view>__<label>.png and the
game_stdout.log files with UDCAP lines). Writes into <run_dir>:
    shots/<view>__<label>.png       every capture, copied out of the tri tree
    <view>__strip.png               every variant of a view in a grid, labelled
    <view>__3way_<best>.png         Classic | Remastered | Remastered + <best>
    <view>__3way_<best>_crop.png    the same, cropped and enlarged 2x
    metrics.md / metrics.json       GPU ms per shot + a sharpness measure
The sharpness measure is the mean absolute Laplacian of luminance (8-bit
units) in three horizontal bands; it rises with fine detail and with noise, so
it is a hint, not a verdict: look at the images.
"""
import argparse
import json
import re
import shutil
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

SHOT_RE = re.compile(r"^(\d{3})_(.+)__([A-Za-z0-9]+)\.png$")
UDCAP_RE = re.compile(r"UDCAP;(.*)$")
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")
GPU_FRAME = re.compile(r"\bframe ([\d.]+) ms")
GPU_WORLD = re.compile(r"\bworld ([\d.]+)")
GPU_POST = re.compile(r"\bpost ([\d.]+):")
GPU_COMP = re.compile(r"\bcomposite ([\d.]+)")
ORDER = ["classic", "remastered", "rnoaniso", "rfix", "rfull", "rfullb", "rcas", "rmacro", "rall",
         "rbias", "rcas5", "rcas8", "rdet1", "rdet2", "rrel3", "rrel8", "rmac", "rcr", "rbox", "rlz", "rlzc",
         "rlzcb", "rbest", "rd1", "rd2", "rd2t", "rsd1", "rsd2", "rsdk", "rbest2", "rbest3", "rsl1", "rsl15", "rsl2", "rsl15o", "rsl15k", "rcas3l", "rfinal", "rsl15r", "rsl15z4", "rsl15z8", "rfinalr"]
NAMES = {
    "classic": "Classic",
    "remastered": "Remastered (current)",
    "rnoaniso": "Remastered, aniso off (diagnostic)",
    "rfix": "Remastered + residency fix",
    "rfull": "Remastered + full-res mips",
    "rfullb": "Remastered + full-res + LOD bias -0.5",
    "rcas": "Remastered + fix + CAS 0.5",
    "rmacro": "Remastered + fix + macro detail",
    "rall": "Remastered + fix + macro + CAS",
    "rbias": "Remastered + LOD bias -0.5",
    "rcas5": "Remastered + CAS 0.5",
    "rcas8": "Remastered + CAS 0.8",
    "rdet1": "Remastered + detail contrast kept (K2=1)",
    "rdet2": "Remastered + detail contrast kept (K2=2)",
    "rrel3": "Remastered + relief from albedo 0.3 m",
    "rrel8": "Remastered + relief from albedo 0.8 m",
    "rmac": "Remastered + macro detail 0.6",
    "rcr": "Remastered + Catmull-Rom terrain magnification",
    "rbox": "Remastered + remip box (control)",
    "rlz": "Remastered + remip Lanczos-2",
    "rlzc": "Remastered + remip Lanczos-2 + alpha coverage",
    "rlzcb": "Remastered + remip Lanczos + coverage + bias",
    "rbest": "Remastered + texsharp combined",
    "rd1": "Remastered + detail kept K2=1",
    "rd2": "Remastered + detail kept K2=2",
    "rd2t": "Remastered + detail kept K2=2 + anti-tiling",
    "rsd1": "Remastered + self-detail 1.0",
    "rsd2": "Remastered + self-detail 2.0",
    "rsdk": "Remastered + self-detail 1 + detail kept + anti-tiling",
    "rbest2": "Remastered + texsharp v6 combined",
    "rbest3": "Remastered + texsharp combined (self-detail, detail kept, CAS 0.5, bias -0.5, CR)",
    "rsl1": "Remastered + self-detail 1.0 (limited)",
    "rsl15": "Remastered + self-detail 1.5 (limited)",
    "rsl2": "Remastered + self-detail 2.0 (limited)",
    "rsl15o": "Remastered + self-detail 1.5 + 2nd octave 0.6",
    "rsl15k": "Remastered + self-detail 1.5 + detail kept + anti-tiling",
    "rcas3l": "Remastered + CAS 0.3 luma only",
    "rfinal": "Remastered + texsharp (self-detail 1.5, detail kept, CAS 0.3 luma)",
    "rsl15r": "Remastered + self-detail 1.5, rotated cells",
    "rsl15z4": "Remastered + self-detail 1.5, zoom 4",
    "rsl15z8": "Remastered + self-detail 1.5, zoom 8",
    "rfinalr": "Remastered + texsharp, rotated cells (self-detail 1.5, detail kept, CAS 0.3 luma)",
}


def font(size):
    for name in ("arialbd.ttf", "arial.ttf", "segoeui.ttf"):
        p = Path("C:/Windows/Fonts") / name
        if p.exists():
            return ImageFont.truetype(str(p), size)
    return ImageFont.load_default()


def label_box(d, xy, text, fnt):
    box = d.textbbox(xy, text, font=fnt)
    d.rectangle([box[0] - 5, box[1] - 3, box[2] + 5, box[3] + 3], fill=(0, 0, 0))
    d.text(xy, text, font=fnt, fill=(255, 255, 255))


def collect(run: Path):
    tri = run / "tri"
    shots = run / "shots"
    shots.mkdir(exist_ok=True)
    best = {}
    for p in tri.rglob("*.png"):
        m = SHOT_RE.match(p.name)
        if not m:
            continue
        key = (m.group(2), m.group(3))
        if key not in best or p.stat().st_mtime > best[key].stat().st_mtime:
            best[key] = p
    for (v, l), src in best.items():
        shutil.copy2(src, shots / f"{v}__{l}.png")
    gpu = {}
    knobs = []
    for p in sorted(tri.rglob("*.log")):
        for raw in p.read_text(encoding="utf-8", errors="replace").splitlines():
            line = ANSI_RE.sub("", raw)
            m = UDCAP_RE.search(line)
            if not m:
                continue
            body = m.group(1).strip().rstrip('"')
            if "event=knob" in body:
                knobs.append(body)
            if "event=shot" not in body:
                continue
            g = body.split(";gpu=", 1)[1] if ";gpu=" in body else ""
            lab = re.search(r"label=([^;]+)", body).group(1)
            v, l = lab.rsplit("__", 1)
            row = {}
            for k, rx in (("frame", GPU_FRAME), ("world", GPU_WORLD), ("post", GPU_POST), ("composite", GPU_COMP)):
                mm = rx.search(g)
                row[k] = float(mm.group(1)) if mm else None
            row["raw"] = g
            gpu[(v, l)] = row
    bad = [k for k in knobs if "res=OK" not in k]
    return shots, gpu, bad


def lap_bands(path):
    im = np.asarray(Image.open(path).convert("L"), dtype=np.float32)
    lap = np.abs(4 * im[1:-1, 1:-1] - im[:-2, 1:-1] - im[2:, 1:-1] - im[1:-1, :-2] - im[1:-1, 2:])
    h = lap.shape[0]
    bands = {"far": (0.36, 0.48), "mid": (0.48, 0.62), "near": (0.62, 1.0)}
    return {k: float(lap[int(a * h):int(b * h)].mean()) for k, (a, b) in bands.items()}


def panel(img, title, ms, fnt_big, fnt_small):
    im = img.copy()
    d = ImageDraw.Draw(im)
    label_box(d, (10, 8), title, fnt_big)
    if ms is not None:
        label_box(d, (10, im.height - 28), f"GPU frame {ms:.2f} ms", fnt_small)
    return im


def grid(view, shots, gpu, out, cols=3):
    labs = [l for l in ORDER if (shots / f"{view}__{l}.png").exists()]
    if not labs:
        return None
    ims = [Image.open(shots / f"{view}__{l}.png").convert("RGB") for l in labs]
    w, h = ims[0].size
    rows = (len(ims) + cols - 1) // cols
    sheet = Image.new("RGB", (cols * w + (cols - 1) * 6, rows * h + (rows - 1) * 6 + 36), (20, 20, 20))
    d = ImageDraw.Draw(sheet)
    d.text((8, 6), f"{view}: texsharp variants (960x640 presented frames)", font=font(22), fill=(230, 230, 230))
    for i, (l, im) in enumerate(zip(labs, ims)):
        r, c = divmod(i, cols)
        g = gpu.get((view, l), {}).get("frame")
        sheet.paste(panel(im, NAMES.get(l, l), g, font(20), font(15)), (c * (w + 6), 36 + r * (h + 6)))
    dst = out / f"{view}__strip.png"
    sheet.save(dst)
    return dst


def three_way(view, best, shots, gpu, out, crop=None):
    labs = ["classic", "remastered", best]
    paths = [shots / f"{view}__{l}.png" for l in labs]
    if not all(p.exists() for p in paths):
        return None
    ims = [Image.open(p).convert("RGB") for p in paths]
    if crop:
        ims = [im.crop(crop) for im in ims]
        ims = [im.resize((im.width * 2, im.height * 2), Image.NEAREST) for im in ims]
    w, h = ims[0].size
    sheet = Image.new("RGB", (3 * w + 12, h + 36), (20, 20, 20))
    d = ImageDraw.Draw(sheet)
    tag = f" crop {crop} at 2x" if crop else ""
    d.text((8, 6), f"{view}: Classic | Remastered (current) | {NAMES.get(best, best)}{tag}", font=font(20),
           fill=(230, 230, 230))
    for i, (l, im) in enumerate(zip(labs, ims)):
        g = gpu.get((view, l), {}).get("frame")
        sheet.paste(panel(im, NAMES.get(l, l), g, font(20), font(15)), (i * (w + 6), 36))
    dst = out / (f"{view}__3way_{best}" + ("_crop" if crop else "") + ".png")
    sheet.save(dst)
    return dst


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--best", default="rall")
    ap.add_argument("--crop", default=None)
    ap.add_argument("--views", default=None)
    a = ap.parse_args()
    run = Path(a.run)
    shots, gpu, bad = collect(run)
    views = sorted({p.stem.rsplit("__", 1)[0] for p in shots.glob("*.png")})
    if a.views:
        views = [v for v in views if v in a.views.split(",")]
    crop = tuple(int(x) for x in a.crop.split(",")) if a.crop else None
    rows = []
    for v in views:
        grid(v, shots, gpu, run)
        three_way(v, a.best, shots, gpu, run)
        if crop:
            three_way(v, a.best, shots, gpu, run, crop)
        base = None
        for l in ORDER:
            p = shots / f"{v}__{l}.png"
            if not p.exists():
                continue
            m = lap_bands(p)
            if l == "remastered":
                base = m
            g = gpu.get((v, l), {})
            rows.append(dict(view=v, label=l, **{f"gpu_{k}": g.get(k) for k in ("frame", "world", "post", "composite")},
                             **{f"lap_{k}": val for k, val in m.items()}))
    (run / "metrics.json").write_text(json.dumps(rows, indent=2), encoding="utf-8")
    md = ["| view | variant | GPU frame | world | post | composite | lap far | lap mid | lap near |",
          "|---|---|---|---|---|---|---|---|---|"]
    f = lambda x: "" if x is None else f"{x:.2f}"
    for r in rows:
        md.append(f"| {r['view']} | {r['label']} | {f(r['gpu_frame'])} | {f(r['gpu_world'])} | {f(r['gpu_post'])} | "
                  f"{f(r['gpu_composite'])} | {f(r['lap_far'])} | {f(r['lap_mid'])} | {f(r['lap_near'])} |")
    (run / "metrics.md").write_text("\n".join(md) + "\n", encoding="utf-8")
    print("\n".join(md))
    if bad:
        print("KNOBS NOT OK:", bad[:10])


if __name__ == "__main__":
    main()
