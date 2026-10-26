"""Collect a night-bright capture run: shots, labelled strips, frame statistics.

python nb_analyze.py <run dir> [--labels classic,remastered,...] [--names "A,B,..."] [--prefix cmp]
"""
import argparse, colorsys, re, shutil
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

SHOT_RE = re.compile(r"^(\d{3})_(.+)__([A-Za-z0-9]+)\.png$")


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
    shots = run / "shots"
    shots.mkdir(exist_ok=True)
    best = {}
    for p in (run / "tri").rglob("*.png"):
        m = SHOT_RE.match(p.name)
        if m:
            k = (m.group(2), m.group(3))
            if k not in best or p.stat().st_mtime > best[k].stat().st_mtime:
                best[k] = p
    for (v, l), p in best.items():
        shutil.copy2(p, shots / f"{v}__{l}.png")
    return sorted(best)


def to_lin(c):
    return (c / 255.0) ** 2.2


def stats(img, box_frac=None):
    im = img.convert("RGB")
    w, h = im.size
    if box_frac:
        x0, y0, x1, y1 = box_frac
        im = im.crop((int(x0 * w), int(y0 * h), int(x1 * w), int(y1 * h)))
    small = im.resize((max(1, im.size[0] // 4), max(1, im.size[1] // 4)), Image.BOX)
    px = list(small.getdata())
    n = len(px)
    r = sum(p[0] for p in px) / n
    g = sum(p[1] for p in px) / n
    b = sum(p[2] for p in px) / n
    import math
    lum_lin = [0.2126 * to_lin(p[0]) + 0.7152 * to_lin(p[1]) + 0.0722 * to_lin(p[2]) for p in px]
    logmean = sum(math.log2(max(l, 1e-5)) for l in lum_lin) / n
    hh, ss, vv = colorsys.rgb_to_hsv(r / 255, g / 255, b / 255)
    return dict(r=r, g=g, b=b, luma=0.2126 * r + 0.7152 * g + 0.0722 * b, log2=logmean, hue=hh * 360, sat=ss,
                rb=r / max(b, 1e-3))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--labels", default="")
    ap.add_argument("--names", default="")
    ap.add_argument("--prefix", default="cmp")
    ap.add_argument("--nocollect", action="store_true")
    a = ap.parse_args()
    run = Path(a.run)
    keys = collect(run) if not a.nocollect else []
    shots = run / "shots"
    views = sorted({p.name.split("__")[0] for p in shots.glob("*.png")})
    labels = a.labels.split(",") if a.labels else sorted({p.stem.split("__")[1] for p in shots.glob("*.png")})
    names = a.names.split(",") if a.names else labels
    lines = ["| view | label | luma | log2 lin | ground RGB | ground sat | ground hue | ground r/b | ground log2 |",
             "|---|---|---|---|---|---|---|---|---|"]
    f = font(18)
    for v in views:
        panels = []
        for l, nm in zip(labels, names):
            p = shots / f"{v}__{l}.png"
            if not p.exists():
                continue
            im = Image.open(p).convert("RGB")
            s = stats(im)
            gs = stats(im, (0.0, 0.72, 1.0, 1.0))
            lines.append("| %s | %s | %.1f | %.2f | %.0f %.0f %.0f | %.3f | %.0f | %.2f | %.2f |" % (
                v, l, s["luma"], s["log2"], gs["r"], gs["g"], gs["b"], gs["sat"], gs["hue"], gs["rb"], gs["log2"]))
            d = ImageDraw.Draw(im)
            box(d, (10, 10), nm, f)
            box(d, (10, im.size[1] - 30), "frame luma %.1f, ground sat %.2f" % (s["luma"], gs["sat"]), font(14))
            panels.append(im)
        if not panels:
            continue
        w, h = panels[0].size
        strip = Image.new("RGB", (w * len(panels) + 6 * (len(panels) - 1), h + 34), (24, 24, 24))
        d = ImageDraw.Draw(strip)
        d.text((10, 6), v, font=font(20), fill=(255, 255, 255))
        for i, im in enumerate(panels):
            strip.paste(im, (i * (w + 6), 34))
        strip.save(run / f"{a.prefix}_{v}.png")
    (run / f"{a.prefix}_stats.md").write_text("\n".join(lines) + "\n")
    print("\n".join(lines))


if __name__ == "__main__":
    main()
