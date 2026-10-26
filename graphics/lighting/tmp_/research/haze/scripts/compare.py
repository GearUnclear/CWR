"""Labelled comparison sheets for the haze research runs.

python compare.py <run_dir> <out_png> <view> <label>=<title> [<label>=<title> ...] [--cols N] [--crop x0,y0,x1,y1] [--scale s]

<run_dir> holds tri/ (the tri output tree with NNN_<view>__<label>.png and
game_stdout.log). Each panel gets its title and the GPU frame / world ms
parsed from that shot's UDCAP line. Runs may be mixed: a label of the form
<other_run_dir>::<label> takes that shot from another run.
"""
import argparse
import os
import re
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

SHOT_RE = re.compile(r"^(\d{3})_(.+?)__(.+)\.png$")
UDCAP_RE = re.compile(r"UDCAP;(.*)$")
FRAME_RE = re.compile(r"\bframe ([\d.]+) ms")
WORLD_RE = re.compile(r"\bworld ([\d.]+)")


def font(size):
    for f in ("arialbd.ttf", "arial.ttf", "DejaVuSans-Bold.ttf"):
        try:
            return ImageFont.truetype(f, size)
        except OSError:
            pass
    return ImageFont.load_default()


def collect(run_dir: Path):
    shots, times = {}, {}
    for p in (run_dir / "tri").rglob("*.png"):
        m = SHOT_RE.match(p.name)
        if not m:
            continue
        key = (m.group(2), m.group(3))
        if key not in shots or p.stat().st_mtime > shots[key].stat().st_mtime:
            shots[key] = p
    for log in (run_dir / "tri").rglob("game_stdout.log"):
        for line in log.read_text(encoding="utf-8", errors="replace").splitlines():
            m = UDCAP_RE.search(line)
            if not m or "event=shot" not in m.group(1):
                continue
            body = m.group(1)
            lab = re.search(r"label=([^;]+)", body)
            if not lab or "__" not in lab.group(1):
                continue
            view, label = lab.group(1).split("__", 1)
            gpu = body.split(";gpu=", 1)[1] if ";gpu=" in body else ""
            fr, wr = FRAME_RE.search(gpu), WORLD_RE.search(gpu)
            times[(view, label)] = (float(fr.group(1)) if fr else None, float(wr.group(1)) if wr else None)
    return shots, times


def panel(img, title, t, scale):
    if scale != 1.0:
        img = img.resize((int(img.width * scale), int(img.height * scale)), Image.LANCZOS)
    d = ImageDraw.Draw(img)
    f1, f2 = font(max(14, int(22 * scale))), font(max(12, int(17 * scale)))
    pad = 6
    tb = d.textbbox((0, 0), title, font=f1)
    d.rectangle([8, 8, 8 + tb[2] + 2 * pad, 8 + tb[3] + 2 * pad], fill=(0, 0, 0))
    d.text((8 + pad, 8 + pad - tb[1] // 2), title, fill=(255, 255, 255), font=f1)
    if t and t[0] is not None:
        s = f"GPU frame {t[0]:.2f} ms" + (f" (world {t[1]:.2f})" if t[1] is not None else "")
        sb = d.textbbox((0, 0), s, font=f2)
        y = img.height - sb[3] - 2 * pad - 8
        d.rectangle([8, y, 8 + sb[2] + 2 * pad, y + sb[3] + 2 * pad], fill=(0, 0, 0))
        d.text((8 + pad, y + pad - sb[1] // 2), s, fill=(255, 255, 255), font=f2)
    return img


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run_dir")
    ap.add_argument("out")
    ap.add_argument("view")
    ap.add_argument("items", nargs="+")
    ap.add_argument("--cols", type=int, default=0)
    ap.add_argument("--crop", default="")
    ap.add_argument("--scale", type=float, default=1.0)
    ap.add_argument("--header", default="")
    a = ap.parse_args()
    cache = {}

    def get(run, view, label):
        run = Path(run)
        if run not in cache:
            cache[run] = collect(run)
        shots, times = cache[run]
        return shots.get((view, label)), times.get((view, label))

    panels = []
    for it in a.items:
        key, title = it.split("=", 1)
        run, label = (key.split("::", 1) if "::" in key else (a.run_dir, key))
        p, t = get(run, a.view, label)
        if p is None:
            raise SystemExit(f"missing shot {a.view}__{label} in {run}")
        img = Image.open(p).convert("RGB")
        if a.crop:
            x0, y0, x1, y1 = (int(v) for v in a.crop.split(","))
            img = img.crop((x0, y0, x1, y1))
        panels.append(panel(img, title, t, a.scale))
    cols = a.cols or len(panels)
    rows = (len(panels) + cols - 1) // cols
    W, H = panels[0].size
    gap = 6
    head = 40 if a.header else 0
    sheet = Image.new("RGB", (cols * W + (cols - 1) * gap, head + rows * H + (rows - 1) * gap), (24, 24, 24))
    if a.header:
        d = ImageDraw.Draw(sheet)
        d.text((10, 8), a.header, fill=(235, 235, 235), font=font(22))
    for i, im in enumerate(panels):
        r, c = divmod(i, cols)
        sheet.paste(im, (c * (W + gap), head + r * (H + gap)))
    os.makedirs(os.path.dirname(os.path.abspath(a.out)), exist_ok=True)
    sheet.save(a.out)
    print("wrote", a.out, sheet.size)


if __name__ == "__main__":
    main()
