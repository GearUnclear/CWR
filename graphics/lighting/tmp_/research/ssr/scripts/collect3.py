"""Collect an SSR research run: copy every NNN_<label>.png out of the tri tree,
read the UDCAP shot lines, and build labelled three-way comparisons
Classic | Remastered | Remastered + <variant> with GPU frame ms per panel.

    python collect3.py <run_dir> [--variants ssr,ssrwet] [--best view:variant ...]
"""
import argparse, re, shutil, json
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

SHOT = re.compile(r"^(\d{3})_(.+)\.png$")
UDCAP = re.compile(r"UDCAP;(.*)$")
ANSI = re.compile(r"\x1b\[[0-9;]*m")

def font(sz):
    for n in ("arialbd.ttf", "arial.ttf"):
        try: return ImageFont.truetype("C:/Windows/Fonts/" + n, sz)
        except OSError: pass
    return ImageFont.load_default()

def parse(body):
    gpu = None
    if ";gpu=" in body: body, gpu = body.split(";gpu=", 1)
    kv = {}
    for part in body.split(";"):
        if "=" in part:
            k, v = part.split("=", 1); kv[k] = v
    kv["gpu"] = (gpu or "").strip().rstrip('"')
    return kv

def num(rx, s):
    m = re.search(rx, s or "")
    return float(m.group(1)) if m else None

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--variants", default="ssr,ssrwet")
    a = ap.parse_args()
    run = Path(a.run); tri = run / "tri"; shots = run / "shots"; shots.mkdir(exist_ok=True)
    best = {}
    for p in tri.rglob("*.png"):
        m = SHOT.match(p.name)
        if not m: continue
        lab = m.group(2)
        if lab not in best or p.stat().st_mtime > best[lab].stat().st_mtime: best[lab] = p
    for lab, p in best.items(): shutil.copy2(p, shots / f"{lab}.png")
    rows = {}
    for p in sorted(tri.rglob("*.log")):
        for raw in p.read_text(encoding="utf-8", errors="replace").splitlines():
            m = UDCAP.search(ANSI.sub("", raw))
            if not m: continue
            kv = parse(m.group(1).strip().rstrip('"'))
            if kv.get("event") == "shot":
                g = kv["gpu"]
                rows[kv["label"]] = {"label": kv["label"], "gpu_raw": g,
                    "frame": num(r"\bframe ([\d.]+) ms", g), "world": num(r"\bworld ([\d.]+)", g),
                    "post": num(r"\bpost ([\d.]+):", g), "ao": num(r"\bao ([\d.]+)", g),
                    "bloom": num(r"\bbloom ([\d.]+)", g), "composite": num(r"\bcomposite ([\d.]+)", g)}
    (run / "shots_timings.json").write_text(json.dumps(rows, indent=1))
    md = ["| shot | GPU frame ms | world | post | ao (+SSR when on) | bloom | composite |", "|---|---|---|---|---|---|---|"]
    f = lambda v: "" if v is None else f"{v:.2f}"
    for lab, r in rows.items():
        md.append(f"| {lab} | {f(r['frame'])} | {f(r['world'])} | {f(r['post'])} | {f(r['ao'])} | {f(r['bloom'])} | {f(r['composite'])} |")
    (run / "shots_timings.md").write_text("\n".join(md) + "\n")
    views = sorted({l.rsplit("__", 1)[0] for l in best if "__" in l})
    big, small = font(26), font(18)
    out = []
    for v in views:
        for var in a.variants.split(","):
            labs = [f"{v}__classic", f"{v}__remastered", f"{v}__{var}"]
            if not all((shots / f"{l}.png").exists() for l in labs): continue
            ims = [Image.open(shots / f"{l}.png").convert("RGB") for l in labs]
            w, h = ims[0].size; title = 40
            canvas = Image.new("RGB", (w * 3 + 16, h + title), (24, 24, 24))
            d = ImageDraw.Draw(canvas)
            d.text((10, 6), f"{v}  ({var})", fill=(235, 235, 235), font=big)
            names = ["Classic", "Remastered (current)", f"Remastered + SSR ({var})"]
            for i, (im, l, n) in enumerate(zip(ims, labs, names)):
                x = i * (w + 8); canvas.paste(im, (x, title))
                bb = d.textbbox((0, 0), n, font=big); d.rectangle((x + 6, title + 6, x + 14 + bb[2], title + 12 + bb[3]), fill=(0, 0, 0)); d.text((x + 10, title + 6), n, fill=(255, 255, 255), font=big)
                r = rows.get(l)
                if r and r["frame"] is not None:
                    t = f"GPU frame {r['frame']:.2f} ms"
                    if r.get("ao") is not None and i > 0: t += f" | ao+ssr {r['ao']:.2f}"
                    bb = d.textbbox((0, 0), t, font=small); d.rectangle((x + 6, title + h - 34, x + 14 + bb[2], title + h - 26 + bb[3]), fill=(0, 0, 0)); d.text((x + 10, title + h - 32), t, fill=(255, 255, 255), font=small)
            p = run / f"cmp_{v}__{var}.png"; canvas.save(p); out.append(p)
    print("shots", len(best), "rows", len(rows), "comparisons", len(out))
    for p in out: print(p)

main()
