"""Collect a shadows-track research run: copy shots, parse timings, build
labelled three-way comparisons (Classic | Remastered | Remastered + variant).

    python collect.py <run_dir> [--views v1,v2] [--crop x0,y0,x1,y1:view]

<run_dir> is the -OutDir of run-captures.ps1 (with tri/ inside).
Writes <run_dir>/shots/<view>__<label>.png, timings_<run>.md/.json and
<run_dir>/cmp/<view>__<variant>.png (3-way) plus zoomed crops.
"""
import argparse
import json
import re
import shutil
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

SHOT_RE = re.compile(r"^(\d{3})_(.+?)__(.+)\.png$")
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")
UDCAP_RE = re.compile(r"UDCAP;(.*?)\"?$")
GPU_RES = {
    "frame": re.compile(r"\bframe ([\d.]+) ms"),
    "world": re.compile(r"\bworld ([\d.]+)"),
    "shadow": re.compile(r"\(shadow ([\d.]+)\)"),
    "post": re.compile(r"\bpost ([\d.]+):"),
    "ao": re.compile(r"\bao ([\d.]+)"),
    "composite": re.compile(r"\bcomposite ([\d.]+)"),
}

LABELS = {
    "classic": "Classic",
    "rem": "Remastered (current)",
    "contact": "+ contact shadows",
    "pcss05": "+ PCSS 0.53 deg sun, 4096, contact",
    "pcss2": "+ PCSS 2 deg sun, 4096, contact",
    "pcss2lod1": "+ PCSS 2 deg, 4096, contact, full-LOD casters",
    "pcss1": "+ PCSS 1 deg sun, 4096",
    "pcss1tight": "+ PCSS 1 deg, 4096, tight near tiers",
    "far": "+ far-field screen-space shadows",
    "combo": "+ PCSS 1 deg, 4096 tight, far contact, full-LOD casters",
    "combo3": "+ PCSS 3 deg, 4096 tight, far contact, full-LOD casters",
    "obj": "+ tree/building casters on",
    "best": "+ PCSS 1 deg, 4096, tree/building casters, contact",
    "bestlong": "+ best, shadow range x2",
    "objpcss": "+ tree/building casters, PCSS 1 deg, 4096 tight",
    "objfull": "+ tree/building casters, PCSS 2 deg, 4096 tight, contact",
    "objpcss2k": "+ tree/building casters, PCSS 1 deg, stock 2048 map",
    "dbgsunvisbest": "debug: sun visibility of best",
    "prod": "+ tree/building casters, PCSS 1 deg, 2048, tight tiers",
    "prodexp": "+ prod, exposure x1.25",
    "dbgsunvisprod": "debug: sun visibility of prod",
}


