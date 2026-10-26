"""Collect a combat research run: copy shots out of the tri tree by label, parse
the UDCAP shot lines (GPU frame ms per label), and build labelled three-way
comparisons (Classic | Remastered | Remastered + combat lights).

    python collect.py <run_dir> [--shots-dir <dir>]
    python collect.py <run_dir> --three <view>_{m}_<suffix> [--out NAME.png] [--title T]
    python collect.py <run_dir> --strip <label1,label2,...> --out NAME.png --title T
"""
import argparse
import json
import re
import shutil
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

SHOT_RE = re.compile(r"^(\d{3})_(.+)\.png$")
UDCAP_RE = re.compile(r"UDCAP;(.*)$")
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")
GPU_FRAME = re.compile(r"\bframe ([\d.]+) ms")
GPU_WORLD = re.compile(r"\bworld ([\d.]+)")
GPU_POST = re.compile(r"\bpost ([\d.]+):")
GPU_COMP = re.compile(r"\bcomposite ([\d.]+)")
MODE_NAMES = {"c": "Classic", "r": "Remastered (current)", "x": "Remastered + combat lights"}


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
        out["gpu"] = gpu.strip()
    return out


def collect(run, tri):
    shots = run / "shots"
    shots.mkdir(exist_ok=True)
    best = {}
    for p in tri.rglob("*.png"):
        m = SHOT_RE.match(p.name)
        if not m:
            continue
        label = m.group(2)
        if label not in best or p.stat().st_mtime > best[label].stat().st_mtime:
            best[label] = p
    for label, src in best.items():
        shutil.copy2(src, shots / f"{label}.png")
    timings = {}
    lines = []
    for log in tri.rglob("game_stdout.log"):
        for raw in log.read_text(encoding="utf-8", errors="replace").splitlines():
            m = UDCAP_RE.search(ANSI_RE.sub("", raw))
            if not m:
                continue
            lines.append(m.group(1))
            kv = parse_kv(m.group(1))
            if kv.get("event") == "shot":
                g = kv.get("gpu", "")
                rec = {"gpu": g, "mode": kv.get("mode"), "lit": kv.get("lit"), "fps": kv.get("fps")}
                for key, rx in (("frame", GPU_FRAME), ("world", GPU_WORLD), ("post", GPU_POST), ("composite", GPU_COMP)):
                    mm = rx.search(g)
                    rec[key] = float(mm.group(1)) if mm else None
                timings[kv.get("label")] = rec
    (run / "udcap.log").write_text("\n".join(lines) + "\n", encoding="utf-8")
    (run / "timings.json").write_text(json.dumps(timings, indent=1), encoding="utf-8")
    print(f"{len(best)} shots, {len(timings)} timing rows")
    return timings


def load_timings(run):
    p = run / "timings.json"
    return json.loads(p.read_text(encoding="utf-8")) if p.exists() else {}


def panel(img_path, title, gpu_txt, scale=0.5):
    im = Image.open(img_path).convert("RGB")
    if scale != 1.0:
        im = im.resize((int(im.width * scale), int(im.height * scale)), Image.LANCZOS)
    d = ImageDraw.Draw(im)
    f = font(15)
    tw = d.textlength(title, font=f)
    d.rectangle([6, 6, 14 + tw, 28], fill=(0, 0, 0))
    d.text((10, 8), title, fill=(255, 255, 255), font=f)
    if gpu_txt:
        f2 = font(13)
        tw = d.textlength(gpu_txt, font=f2)
        d.rectangle([6, im.height - 26, 14 + tw, im.height - 6], fill=(0, 0, 0))
        d.text((10, im.height - 24), gpu_txt, fill=(255, 255, 255), font=f2)
    return im


def gpu_text(t):
    if not t or t.get("frame") is None:
        return "GPU frame n/a"
    s = f"GPU frame {t['frame']:.2f} ms"
    if t.get("composite") is not None:
        s += f" (composite {t['composite']:.2f})"
    return s


def sheet(panels, title, cols, out):
    w, h = panels[0].size
    rows = (len(panels) + cols - 1) // cols
    head = 34
    canvas = Image.new("RGB", (cols * w + (cols - 1) * 6, head + rows * h + (rows - 1) * 6), (24, 24, 24))
    d = ImageDraw.Draw(canvas)
    d.text((10, 7), title, fill=(255, 255, 255), font=font(18))
    for i, p in enumerate(panels):
        r, c = divmod(i, cols)
        canvas.paste(p, (c * (w + 6), head + r * (h + 6)))
    canvas.save(out)
    print("wrote", out)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--tri", default=None)
    ap.add_argument("--three", default=None, help="label pattern with {m}")
    ap.add_argument("--strip", default=None, help="comma list of labels")
    ap.add_argument("--titles", default=None, help="comma list of panel titles for --strip")
    ap.add_argument("--out", default=None)
    ap.add_argument("--title", default="")
    ap.add_argument("--scale", type=float, default=0.5)
    ap.add_argument("--cols", type=int, default=3)
    ap.add_argument("--timing-from", default=None, help="label pattern with {m} whose timings to print")
    a = ap.parse_args()
    run = Path(a.run)
    if not a.three and not a.strip:
        collect(run, Path(a.tri) if a.tri else run / "tri")
        return
    t = load_timings(run)
    shots = run / "shots"
    if a.three:
        panels = []
        for m in ("c", "r", "x"):
            label = a.three.format(m=m)
            tl = t.get(a.timing_from.format(m=m)) if a.timing_from else t.get(label)
            panels.append(panel(shots / f"{label}.png", MODE_NAMES[m], gpu_text(tl), a.scale))
        out = a.out or (a.three.replace("_{m}", "") + "_3way.png")
        sheet(panels, a.title or a.three.replace("{m}", "*"), 3, run / out)
    else:
        labels = a.strip.split(",")
        titles = a.titles.split(",") if a.titles else labels
        panels = [panel(shots / f"{l}.png", ti, gpu_text(t.get(l)), a.scale) for l, ti in zip(labels, titles)]
        sheet(panels, a.title, a.cols, run / a.out)


if __name__ == "__main__":
    main()
