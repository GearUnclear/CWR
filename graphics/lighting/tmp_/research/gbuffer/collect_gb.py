"""Collect a gbuffer research capture run.

    python collect_gb.py <run_dir> [--proto gi] [--views a,b] [--extra label,label]

Copies NNN_<view>__<label>.png out of <run_dir>/tri, parses the UDCAP shot
lines (GPU timings), and writes into <run_dir>:
    <view>__<label>.png        the captures
    cmp_<view>__<proto>.png    Classic | Remastered | <proto>, GPU frame ms on each panel
    dbg_<view>.png             G-buffer debug sheet (if debug shots exist)
    diff_<view>__<proto>.png   |proto - remastered| x4 (where the prototype changed the frame)
    timings_gb.md / timings_gb.json
"""
import argparse
import json
import re
import shutil
import sys
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageFont

SHOT_RE = re.compile(r"^(\d{3})_(.+?)__(.+)\.png$")
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")
UDCAP_RE = re.compile(r"UDCAP;(.*)$")
GPU_RES = {
    "frame": re.compile(r"\bframe ([\d.]+) ms"),
    "world": re.compile(r"\bworld ([\d.]+)"),
    "shadow": re.compile(r"\(shadow ([\d.]+)\)"),
    "post": re.compile(r"\bpost ([\d.]+):"),
    "ao": re.compile(r"\bao ([\d.]+)"),
    "bloom": re.compile(r"\bbloom ([\d.]+)"),
    "shafts": re.compile(r"\bshafts ([\d.]+)"),
    "composite": re.compile(r"\bcomposite ([\d.]+)"),
}

PRETTY = {
    "classic": "Classic",
    "remastered": "Remastered (current)",
    "rm_vbo": "Remastered + late VBO (vehicles lit per pixel)",
    "gbufonly": "+ G-buffer writes only",
    "gi": "+ G-buffer GI (r 6 m)",
    "gi_src1": "+ G-buffer GI, whole-frame source",
    "gi_r3": "+ G-buffer GI (r 3 m)",
    "giao": "+ G-buffer ray AO only",
}
DEBUG_ORDER = [
    ("gi", "final: G-buffer GI"),
    ("dbg_normals", "view-space normals"),
    ("dbg_rough", "roughness (white = matte)"),
    ("dbg_direct", "non-sky radiance (sun/moon/local)"),
    ("dbg_albedo", "ambient response (albedo x ka)"),
    ("dbg_bounce", "GI bounce irradiance"),
    ("dbg_vis", "GI sky visibility"),
    ("dbg_sky", "sky/ambient radiance (frame - direct)"),
]


def font(size):
    for name in ("arialbd.ttf", "arial.ttf", "segoeui.ttf"):
        p = Path("C:/Windows/Fonts") / name
        if p.exists():
            return ImageFont.truetype(str(p), size)
    return ImageFont.load_default()


def label_box(draw, xy, text, fnt):
    x, y = xy
    box = draw.textbbox((x, y), text, font=fnt)
    draw.rectangle([box[0] - 6, box[1] - 4, box[2] + 6, box[3] + 4], fill=(0, 0, 0))
    draw.text((x, y), text, font=fnt, fill=(255, 255, 255))


def collect(tri_dir: Path, out_dir: Path):
    best = {}
    for p in tri_dir.rglob("*.png"):
        m = SHOT_RE.match(p.name)
        if not m:
            continue
        key = (m.group(2), m.group(3))
        if key not in best or p.stat().st_mtime > best[key].stat().st_mtime:
            best[key] = p
    for (view, label), src in best.items():
        shutil.copy2(src, out_dir / f"{view}__{label}.png")
    return sorted(best.keys())


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


def read_rows(tri_dir: Path):
    rows = {}
    other = []
    for p in sorted(tri_dir.rglob("*.log")):
        try:
            text = p.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        for raw in text.splitlines():
            line = ANSI_RE.sub("", raw)
            if "gi history reset" in line or "gbuffer prototype" in line or "compile error" in line or "link error" in line:
                other.append(line.strip())
            m = UDCAP_RE.search(line)
            if not m:
                continue
            kv = parse_kv(m.group(1).strip().rstrip('"'))
            if kv.get("event") != "shot":
                continue
            label = kv.get("label", "")
            if "__" not in label:
                continue
            view, mode = label.split("__", 1)
            row = {"view": view, "mode": mode, "gpu_raw": kv.get("gpu", ""), "lit": kv.get("lit", "")}
            for f, rx in GPU_RES.items():
                mm = rx.search(row["gpu_raw"])
                row[f] = float(mm.group(1)) if mm else None
            rows[(view, mode)] = row
    return rows, other


