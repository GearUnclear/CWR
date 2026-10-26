"""Collect a night research run and build labelled comparison strips.

python night_compare.py <run dir> [--labels classic,remastered,v1] [--names "Classic,Remastered (current),Night v1"]
                        [--grid] [--crop x0,y0,x1,y1]

Copies NNN_<view>__<label>.png from <run dir>/tri into <run dir>/shots/,
parses the UDCAP shot lines (GPU frame ms, world, post, shafts) into
timings.md, and writes <run dir>/cmp_<view>.png: the chosen labels side by
side (or 2x2 with --grid), each panel labelled with its GPU frame time.
"""
import argparse, re, shutil, json
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

SHOT_RE = re.compile(r"^(\d{3})_(.+)__([A-Za-z0-9]+)\.png$")
UDCAP_RE = re.compile(r"UDCAP;(.*)$")
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")

def font(sz):
    for n in ("arialbd.ttf", "arial.ttf"):
        p = Path("C:/Windows/Fonts") / n
        if p.exists():
            return ImageFont.truetype(str(p), sz)
    return ImageFont.load_default()

def box(d, xy, text, f):
    b = d.textbbox(xy, text, font=f)
    d.rectangle([b[0] - 5, b[1] - 3, b[2] + 5, b[3] + 3], fill=(0, 0, 0))
    d.text(xy, text, font=f, fill=(255, 255, 255))

def collect(run):
    tri = run / "tri"
    shots = run / "shots"
    shots.mkdir(exist_ok=True)
    best = {}
    for p in tri.rglob("*.png"):
        m = SHOT_RE.match(p.name)
        if m:
            k = (m.group(2), m.group(3))
            if k not in best or p.stat().st_mtime > best[k].stat().st_mtime:
                best[k] = p
    for (v, l), p in best.items():
        shutil.copy2(p, shots / f"{v}__{l}.png")
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
            v, l = kv["label"].rsplit("__", 1)
            g = {}
            for key, rx in (("frame", r"frame ([\d.]+) ms"), ("world", r"world ([\d.]+)"), ("shadow", r"\(shadow ([\d.]+)\)"),
                            ("post", r"post ([\d.]+):"), ("ao", r"ao ([\d.]+)"), ("bloom", r"bloom ([\d.]+)"),
                            ("shafts", r"shafts ([\d.]+)"), ("composite", r"composite ([\d.]+)")):
                mm = re.search(rx, gpu)
                g[key] = float(mm.group(1)) if mm else None
            rows[(v, l)] = {"view": v, "label": l, "lit": kv.get("lit", ""), "gpu": gpu, **g}
    return best, rows

def strip(run, view, labels, names, rows, grid=False, crop=None, out_name=None):
    imgs = []
    for l in labels:
        p = run / "shots" / f"{view}__{l}.png"
        if not p.exists():
            return None
        im = Image.open(p).convert("RGB")
        if crop:
            im = im.crop(crop)
        imgs.append(im)
    w, h = imgs[0].size
    gap, title = 6, 34
    n = len(imgs)
    cols = 2 if grid and n == 4 else n
    rws = (n + cols - 1) // cols
    canvas = Image.new("RGB", (cols * w + (cols - 1) * gap, title + rws * h + (rws - 1) * gap), (20, 20, 20))
    d = ImageDraw.Draw(canvas)
    d.text((8, 6), view, font=font(20), fill=(235, 235, 235))
    for i, (im, l, nm) in enumerate(zip(imgs, labels, names)):
        r, c = divmod(i, cols)
        x0, y0 = c * (w + gap), title + r * (h + gap)
        canvas.paste(im, (x0, y0))
        box(d, (x0 + 10, y0 + 8), nm, font(18))
        row = rows.get((view, l))
        if row and row.get("frame") is not None:
            t = f"GPU frame {row['frame']:.2f} ms"
            if row.get("shafts") is not None:
                t += f" (haze {row['shafts']:.2f})"
            if row.get("shadow") is not None:
                t += f" (shadow {row['shadow']:.2f})"
            box(d, (x0 + 10, y0 + h - 28), t, font(14))
        if row and row.get("lit") not in ("OK", "", None):
            box(d, (x0 + 10, y0 + 36), "lit: " + row["lit"][:60], font(13))
    dst = run / (out_name or f"cmp_{view}.png")
    canvas.save(dst)
    return dst

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--labels", default="")
    ap.add_argument("--names", default="")
    ap.add_argument("--grid", action="store_true")
    ap.add_argument("--crop", default="")
    ap.add_argument("--views", default="")
    ap.add_argument("--suffix", default="")
    a = ap.parse_args()
    run = Path(a.run)
    best, rows = collect(run)
    views = []
    for (v, l) in sorted(best):
        if v not in views:
            views.append(v)
    if a.views:
        views = a.views.split(",")
    (run / "timings.json").write_text(json.dumps(list(rows.values()), indent=1), encoding="utf-8")
    md = ["| view | label | GPU frame ms | world | shadow | post | ao | bloom | haze(shafts slot) | composite | lit |", "|---|---|---|---|---|---|---|---|---|---|---|"]
    f = lambda x: "" if x is None else f"{x:.2f}"
    for r in rows.values():
        md.append(f"| {r['view']} | {r['label']} | {f(r['frame'])} | {f(r['world'])} | {f(r['shadow'])} | {f(r['post'])} | {f(r['ao'])} | {f(r['bloom'])} | {f(r['shafts'])} | {f(r['composite'])} | {r['lit']} |")
    (run / "timings.md").write_text("\n".join(md) + "\n", encoding="utf-8")
    crop = tuple(int(x) for x in a.crop.split(",")) if a.crop else None
    for v in views:
        labels = a.labels.split(",") if a.labels else sorted({l for (vv, l) in best if vv == v})
        names = a.names.split(",") if a.names else labels
        dst = strip(run, v, labels, names, rows, a.grid, crop, f"cmp_{v}{a.suffix}.png")
        print(v, "->", dst)

main()
