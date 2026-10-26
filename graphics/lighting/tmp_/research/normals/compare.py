"""Collect a normals-track research run into labelled comparison sheets.

    python compare.py <run_dir> [--panels classic,remastered,v2tex] [--name tag] [--crop x0,y0,x1,y1]

Reads <run_dir>/tri/**/NNN_<view>__<label>.png and the UDCAP shot lines of
that tree (GPU frame ms per shot), copies the shots to <run_dir>/shots/, and
writes <run_dir>/<view>__<name>.png: the panels side by side (or in a grid),
each labelled with its name and GPU frame time. With --crop the panels are
crops (same box on every panel) scaled 2x, for close inspection.
"""
import argparse
import re
import shutil
import sys
from pathlib import Path

from PIL import Image, ImageDraw

sys.path.insert(0, r"D:\Arma_CWA_worktrees\lighting\arma_CWR\tests\rendering\captures")
from make_pairs import read_udcap, parse_kv, font, label_box, GPU_RES  # noqa: E402

SHOT_RE = re.compile(r"^(\d{3})_(.+)__([A-Za-z0-9_]+)\.png$")

NICE = {
    "classic": "Classic",
    "remastered": "Remastered (current)",
    "v1ss": "+ screen-space luma bump",
    "v2tex": "+ albedo bump (shader taps)",
    "v3tex2x": "+ albedo bump x2",
    "v4nogate": "+ albedo bump, no chroma gate",
    "v5nmap": "+ load-time normal map",
    "v6nmap2x": "+ load-time normal map x2",
    "v7shad": "+ load-time normal map + micro shadows",
    "v8shad2x": "+ normal map x2 + micro shadows",
    "cur": "Remastered (as shipped)",
    "dbgnormal": "debug: bumped normals",
}


def collect(run: Path):
    best = {}
    for p in (run / "tri").rglob("*.png"):
        m = SHOT_RE.match(p.name)
        if not m:
            continue
        key = (m.group(2), m.group(3))
        if key not in best or p.stat().st_mtime > best[key].stat().st_mtime:
            best[key] = p
    shots = run / "shots"
    shots.mkdir(exist_ok=True)
    for (view, label), src in best.items():
        shutil.copy2(src, shots / f"{view}__{label}.png")
    gpu = {}
    for _p, _line, body in read_udcap(run / "tri"):
        if body is None:
            continue
        kv = parse_kv(body)
        if kv.get("event") != "shot" or "__" not in kv.get("label", ""):
            continue
        view, label = kv["label"].rsplit("__", 1)
        row = {}
        for f, rx in GPU_RES.items():
            m = rx.search(kv.get("gpu", ""))
            row[f] = float(m.group(1)) if m else None
        gpu[(view, label)] = row
    return best, gpu


def sheet(run, view, labels, gpu, name, crop=None, cols=None, scale=1.0):
    imgs = []
    for lab in labels:
        p = run / "shots" / f"{view}__{lab}.png"
        if not p.exists():
            print("missing", p)
            return None
        im = Image.open(p).convert("RGB")
        if crop:
            im = im.crop(crop)
        if scale != 1.0:
            im = im.resize((int(im.width * scale), int(im.height * scale)), Image.LANCZOS)
        imgs.append(im)
    cols = cols or len(imgs)
    rows = (len(imgs) + cols - 1) // cols
    w, h = imgs[0].size
    gap, title_h = 8, 40
    out = Image.new("RGB", (cols * w + (cols - 1) * gap, rows * (h + gap) - gap + title_h), (24, 24, 24))
    d = ImageDraw.Draw(out)
    d.text((10, 8), f"{view}" + (f"  (crop {crop}, x{scale:g})" if crop else ""), font=font(22), fill=(235, 235, 235))
    for i, (lab, im) in enumerate(zip(labels, imgs)):
        r, c = divmod(i, cols)
        x0, y0 = c * (w + gap), title_h + r * (h + gap)
        out.paste(im, (x0, y0))
        label_box(d, (x0 + 10, y0 + 8), NICE.get(lab, lab), font(18))
        g = gpu.get((view, lab), {}).get("frame")
        if g is not None:
            label_box(d, (x0 + 10, y0 + h - 28), f"GPU frame {g:.2f} ms", font(15))
    dst = run / f"{view}__{name}.png"
    out.save(dst)
    return dst


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--panels", default="classic,remastered,v2tex")
    ap.add_argument("--name", default="three")
    ap.add_argument("--views", default="")
    ap.add_argument("--crop", default="")
    ap.add_argument("--scale", type=float, default=1.0)
    ap.add_argument("--cols", type=int, default=0)
    ap.add_argument("--rename", action="append", default=[], help="label=Panel title")
    a = ap.parse_args()
    run = Path(a.run)
    for r in a.rename:
        k, v = r.split("=", 1)
        NICE[k] = v
    best, gpu = collect(run)
    views = sorted({v for v, _ in best}) if not a.views else a.views.split(",")
    labels = a.panels.split(",")
    crop = tuple(int(x) for x in a.crop.split(",")) if a.crop else None
    for v in views:
        dst = sheet(run, v, labels, gpu, a.name, crop, a.cols or None, a.scale)
        if dst:
            print(dst)
    # timing table
    with open(run / "gpu_frames.md", "w") as f:
        f.write("| view | label | gpu frame | world | post |\n|---|---|---|---|---|\n")
        for (v, lab), row in sorted(gpu.items()):
            f.write(f"| {v} | {lab} | {row.get('frame')} | {row.get('world')} | {row.get('post')} |\n")


if __name__ == "__main__":
    main()
