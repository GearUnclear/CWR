r"""Collect a foliage research capture run into labelled comparison sheets.

    python collect.py <run_dir> [--compare remastered,vB] [--crop x0,y0,x1,y1]

<run_dir> is the -OutDir of run-captures.ps1 (it holds tri\). Copies every
NNN_<view>__<label>.png out of the tri tree as <view>__<label>.png, reads the
UDCAP shot lines for GPU frame / world ms, and writes:
    <view>__all.png      every label of the view side by side (2 per row),
                         GPU frame ms on each panel
    timings.md           one row per shot
make_three() builds Classic | Remastered | <variant> strips for a view.
"""
import argparse
import os
import re
import shutil
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

SHOT_RE = re.compile(r"^(\d{3})_(.+)__([A-Za-z0-9]+)\.png$")
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")
UDCAP_RE = re.compile(r"UDCAP;(.*)$")
FRAME_RE = re.compile(r"\bframe ([\d.]+) ms")
WORLD_RE = re.compile(r"\bworld ([\d.]+)")
ORDER = ["classic", "remastered", "vA", "vB", "vC", "vD", "vE", "vF", "dbgdepth", "dbgtrans", "dbghash"]
NAMES = {"classic": "Classic", "remastered": "Remastered (current)"}


def font(size):
    for name in ("arialbd.ttf", "arial.ttf"):
        p = Path(os.environ.get("WINDIR", "C:/Windows")) / "Fonts" / name
        if p.exists():
            return ImageFont.truetype(str(p), size)
    return ImageFont.load_default()


def label_box(d, xy, text, fnt):
    box = d.textbbox(xy, text, font=fnt)
    d.rectangle([box[0] - 5, box[1] - 3, box[2] + 5, box[3] + 3], fill=(0, 0, 0))
    d.text(xy, text, font=fnt, fill=(255, 255, 255))


def collect(run: Path):
    tri = run / "tri"
    best = {}
    for p in tri.rglob("*.png"):
        m = SHOT_RE.match(p.name)
        if m:
            key = (m.group(2), m.group(3))
            if key not in best or p.stat().st_mtime > best[key].stat().st_mtime:
                best[key] = p
    for (v, lab), src in best.items():
        shutil.copy2(src, run / f"{v}__{lab}.png")
    rows = {}
    for p in sorted(tri.rglob("*.log")):
        for raw in p.read_text(encoding="utf-8", errors="replace").splitlines():
            m = UDCAP_RE.search(ANSI_RE.sub("", raw))
            if not m:
                continue
            body = m.group(1).strip().rstrip('"')
            gpu = ""
            if ";gpu=" in body:
                body, gpu = body.split(";gpu=", 1)
            kv = dict(x.split("=", 1) for x in body.split(";") if "=" in x)
            if kv.get("event") != "shot":
                continue
            v, lab = kv["label"].rsplit("__", 1)
            fm = FRAME_RE.search(gpu)
            wm = WORLD_RE.search(gpu)
            rows[(v, lab)] = {"frame": float(fm.group(1)) if fm else None,
                              "world": float(wm.group(1)) if wm else None, "gpu": gpu.strip(),
                              "lit": kv.get("lit", "")}
    return sorted({k[0] for k in best}), {k[1] for k in best}, rows


def panel(run, view, lab, rows, title=None):
    p = run / f"{view}__{lab}.png"
    if not p.exists():
        return None
    im = Image.open(p).convert("RGB")
    d = ImageDraw.Draw(im)
    label_box(d, (10, 8), title or NAMES.get(lab, lab), font(20))
    r = rows.get((view, lab))
    if r and r["frame"] is not None and not lab.startswith("dbg"):
        label_box(d, (10, im.height - 26), f"GPU frame {r['frame']:.2f} ms (world {r['world']:.2f})", font(14))
    return im


def grid(ims, cols, title):
    ims = [i for i in ims if i is not None]
    if not ims:
        return None
    w, h = ims[0].size
    gap, th = 6, 34
    rows = (len(ims) + cols - 1) // cols
    out = Image.new("RGB", (cols * w + (cols - 1) * gap, th + rows * h + (rows - 1) * gap), (20, 20, 20))
    ImageDraw.Draw(out).text((8, 6), title, font=font(20), fill=(235, 235, 235))
    for i, im in enumerate(ims):
        r, c = divmod(i, cols)
        out.paste(im, (c * (w + gap), th + r * (h + gap)))
    return out


def make_three(run, view, variant, rows, dst, crop=None, vname=None):
    labs = ["classic", "remastered", variant]
    names = {variant: vname or f"Remastered + foliage {variant}"}
    ims = []
    for lab in labs:
        im = panel(run, view, lab, rows, names.get(lab))
        if im is not None and crop:
            # crop first, then relabel on the crop
            raw = Image.open(run / f"{view}__{lab}.png").convert("RGB").crop(crop)
            s = 960 / raw.width
            raw = raw.resize((960, int(raw.height * s)), Image.LANCZOS)
            d = ImageDraw.Draw(raw)
            label_box(d, (10, 8), (names.get(lab) or NAMES.get(lab, lab)) + " (zoom)", font(20))
            r = rows.get((view, lab))
            if r and r["frame"] is not None:
                label_box(d, (10, raw.height - 26), f"GPU frame {r['frame']:.2f} ms", font(14))
            im = raw
        ims.append(im)
    g = grid(ims, 3, f"{view}: Classic | Remastered (current) | Remastered + foliage {variant}")
    if g:
        g.save(dst)
    return dst


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    a = ap.parse_args()
    run = Path(a.run)
    views, labs, rows = collect(run)
    for v in views:
        have = [l for l in ORDER if (run / f"{v}__{l}.png").exists()]
        have += sorted(l for l in labs if l not in ORDER and (run / f"{v}__{l}.png").exists())
        g = grid([panel(run, v, l, rows) for l in have], 2, f"{v}: {' | '.join(have)}")
        if g:
            g.save(run / f"{v}__all.png")
    with open(run / "timings.md", "w", encoding="utf-8") as f:
        f.write("| view | label | GPU frame ms | world ms | lit | gpu summary |\n|---|---|---|---|---|---|\n")
        for v in views:
            for l in ORDER + sorted(labs):
                r = rows.get((v, l))
                if r and l in ORDER or (r and l not in ORDER):
                    if r is None:
                        continue
                    f.write(f"| {v} | {l} | {r['frame']} | {r['world']} | {r['lit']} | {r['gpu']} |\n")
                    rows.pop((v, l), None)
    print("views:", views)


if __name__ == "__main__":
    main()
