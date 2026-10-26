"""Atmosphere research track: N-way comparison panels from a capture run.

    python make_compare.py <run_dir> [--modes classic,remastered,atmo] [--labels "Classic,Remastered,Remastered + atmosphere"]

Copies NNN_<view>__<mode>.png out of <run_dir>/tri, parses the UDCAP shot lines
(GPU frame / world / post ms), and writes into <run_dir>:
    <view>__<mode>.png          the captures
    <view>__compare.png         the modes side by side, labelled, GPU frame ms on each panel
    timings.md                  every shot with its GPU numbers
"""
import argparse
import re
import shutil
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

SHOT_RE = re.compile(r"^(\d{3})_(.+)__([A-Za-z0-9_-]+)\.png$")
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")
UDCAP_RE = re.compile(r"UDCAP;(.*)$")
GPU_RES = {
    "frame": re.compile(r"\bframe ([\d.]+) ms"),
    "world": re.compile(r"\bworld ([\d.]+)"),
    "post": re.compile(r"\bpost ([\d.]+):"),
}


def font(size):
    for name in ("arialbd.ttf", "arial.ttf", "segoeui.ttf"):
        p = Path("C:/Windows/Fonts") / name
        if p.exists():
            return ImageFont.truetype(str(p), size)
    return ImageFont.load_default()


def label_box(d, xy, text, fnt):
    x, y = xy
    box = d.textbbox((x, y), text, font=fnt)
    d.rectangle([box[0] - 6, box[1] - 4, box[2] + 6, box[3] + 4], fill=(0, 0, 0))
    d.text((x, y), text, font=fnt, fill=(255, 255, 255))


def parse_kv(body):
    gpu = None
    if ";gpu=" in body:
        body, gpu = body.split(";gpu=", 1)
    out = {}
    for part in body.split(";"):
        if "=" in part:
            k, v = part.split("=", 1)
            out[k.strip()] = v.strip()
    if gpu is not None:
        out["gpu"] = gpu.strip().rstrip('"')
    return out


def main(argv):
    ap = argparse.ArgumentParser()
    ap.add_argument("run_dir")
    ap.add_argument("--modes", default="classic,remastered,atmo")
    ap.add_argument("--labels", default="Classic,Remastered (current),Remastered + atmosphere")
    ap.add_argument("--suffix", default="compare")
    a = ap.parse_args(argv)
    run = Path(a.run_dir).resolve()
    tri = run / "tri"
    modes = a.modes.split(",")
    labels = a.labels.split(",")

    best = {}
    if tri.exists():
        for p in tri.rglob("*.png"):
            m = SHOT_RE.match(p.name)
            if not m:
                continue
            key = (m.group(2), m.group(3))
            if key not in best or p.stat().st_mtime > best[key].stat().st_mtime:
                best[key] = p
        for (view, mode), src in best.items():
            shutil.copy2(src, run / f"{view}__{mode}.png")

    rows = {}
    order = []
    if tri.exists():
        for p in sorted(tri.rglob("*.log")):
            for raw in p.read_text(encoding="utf-8", errors="replace").splitlines():
                m = UDCAP_RE.search(ANSI_RE.sub("", raw))
                if not m:
                    continue
                kv = parse_kv(m.group(1).strip().rstrip('"'))
                if kv.get("event") != "shot":
                    continue
                label = kv.get("label", "")
                if "__" not in label:
                    continue
                view, mode = label.rsplit("__", 1)
                g = kv.get("gpu", "")
                row = {"view": view, "mode": mode, "gpu": g, "lit": kv.get("lit", "")}
                for f, rx in GPU_RES.items():
                    mm = rx.search(g)
                    row[f] = float(mm.group(1)) if mm else None
                rows[(view, mode)] = row
                if view not in order:
                    order.append(view)

    views = order or sorted({v for v, _m in best})
    big, small = font(22), font(15)
    for view in views:
        imgs = []
        for mode in modes:
            p = run / f"{view}__{mode}.png"
            imgs.append(Image.open(p).convert("RGB") if p.exists() else None)
        if not any(imgs):
            continue
        w = max(i.width for i in imgs if i)
        h = max(i.height for i in imgs if i)
        gap, title_h = 8, 40
        n = len(modes)
        out = Image.new("RGB", (w * n + gap * (n - 1), h + title_h), (24, 24, 24))
        d = ImageDraw.Draw(out)
        d.text((10, 8), view, font=big, fill=(235, 235, 235))
        for i, (mode, im) in enumerate(zip(modes, imgs)):
            x0 = i * (w + gap)
            if im:
                out.paste(im, (x0, title_h))
            else:
                d.rectangle([x0, title_h, x0 + w, title_h + h], fill=(60, 0, 0))
                d.text((x0 + 20, title_h + h // 2), "missing", font=big, fill=(255, 160, 160))
            label_box(d, (x0 + 12, title_h + 10), labels[i] if i < len(labels) else mode, big)
            r = rows.get((view, mode))
            if r and r.get("frame") is not None:
                label_box(d, (x0 + 12, title_h + h - 30), f"GPU frame {r['frame']:.2f} ms (world {r['world']:.2f})", small)
        out.save(run / f"{view}__{a.suffix}.png")

    md = ["# Atmosphere research captures", "", "| view | mode | GPU frame ms | world ms | post ms | triSetLighting |",
          "|---|---|---|---|---|---|"]
    for (view, mode), r in rows.items():
        f = lambda v: "" if v is None else f"{v:.2f}"
        md.append(f"| {view} | {mode} | {f(r['frame'])} | {f(r['world'])} | {f(r['post'])} | {r['lit']} |")
    (run / "timings.md").write_text("\n".join(md) + "\n", encoding="utf-8")
    print(f"shots {len(best)}, log rows {len(rows)}, views {views}")


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
