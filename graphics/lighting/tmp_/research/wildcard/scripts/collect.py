"""Collect a wildcard research capture run and build labelled comparison strips.

    python collect.py <run_dir> [--strip label1,label2,...] [--baseline <dir>]

Copies NNN_<view>__<label>.png out of <run_dir>/tri (newest wins) into
<run_dir>/<view>__<label>.png, parses the UDCAP shot lines into
<run_dir>/timings.md, and writes <run_dir>/strip_<view>.png with one panel per
label (GPU frame ms on each panel). A label "shipped" takes
<baseline>/<view>__remastered.png (the shipped dist/lighting-rwdi exe, run
20261017-final) when that file exists.
"""
import argparse
import re
import shutil
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

SHOT_RE = re.compile(r"^(\d{3})_(.+?)__(.+)\.png$")
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")
UDCAP_RE = re.compile(r"UDCAP;(.*)$")
FRAME_RE = re.compile(r"\bframe ([\d.]+) ms")
WORLD_RE = re.compile(r"\bworld ([\d.]+)")
SHADOW_RE = re.compile(r"\(shadow ([\d.]+)\)")
POST_RE = re.compile(r"\bpost ([\d.]+):")

NAMES = {
    "classic": "Classic",
    "shipped": "Remastered (shipped exe)",
    "remastered": "Remastered (this build, maps off)",
    "svm_occ": "+ sky visibility maps",
    "svm_full": "+ sky maps + one bounce",
    "dbg_vis": "debug: sky visibility",
    "dbg_bounce": "debug: bounce x2",
}


def font(size):
    for f in ("arialbd.ttf", "arial.ttf", "DejaVuSans-Bold.ttf"):
        try:
            return ImageFont.truetype(f, size)
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


def collect(run: Path):
    tri = run / "tri"
    best = {}
    for p in tri.rglob("*.png"):
        m = SHOT_RE.match(p.name)
        if not m:
            continue
        key = (m.group(2), m.group(3))
        if key not in best or p.stat().st_mtime > best[key].stat().st_mtime:
            best[key] = p
    for (view, label), src in best.items():
        shutil.copy2(src, run / f"{view}__{label}.png")
    rows = {}
    raw = []
    for p in sorted(tri.rglob("*.log")):
        try:
            text = p.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        for line in text.splitlines():
            line = ANSI_RE.sub("", line)
            m = UDCAP_RE.search(line)
            if not m:
                continue
            body = m.group(1).strip().rstrip('"')
            raw.append(body)
            kv = parse_kv(body)
            if kv.get("event") != "shot":
                continue
            lab = kv.get("label", "")
            if "__" not in lab:
                continue
            view, label = lab.split("__", 1)
            g = kv.get("gpu", "")
            def num(rx):
                mm = rx.search(g)
                return float(mm.group(1)) if mm else None
            rows[(view, label)] = dict(frame=num(FRAME_RE), world=num(WORLD_RE), shadow=num(SHADOW_RE),
                                       post=num(POST_RE), gpu=g, perf=kv.get("perf", ""))
    (run / "udcap.log").write_text("\n".join(raw) + "\n", encoding="utf-8")
    lines = ["# timings (%s)" % run.name, "", "| view | label | GPU frame ms | world | shadow (incl. sky maps) | post | raw |",
             "|---|---|---|---|---|---|---|"]
    for (view, label), r in sorted(rows.items()):
        lines.append("| %s | %s | %s | %s | %s | %s | %s |" % (view, label, r["frame"], r["world"], r["shadow"], r["post"], r["gpu"]))
    (run / "timings.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
    return best, rows


def panel(img_path, title, ms, w=960, h=640):
    im = Image.open(img_path).convert("RGB").resize((w, h), Image.LANCZOS)
    d = ImageDraw.Draw(im)
    f = font(22)
    tb = d.textbbox((0, 0), title, font=f)
    d.rectangle([6, 6, 6 + tb[2] - tb[0] + 12, 6 + tb[3] - tb[1] + 12], fill=(0, 0, 0))
    d.text((12, 10), title, fill=(255, 255, 255), font=f)
    if ms is not None:
        t2 = "GPU frame %.2f ms" % ms
        f2 = font(16)
        tb = d.textbbox((0, 0), t2, font=f2)
        d.rectangle([6, h - 34, 6 + tb[2] - tb[0] + 12, h - 6], fill=(0, 0, 0))
        d.text((12, h - 30), t2, fill=(255, 255, 255), font=f2)
    return im


def strip(run: Path, view, labels, rows, baseline: Path, out_name=None, cols=None, scale=1.0, names=None):
    names = dict(NAMES, **(names or {}))
    panels = []
    for lab in labels:
        if lab == "shipped":
            p = baseline / f"{view}__remastered.png" if baseline else None
            ms = baseline_ms(baseline, view) if baseline else None
        else:
            p = run / f"{view}__{lab}.png"
            ms = rows.get((view, lab), {}).get("frame")
        if p is None or not p.exists():
            continue
        panels.append(panel(p, names.get(lab, lab), ms))
    if not panels:
        return None
    n = len(panels)
    cols = cols or n
    rws = (n + cols - 1) // cols
    W, H = 960, 640
    gap = 6
    sheet = Image.new("RGB", (cols * W + (cols - 1) * gap, rws * H + (rws - 1) * gap + 40), (24, 24, 24))
    d = ImageDraw.Draw(sheet)
    d.text((10, 8), view, fill=(255, 255, 255), font=font(24))
    for i, pnl in enumerate(panels):
        r, c = divmod(i, cols)
        sheet.paste(pnl, (c * (W + gap), 40 + r * (H + gap)))
    if scale != 1.0:
        sheet = sheet.resize((int(sheet.width * scale), int(sheet.height * scale)), Image.LANCZOS)
    out = run / (out_name or f"strip_{view}.png")
    sheet.save(out)
    return out


_base_cache = {}


def baseline_ms(baseline: Path, view):
    if baseline not in _base_cache:
        rows = {}
        tj = baseline / "timings.csv"
        if tj.exists():
            import csv
            with tj.open(encoding="utf-8") as fh:
                for r in csv.DictReader(fh):
                    try:
                        rows[(r["view"], r["mode"])] = float(r.get("gpu_frame_ms") or "nan")
                    except ValueError:
                        pass
        _base_cache[baseline] = rows
    return _base_cache[baseline].get((view, "remastered"))


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--strip", default="classic,shipped,remastered,svm_occ,svm_full,dbg_vis,dbg_bounce")
    ap.add_argument("--baseline", default=r"D:\Arma_CWA_worktrees\lighting\tmp\captures\20261017-final")
    ap.add_argument("--cols", type=int, default=4)
    ap.add_argument("--scale", type=float, default=0.5)
    a = ap.parse_args()
    run = Path(a.run)
    best, rows = collect(run)
    views = sorted({v for v, _ in best})
    for v in views:
        o = strip(run, v, a.strip.split(","), rows, Path(a.baseline) if a.baseline else None, cols=a.cols, scale=a.scale)
        print("strip:", o)
    print("shots:", len(best), "timing rows:", len(rows))
