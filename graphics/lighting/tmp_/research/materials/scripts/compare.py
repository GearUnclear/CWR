"""Build labelled comparison strips from a research capture run.

    python compare.py <run_dir> <view> <out.png> label[:Title] [label[:Title] ...]
        [--crop x0,y0,x1,y1] [--cols N] [--scale S]

Finds NNN_<view>__<label>.png in <run_dir>/tri (newest wins), copies each to
<run_dir>/<view>__<label>.png, reads the GPU frame time of each shot from the
UDCAP log lines, and writes one image: panels left to right (or in a grid with
--cols), each labelled with its title and GPU frame ms. --crop cuts the same
window out of every capture (pixel coordinates of the 960x640 shot) and scales
it up, for close looks at materials."""
import argparse
import re
import shutil
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

SHOT_RE = re.compile(r"^(\d{3})_(.+)\.png$")
UDCAP_RE = re.compile(r"UDCAP;(.*)$")
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")
FRAME_RE = re.compile(r"\bframe ([\d.]+) ms")
WORLD_RE = re.compile(r"\bworld ([\d.]+)")


def font(size):
    for name in ("arialbd.ttf", "arial.ttf", "DejaVuSans-Bold.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            pass
    return ImageFont.load_default()


def find_shots(tri: Path):
    best = {}
    for p in tri.rglob("*.png"):
        m = SHOT_RE.match(p.name)
        if not m:
            continue
        key = m.group(2)
        if key not in best or p.stat().st_mtime > best[key].stat().st_mtime:
            best[key] = p
    return best


def gpu_times(tri: Path):
    out = {}
    for p in sorted(tri.rglob("*.log")):
        try:
            text = p.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        for raw in text.splitlines():
            m = UDCAP_RE.search(ANSI_RE.sub("", raw))
            if not m:
                continue
            body = m.group(1).strip().rstrip('"')
            if "event=shot" not in body:
                continue
            lab = re.search(r"label=([^;]+)", body)
            gpu = body.split(";gpu=", 1)[1] if ";gpu=" in body else ""
            fm, wm = FRAME_RE.search(gpu), WORLD_RE.search(gpu)
            if lab:
                out[lab.group(1)] = (float(fm.group(1)) if fm else None, float(wm.group(1)) if wm else None)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("view")
    ap.add_argument("out")
    ap.add_argument("labels", nargs="+")
    ap.add_argument("--crop", default="")
    ap.add_argument("--cols", type=int, default=0)
    ap.add_argument("--scale", type=float, default=1.0)
    ap.add_argument("--title", default="")
    a = ap.parse_args()
    run = Path(a.run)
    tri = run / "tri"
    shots = find_shots(tri)
    times = gpu_times(tri)
    panels = []
    for spec in a.labels:
        label, _, title = spec.partition(":")
        if label.startswith("@"):
            # @<png path relative to the run dir or absolute>: a panel from another run
            path = Path(label[1:])
            if not path.is_absolute():
                path = run / path
            key = path.stem
            im = Image.open(path).convert("RGB")
        else:
            key = f"{a.view}__{label}"
            if key not in shots:
                raise SystemExit(f"missing shot {key} in {tri}")
            shutil.copy2(shots[key], run / f"{key}.png")
            im = Image.open(shots[key]).convert("RGB")
        if a.crop:
            x0, y0, x1, y1 = (int(v) for v in a.crop.split(","))
            im = im.crop((x0, y0, x1, y1))
        if a.scale != 1.0:
            im = im.resize((int(im.width * a.scale), int(im.height * a.scale)), Image.LANCZOS)
        panels.append((im, title or label, times.get(key, (None, None))))
    cols = a.cols or len(panels)
    rows = (len(panels) + cols - 1) // cols
    pw, ph = panels[0][0].size
    gap, top = 6, 40
    W = cols * pw + (cols - 1) * gap
    H = top + rows * ph + (rows - 1) * gap
    sheet = Image.new("RGB", (W, H), (24, 24, 24))
    d = ImageDraw.Draw(sheet)
    d.text((10, 8), a.title or a.view, fill=(235, 235, 235), font=font(20))
    fl, fs = font(20), font(15)
    for i, (im, title, (frame, world)) in enumerate(panels):
        x = (i % cols) * (pw + gap)
        y = top + (i // cols) * (ph + gap)
        sheet.paste(im, (x, y))
        tw = d.textlength(title, font=fl)
        d.rectangle((x + 6, y + 6, x + 14 + tw, y + 32), fill=(0, 0, 0))
        d.text((x + 10, y + 8), title, fill=(255, 255, 255), font=fl)
        if frame is not None:
            t = f"GPU frame {frame:.2f} ms" + (f" (world {world:.2f})" if world is not None else "")
            tw = d.textlength(t, font=fs)
            d.rectangle((x + 6, y + ph - 28, x + 14 + tw, y + ph - 6), fill=(0, 0, 0))
            d.text((x + 10, y + ph - 26), t, fill=(255, 255, 255), font=fs)
    sheet.save(a.out)
    print("wrote", a.out, sheet.size, {k: v for k, v in times.items() if k.startswith(a.view)})


if __name__ == "__main__":
    main()
