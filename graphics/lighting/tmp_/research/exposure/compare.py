"""Collect an exposure research run and build labelled comparison sheets.

    python compare.py <run dir> [--tri-dir DIR]
    python compare.py <run dir> --three VIEW VARIANT OUT.png [--title TEXT]

Collect mode copies every NNN_<view>__<label>.png out of tri's tree to
<run dir>/<view>__<label>.png, parses the UDCAP shot lines into
<run dir>/timings.csv (+ timings.md), and writes <run dir>/<view>__sheet.png:
every shot of the view in run order, labelled, with GPU frame / post /
composite ms on each panel.

--three writes a Classic | Remastered | <variant> strip for one view.
"""
import argparse
import csv
import re
import shutil
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

SHOT_RE = re.compile(r"^(\d{3})_(.+?)__(.+)\.png$")
UDCAP_RE = re.compile(r"UDCAP;(.*)$")
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")
GPU_RES = {
    "frame": re.compile(r"\bframe ([\d.]+) ms"),
    "world": re.compile(r"\bworld ([\d.]+)"),
    "post": re.compile(r"\bpost ([\d.]+):"),
    "composite": re.compile(r"\bcomposite ([\d.]+)"),
}

NAMES = {
    "classic": "Classic",
    "remastered": "Remastered (current)",
}


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


def collect(run: Path, tri: Path):
    best = {}
    order = {}
    for p in tri.rglob("*.png"):
        m = SHOT_RE.match(p.name)
        if not m:
            continue
        key = (m.group(2), m.group(3))
        if key not in best or p.stat().st_mtime > best[key].stat().st_mtime:
            best[key] = p
            order[key] = int(m.group(1))
    for (view, label), src in best.items():
        shutil.copy2(src, run / f"{view}__{label}.png")
    rows = {}
    for p in sorted(tri.rglob("*.log")):
        for raw in p.read_text(encoding="utf-8", errors="replace").splitlines():
            m = UDCAP_RE.search(ANSI_RE.sub("", raw))
            if not m:
                continue
            kv = parse_kv(m.group(1).strip().rstrip('"'))
            if kv.get("event") != "shot" or "__" not in kv.get("label", ""):
                continue
            view, label = kv["label"].split("__", 1)
            row = {"view": view, "label": label, "lit": kv.get("lit", ""), "gpu_raw": kv.get("gpu", "")}
            for f, rx in GPU_RES.items():
                mm = rx.search(row["gpu_raw"])
                row[f] = float(mm.group(1)) if mm else None
            rows[(view, label)] = row
    with open(run / "timings.csv", "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=["view", "label", "frame", "world", "post", "composite", "lit", "gpu_raw"])
        w.writeheader()
        for k in sorted(rows, key=lambda k: order.get(k, 999)):
            w.writerow(rows[k])
    md = ["| view | shot | GPU frame ms | world | post | composite (incl. local exposure) | triSetLighting |",
          "|---|---|---|---|---|---|---|"]
    fm = lambda v: "" if v is None else f"{v:.2f}"
    for k in sorted(rows, key=lambda k: order.get(k, 999)):
        r = rows[k]
        md.append(f"| {r['view']} | {r['label']} | {fm(r['frame'])} | {fm(r['world'])} | {fm(r['post'])} | "
                  f"{fm(r['composite'])} | {r['lit'][:60]} |")
    (run / "timings.md").write_text("\n".join(md) + "\n", encoding="utf-8")
    return best, order, rows


def panel(img_path: Path, title: str, row, w=960, h=640):
    im = Image.open(img_path).convert("RGB")
    if im.size != (w, h):
        im = im.resize((w, h), Image.LANCZOS)
    d = ImageDraw.Draw(im)
    label_box(d, (12, 10), title, font(22))
    if row and row.get("frame") is not None:
        txt = f"GPU frame {row['frame']:.2f} ms"
        if row.get("post") is not None:
            txt += f" | post {row['post']:.2f}"
        if row.get("composite") is not None:
            txt += f" | composite {row['composite']:.2f}"
        label_box(d, (12, h - 30), txt, font(15))
    if row and row.get("lit") and row["lit"] != "OK":
        label_box(d, (12, 46), "knobs: " + row["lit"][:80], font(14))
    return im


def sheet(run: Path, view: str, labels, rows, names, cols=3, out=None, title=None):
    panels = []
    for lab in labels:
        p = run / f"{view}__{lab}.png"
        if p.exists():
            panels.append(panel(p, names.get(lab, lab), rows.get((view, lab))))
    if not panels:
        return None
    pw, ph = panels[0].size
    gap = 8
    head = 40
    cols = min(cols, len(panels))
    nrows = (len(panels) + cols - 1) // cols
    sh = Image.new("RGB", (cols * pw + (cols - 1) * gap, head + nrows * ph + (nrows - 1) * gap), (20, 20, 20))
    d = ImageDraw.Draw(sh)
    d.text((10, 8), title or view, font=font(22), fill=(235, 235, 235))
    for i, p in enumerate(panels):
        r, c = divmod(i, cols)
        sh.paste(p, (c * (pw + gap), head + r * (ph + gap)))
    dst = out or (run / f"{view}__sheet.png")
    sh.save(dst)
    return dst


def main(argv):
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--tri-dir")
    ap.add_argument("--three", nargs=3, metavar=("VIEW", "VARIANT", "OUT"))
    ap.add_argument("--title")
    ap.add_argument("--name", action="append", default=[], help="label=Display name")
    ap.add_argument("--cols", type=int, default=3)
    a = ap.parse_args(argv)
    run = Path(a.run)
    tri = Path(a.tri_dir) if a.tri_dir else run / "tri"
    names = dict(NAMES)
    for n in a.name:
        k, v = n.split("=", 1)
        names[k] = v
    best, order, rows = collect(run, tri)
    if a.three:
        view, var, out = a.three
        dst = sheet(run, view, ["classic", "remastered", var], rows, names, cols=3, out=Path(out), title=a.title)
        print(f"wrote {dst}")
        return 0
    views = []
    for (v, lab) in sorted(best, key=lambda k: order[k]):
        if v not in views:
            views.append(v)
    for v in views:
        labs = [lab for (vv, lab) in sorted(best, key=lambda k: order[k]) if vv == v]
        dst = sheet(run, v, labs, rows, names, cols=a.cols)
        print(f"{v}: {len(labs)} shots -> {dst}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
