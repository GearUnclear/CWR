"""Three-way and overview comparisons for the water research runs.

    python compare.py --stock runs/stock --variant A=runs/A --variant B=runs/B ... --out <dir>

Writes into <out>:
    triptych_<view>__<id>.png   Classic | Remastered now | Remastered + variant <id>, labelled
    overview_<view>.png         Classic | now | every variant, half size, one row
    metrics.md / metrics.json   water-region luminance mean and local contrast per image
Classic is taken from the stock run (its classic shot; the variants override
only the remastered detail shader, so their classic shots must match it).
"""
import argparse
import json
import math
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont, ImageStat

VIEWS = ["coastline_sunset", "sea_noon", "sea_away_from_sun", "coast_shore_midsun"]
# water region per view in the 960x640 captures: (left, top, right, bottom)
WATER = {
    "coastline_sunset": (0, 330, 960, 490),
    "sea_noon": (0, 210, 960, 630),
    "sea_away_from_sun": (0, 285, 960, 440),
    "coast_shore_midsun": (0, 95, 960, 600),
}


def font(size):
    for name in ("arialbd.ttf", "arial.ttf", "segoeui.ttf"):
        p = Path("C:/Windows/Fonts") / name
        if p.exists():
            return ImageFont.truetype(str(p), size)
    return ImageFont.load_default()


def label(draw, xy, text, fnt):
    box = draw.textbbox(xy, text, font=fnt)
    draw.rectangle([box[0] - 6, box[1] - 4, box[2] + 6, box[3] + 4], fill=(0, 0, 0))
    draw.text(xy, text, font=fnt, fill=(255, 255, 255))


def gpu_ms(run: Path, view: str, mode: str):
    p = run / "timings.json"
    if not p.exists():
        return None
    for r in json.loads(p.read_text(encoding="utf-8")):
        if r["view"] == view and r["mode"] == mode:
            return r.get("gpu_frame_ms"), r.get("gpu_world_ms")
    return None


def metrics(img: Image.Image, box):
    g = img.convert("L").crop(box)
    mean = ImageStat.Stat(g).mean[0]
    blur = g.filter(ImageFilter.BoxBlur(4))
    hp = ImageChops.subtract(g, blur, 1.0, 128)
    sd = ImageStat.Stat(hp).stddev[0]
    # coarse contrast: std of the 9x9-box blurred image (wave-scale structure, not pixel noise)
    coarse = ImageStat.Stat(g.filter(ImageFilter.BoxBlur(2))).stddev[0]
    return {"mean": round(mean, 1), "local_sd": round(sd, 2), "sd": round(coarse, 2)}


def strip(panels, title, w, h):
    gap, th = 8, 40
    out = Image.new("RGB", (len(panels) * w + (len(panels) - 1) * gap, h + th), (24, 24, 24))
    d = ImageDraw.Draw(out)
    d.text((10, 8), title, font=font(22), fill=(235, 235, 235))
    for i, (img, name, note) in enumerate(panels):
        x = i * (w + gap)
        if img is None:
            d.rectangle([x, th, x + w, th + h], fill=(60, 0, 0))
        else:
            out.paste(img.resize((w, h), Image.LANCZOS) if img.size != (w, h) else img, (x, th))
        f = font(22 if w >= 900 else 15)
        label(d, (x + 10, th + 8), name, f)
        if note:
            label(d, (x + 10, th + h - (30 if w >= 900 else 22)), note, font(15 if w >= 900 else 12))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--stock", required=True)
    ap.add_argument("--variant", action="append", default=[], help="ID=run_dir")
    ap.add_argument("--out", required=True)
    a = ap.parse_args()
    stock = Path(a.stock)
    variants = [(s.split("=", 1)[0], Path(s.split("=", 1)[1])) for s in a.variant]
    out = Path(a.out)
    out.mkdir(parents=True, exist_ok=True)
    allm = {}
    md = ["# Water-region metrics", "",
          "Luminance (0-255) over the water region of each 960x640 capture: mean, sd of a lightly blurred",
          "image (wave-scale contrast), and sd of a 9x9 high-pass (fine detail). Higher sd = more defined.", "",
          "| view | image | mean | sd (wave scale) | sd (fine) | GPU frame ms | GPU world ms |", "|---|---|---|---|---|---|---|"]
    for view in VIEWS:
        def load(run, mode):
            p = run / f"{view}__{mode}.png"
            return Image.open(p).convert("RGB") if p.exists() else None
        classic = load(stock, "classic")
        now = load(stock, "remastered")
        rows = [("Classic", classic, gpu_ms(stock, view, "classic")),
                ("Remastered now", now, gpu_ms(stock, view, "remastered"))]
        for vid, run in variants:
            rows.append((f"Variant {vid}", load(run, "remastered"), gpu_ms(run, view, "remastered")))
            # check the variant run's classic shot against the stock classic
            vc = load(run, "classic")
            if vc is not None and classic is not None:
                diff = ImageChops.difference(vc, classic).convert("L")
                allm.setdefault("classic_check", {})[f"{view}/{vid}"] = round(ImageStat.Stat(diff).mean[0], 3)
        for name, img, g in rows:
            if img is None:
                continue
            m = metrics(img, WATER[view])
            allm.setdefault(view, {})[name] = dict(m, gpu=g)
            gs = ("", "") if not g else (f"{g[0]:.2f}" if g[0] is not None else "", f"{g[1]:.2f}" if g[1] is not None else "")
            md.append(f"| {view} | {name} | {m['mean']} | {m['sd']} | {m['local_sd']} | {gs[0]} | {gs[1]} |")
        def note(g):
            return f"GPU frame {g[0]:.2f} ms" if g and g[0] is not None else ""
        for vid, run in variants:
            v_img = load(run, "remastered")
            g = gpu_ms(run, view, "remastered")
            t = strip([(classic, "Classic", note(rows[0][2])), (now, "Remastered now", note(rows[1][2])),
                       (v_img, f"Remastered + water {vid}", note(g))], f"{view}: Classic | Remastered now | prototype {vid}", 960, 640)
            t.save(out / f"triptych_{view}__{vid}.png")
        ov = strip([(img, name, "") for name, img, _g in rows], f"{view}", 480, 320)
        ov.save(out / f"overview_{view}.png")
    (out / "metrics.json").write_text(json.dumps(allm, indent=2), encoding="utf-8")
    if "classic_check" in allm:
        md += ["", "Classic shot of each variant run vs the stock run (mean abs difference, 0-255):", ""]
        md += [f"- {k}: {v}" for k, v in allm["classic_check"].items()]
    (out / "metrics.md").write_text("\n".join(md) + "\n", encoding="utf-8")
    print("\n".join(md))


if __name__ == "__main__":
    main()