def three_way(view, proto, out_dir, rows, base=("classic", "remastered"), tag=""):
    labels = list(base) + [proto]
    imgs = []
    for l in labels:
        p = out_dir / f"{view}__{l}.png"
        imgs.append(Image.open(p).convert("RGB") if p.exists() else None)
    if imgs[-1] is None:
        return None
    w = max(i.width for i in imgs if i)
    h = max(i.height for i in imgs if i)
    gap, title_h = 8, 40
    sheet = Image.new("RGB", (w * len(labels) + gap * (len(labels) - 1), h + title_h), (24, 24, 24))
    d = ImageDraw.Draw(sheet)
    big, small = font(22), font(15)
    d.text((10, 8), view, font=big, fill=(235, 235, 235))
    for i, (l, im) in enumerate(zip(labels, imgs)):
        x0 = i * (w + gap)
        if im:
            sheet.paste(im, (x0, title_h))
        else:
            d.rectangle([x0, title_h, x0 + w, title_h + h], fill=(60, 0, 0))
        label_box(d, (x0 + 12, title_h + 10), PRETTY.get(l, l), big)
        r = rows.get((view, l))
        if r and r.get("frame") is not None:
            txt = f"GPU frame {r['frame']:.2f} ms | world {r['world'] or 0:.2f} | post {r['post'] or 0:.2f} (ao/gi {r['ao'] or 0:.2f})"
            label_box(d, (x0 + 12, title_h + h - 30), txt, small)
    dst = out_dir / f"cmp{tag}_{view}__{proto}.png"
    sheet.save(dst)
    return dst


def diff(view, proto, out_dir, ref="remastered", gain=4):
    tagref = "" if ref == "remastered" else "_vs_" + ref
    a = out_dir / f"{view}__{ref}.png"
    b = out_dir / f"{view}__{proto}.png"
    if not (a.exists() and b.exists()):
        return None
    ia, ib = Image.open(a).convert("RGB"), Image.open(b).convert("RGB")
    dd = ImageChops.difference(ia, ib).point(lambda v: min(255, v * gain))
    dst = out_dir / f"diff_{view}__{proto}{tagref}.png"
    dd.save(dst)
    return dst


def debug_sheet(view, out_dir, rows):
    tiles = []
    for l, title in DEBUG_ORDER:
        p = out_dir / f"{view}__{l}.png"
        if p.exists():
            tiles.append((Image.open(p).convert("RGB"), title))
    if len(tiles) < 2:
        return None
    tw = 640
    th = int(tiles[0][0].height * tw / tiles[0][0].width)
    cols = 4
    rows_n = (len(tiles) + cols - 1) // cols
    gap = 6
    sheet = Image.new("RGB", (cols * tw + (cols - 1) * gap, rows_n * th + (rows_n - 1) * gap + 36), (20, 20, 20))
    d = ImageDraw.Draw(sheet)
    d.text((8, 6), f"{view}: G-buffer targets (debug 5..11) and the final frame", font=font(20), fill=(235, 235, 235))
    for i, (im, title) in enumerate(tiles):
        r, c = divmod(i, cols)
        x0, y0 = c * (tw + gap), 36 + r * (th + gap)
        sheet.paste(im.resize((tw, th), Image.LANCZOS), (x0, y0))
        label_box(d, (x0 + 8, y0 + 8), title, font(15))
    dst = out_dir / f"dbg_{view}.png"
    sheet.save(dst)
    return dst


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run_dir")
    ap.add_argument("--proto", default="gi")
    ap.add_argument("--extra", default="")
    args = ap.parse_args()
    out = Path(args.run_dir)
    tri = out / "tri"
    keys = collect(tri, out)
    rows, other = read_rows(tri)
    views = sorted({v for v, _ in keys})
    protos = [args.proto] + [x for x in args.extra.split(",") if x]
    made = []
    for v in views:
        if (out / f"{v}__rm_vbo.png").exists():
            f4 = three_way(v, "gi", out, rows, base=("classic", "remastered", "rm_vbo"), tag="4")
            if f4:
                made.append(f4.name)
            for pr in protos:
                f = diff(v, pr, out, ref="rm_vbo")
                if f:
                    made.append(f.name)
        for pr in protos:
            for f in (three_way(v, pr, out, rows), diff(v, pr, out)):
                if f:
                    made.append(f.name)
        f = debug_sheet(v, out, rows)
        if f:
            made.append(f.name)
    # timings
    md = ["| view | shot | GPU frame | world | shadow | post | ao/gi | bloom | shafts | composite |",
          "|---|---|---|---|---|---|---|---|---|---|"]
    for (v, m), r in rows.items():
        def f(k):
            return "" if r.get(k) is None else f"{r[k]:.2f}"
        md.append(f"| {v} | {m} | {f('frame')} | {f('world')} | {f('shadow')} | {f('post')} | {f('ao')} | {f('bloom')} | {f('shafts')} | {f('composite')} |")
    (out / "timings_gb.md").write_text("\n".join(md) + "\n\nengine log lines:\n\n" + "\n".join(other[:80]) + "\n", encoding="utf-8")
    (out / "timings_gb.json").write_text(json.dumps([r for r in rows.values()], indent=1), encoding="utf-8")
    print("shots:", len(keys), "views:", views)
    print("made:", made)
    print("\n".join(md))
    print("\n".join(other[:40]))


if __name__ == "__main__":
    main()