def font(size):
    for name in ("arialbd.ttf", "arial.ttf", "DejaVuSans-Bold.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            pass
    return ImageFont.load_default()


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


def collect(run_dir):
    tri = run_dir / "tri"
    shots = run_dir / "shots"
    shots.mkdir(exist_ok=True)
    best = {}
    for p in tri.rglob("*.png"):
        m = SHOT_RE.match(p.name)
        if not m:
            continue
        key = (m.group(2), m.group(3))
        if key not in best or p.stat().st_mtime > best[key].stat().st_mtime:
            best[key] = p
    for (view, label), src in best.items():
        shutil.copy2(src, shots / f"{view}__{label}.png")
    timings = {}
    for log in tri.rglob("game_stdout.log"):
        for line in log.read_text(encoding="utf-8", errors="replace").splitlines():
            line = ANSI_RE.sub("", line)
            m = UDCAP_RE.search(line)
            if not m:
                continue
            kv = parse_kv(m.group(1))
            if kv.get("event") != "shot":
                continue
            row = {"label": kv.get("label"), "mode": kv.get("mode"), "gpu": kv.get("gpu", "")}
            for k, rx in GPU_RES.items():
                mm = rx.search(row["gpu"])
                row[k] = float(mm.group(1)) if mm else None
            timings[row["label"]] = row
    (run_dir / "timings.json").write_text(json.dumps(timings, indent=1), encoding="utf-8")
    lines = ["| shot | GPU frame | world | shadow | post | ao | composite |", "|---|---|---|---|---|---|---|"]
    for label in sorted(timings):
        r = timings[label]
        f = lambda k: "" if r[k] is None else "%.2f" % r[k]
        lines.append(f"| {label} | {f('frame')} | {f('world')} | {f('shadow')} | {f('post')} | {f('ao')} | {f('composite')} |")
    (run_dir / "timings.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
    return best, timings


def panel(img, title, ms, w):
    img = img.convert("RGB")
    if img.width != w:
        img = img.resize((w, int(img.height * w / img.width)), Image.LANCZOS)
    d = ImageDraw.Draw(img)
    f = font(max(14, w // 40))
    tb = d.textbbox((0, 0), title, font=f)
    d.rectangle((6, 6, 14 + tb[2], 12 + tb[3]), fill=(0, 0, 0))
    d.text((10, 8), title, font=f, fill=(255, 255, 255))
    if ms is not None:
        t = "GPU frame %.2f ms" % ms
        tb = d.textbbox((0, 0), t, font=f)
        y = img.height - tb[3] - 14
        d.rectangle((6, y - 2, 14 + tb[2], y + tb[3] + 4), fill=(0, 0, 0))
        d.text((10, y), t, font=f, fill=(255, 255, 255))
    return img


def strip(images, gap=6):
    w = sum(i.width for i in images) + gap * (len(images) - 1)
    h = max(i.height for i in images)
    out = Image.new("RGB", (w, h), (20, 20, 20))
    x = 0
    for i in images:
        out.paste(i, (x, 0))
        x += i.width + gap
    return out


def compare(run_dir, timings, views, crops):
    shots = run_dir / "shots"
    cmp_dir = run_dir / "cmp"
    cmp_dir.mkdir(exist_ok=True)
    made = []
    for view in views:
        base = shots / f"{view}__classic.png"
        rem = shots / f"{view}__rem.png"
        if not base.exists() or not rem.exists():
            continue
        others = sorted(p for p in shots.glob(f"{view}__*.png") if p.stem.split("__", 1)[1] not in ("classic", "rem", "reset"))
        for p in others:
            label = p.stem.split("__", 1)[1]
            ims = []
            for path, lab in ((base, "classic"), (rem, "rem"), (p, label)):
                t = timings.get(f"{view}__{lab}", {})
                ims.append(panel(Image.open(path), LABELS.get(lab, "+ " + lab), t.get("frame"), 960))
            out = cmp_dir / f"{view}__{label}.png"
            strip(ims).save(out)
            made.append(out)
            if view in crops:
                x0, y0, x1, y1 = crops[view]
                zs = []
                for path, lab in ((base, "classic"), (rem, "rem"), (p, label)):
                    c = Image.open(path).convert("RGB").crop((x0, y0, x1, y1))
                    c = c.resize((c.width * 2, c.height * 2), Image.LANCZOS)
                    zs.append(panel(c, LABELS.get(lab, "+ " + lab) + " (2x crop)", None, c.width))
                outz = cmp_dir / f"{view}__{label}__zoom.png"
                strip(zs).save(outz)
                made.append(outz)
    return made


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run_dir")
    ap.add_argument("--views", default="lowsun_fp,noon_fields,forest_edge_golden,sinai_lowsun_fp,forest_side_golden,town_golden")
    args = ap.parse_args()
    run_dir = Path(args.run_dir)
    best, timings = collect(run_dir)
    print("shots:", len(best), "timed:", len(timings))
    crops = {
        "lowsun_fp": (0, 300, 620, 560),
        "noon_fields": (230, 380, 760, 560),
        "forest_edge_golden": (420, 330, 760, 560),
        "sinai_lowsun_fp": (0, 280, 640, 560),
        "forest_side_golden": (0, 250, 960, 640),
        "town_golden": (0, 250, 960, 640),
    }
    made = compare(run_dir, timings, args.views.split(","), crops)
    for m in made:
        print(m)


if __name__ == "__main__":
    main()
