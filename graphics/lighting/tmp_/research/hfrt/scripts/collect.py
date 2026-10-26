"""Collect an HFRT research capture run: copy shots out of the tri tree, parse
the UDCAP shot lines (GPU timings), build labelled comparison sheets.

usage: python collect.py <run_dir> [--best hfB] [--crop x0,y0,x1,y1]
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
POST_RE = re.compile(r"\bpost ([\d.]+):")

NICE = {
    "classic": "Classic",
    "remastered": "Remastered (current, horizon map)",
    "remastered2": "Remastered (current, re-timed)",
    "hfA": "HFRT A: traced sun + sky visibility",
    "hfB": "HFRT B: A + one-bounce terrain GI",
    "hfC": "HFRT C: B with bounce x3",
    "hfLite": "HFRT lite: B at 1/2 budget",
    "hfSharp": "HFRT sharp: B, 0.4 deg penumbra",
    "dbgsun_rem": "sun visibility: horizon map",
    "dbgsun_hf": "sun visibility: heightfield traced",
    "dbgsky_rem": "sky visibility: horizon map (x0.6)",
    "dbgsky_hf": "sky visibility: traced, normal-aware",
    "dbgbounce_hf": "terrain bounce irradiance (x4)",
    "hfCan": "HFRT: terrain + canopy traced, sky vis, bounce",
    "hfCan2": "HFRT canopy (re-timed)",
    "hfArt": "HFRT canopy, bounce x3",
    "hf4": "HFRT: terrain + canopy traced",
    "dbgsun_hf4": "sun visibility: terrain + canopy traced",
    "dbgsun_can": "sun visibility: terrain + canopy traced",
    "dbgsky_can": "sky visibility: terrain + canopy traced",
    "dbgbounce_can": "terrain bounce irradiance (x4)",
}


def font(size):
    for name in ("arialbd.ttf", "arial.ttf", "DejaVuSans-Bold.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            pass
    return ImageFont.load_default()


def label_box(d, xy, text, fnt):
    x, y = xy
    l, t, r, b = d.textbbox((x, y), text, font=fnt)
    d.rectangle((l - 5, t - 4, r + 5, b + 4), fill=(0, 0, 0))
    d.text((x, y), text, font=fnt, fill=(255, 255, 255))


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
    shots = {}
    for (view, label), src in best.items():
        dst = run / f"{view}__{label}.png"
        shutil.copy2(src, dst)
        shots.setdefault(view, {})[label] = dst
    return shots


def timings(run: Path):
    rows = {}
    for p in sorted((run / "tri").rglob("*.log")):
        for line in p.read_text(encoding="utf-8", errors="replace").splitlines():
            line = ANSI_RE.sub("", line)
            m = UDCAP_RE.search(line)
            if not m or "event=shot" not in m.group(1):
                continue
            body = m.group(1)
            gpu = ""
            if ";gpu=" in body:
                body, gpu = body.split(";gpu=", 1)
            kv = dict(x.split("=", 1) for x in body.split(";") if "=" in x)
            label = kv.get("label", "")
            if "__" not in label:
                continue
            view, mode = label.split("__", 1)
            fr = FRAME_RE.search(gpu)
            wo = WORLD_RE.search(gpu)
            po = POST_RE.search(gpu)
            rows[(view, mode)] = dict(frame=float(fr.group(1)) if fr else None,
                                      world=float(wo.group(1)) if wo else None,
                                      post=float(po.group(1)) if po else None, gpu=gpu.strip())
    return rows


def panel(path, title, row, scale=1.0):
    im = Image.open(path).convert("RGB")
    if scale != 1.0:
        im = im.resize((int(im.width * scale), int(im.height * scale)), Image.LANCZOS)
    d = ImageDraw.Draw(im)
    big, small = font(int(22 * max(scale, 0.6))), font(int(16 * max(scale, 0.6)))
    label_box(d, (10, 8), title, big)
    if row and row.get("frame") is not None and not title.startswith(("sun vis", "sky vis", "terrain bounce")):
        label_box(d, (10, im.height - 28), f"GPU frame {row['frame']:.2f} ms  (world {row['world']:.2f})", small)
    return im


def sheet(images, cols, title, dst, gap=6):
    w, h = images[0].size
    rows = (len(images) + cols - 1) // cols
    th = 40
    out = Image.new("RGB", (cols * w + (cols - 1) * gap, th + rows * h + (rows - 1) * gap), (24, 24, 24))
    d = ImageDraw.Draw(out)
    d.text((10, 8), title, font=font(22), fill=(235, 235, 235))
    for i, im in enumerate(images):
        out.paste(im, ((i % cols) * (w + gap), th + (i // cols) * (h + gap)))
    out.save(dst)
    return dst


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--best", default="hfB")
    args = ap.parse_args()
    run = Path(args.run)
    shots = collect(run)
    tim = timings(run)
    md = ["| view | mode | GPU frame ms | world ms | post ms |", "|---|---|---|---|---|"]
    for (view, mode), r in sorted(tim.items()):
        if mode.startswith("dbg"):
            continue
        f = lambda v: f"{v:.2f}" if v is not None else ""
        md.append(f"| {view} | {mode} | {f(r['frame'])} | {f(r['world'])} | {f(r['post'])} |")
    (run / "timings_hfrt.md").write_text("\n".join(md) + "\n", encoding="utf-8")
    for view, labels in sorted(shots.items()):
        get = lambda m: tim.get((view, m))
        three = [m for m in ("classic", "remastered", args.best) if m in labels]
        if len(three) == 3:
            sheet([panel(labels[m], NICE.get(m, m), get(m)) for m in three], 3,
                  f"{view}: Classic | Remastered (current) | Remastered + {NICE.get(args.best, args.best)}",
                  run / f"{view}__3way.png")
        var = [m for m in ("remastered", "hfA", "hfB", "hfC", "hfCan", "hfLite", "hfSharp", "hfArt") if m in labels]
        if var:
            sheet([panel(labels[m], NICE.get(m, m), get(m), 0.75) for m in var], 3, f"{view}: HFRT variants",
                  run / f"{view}__variants.png")
        dbg = [m for m in ("dbgsun_rem", "dbgsun_hf", "dbgsun_hf4", "dbgsun_can", "dbgsky_rem", "dbgsky_hf", "dbgsky_can", "dbgbounce_hf", "dbgbounce_can", args.best) if m in labels]
        if dbg:
            sheet([panel(labels[m], NICE.get(m, m), get(m), 0.75) for m in dbg], 2, f"{view}: debug views",
                  run / f"{view}__debug.png")
    print("views:", {v: sorted(l) for v, l in shots.items()})
    print("\n".join(md))


if __name__ == "__main__":
    main()
