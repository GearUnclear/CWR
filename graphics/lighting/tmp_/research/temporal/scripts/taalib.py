"""Helpers for the temporal (TAA) research track: collect captures out of a
tri tree, parse the UDCAP shot lines for GPU timings, and build labelled
comparison sheets and crops with Pillow."""

import json
import re
import shutil
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

SHOT_RE = re.compile(r"^(\d{3})_(.+)\.png$")
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")
UDCAP_RE = re.compile(r"UDCAP;(.*)$")
GPU_RES = {
    "frame": re.compile(r"\bframe ([\d.]+) ms"),
    "world": re.compile(r"\bworld ([\d.]+)"),
    "shadow": re.compile(r"\(shadow ([\d.]+)\)"),
    "post": re.compile(r"\bpost ([\d.]+):"),
    "ao": re.compile(r"\bao ([\d.]+)"),
    "bloom": re.compile(r"\bbloom ([\d.]+)"),
    "shafts": re.compile(r"\bshafts ([\d.]+)"),
    "composite": re.compile(r"\bcomposite ([\d.]+)"),
}


def font(size):
    for name in ("arialbd.ttf", "arial.ttf", "DejaVuSans-Bold.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            pass
    return ImageFont.load_default()


def collect(tri_dir: Path, out_dir: Path):
    """Copy NNN_<label>.png out of the tri tree (newest wins) to out_dir/<label>.png."""
    best = {}
    for p in Path(tri_dir).rglob("*.png"):
        m = SHOT_RE.match(p.name)
        if not m:
            continue
        label = m.group(2)
        if label not in best or p.stat().st_mtime > best[label].stat().st_mtime:
            best[label] = p
    out_dir = Path(out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    for label, src in sorted(best.items()):
        shutil.copy2(src, out_dir / f"{label}.png")
    return sorted(best)


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


def timings(tri_dir: Path):
    """label -> {gpu fields (ms), raw gpu string, perf}. Last shot line wins."""
    rows = {}
    udcap = []
    for p in sorted(Path(tri_dir).rglob("*.log")):
        try:
            text = p.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        for raw in text.splitlines():
            line = ANSI_RE.sub("", raw)
            m = UDCAP_RE.search(line)
            if not m:
                continue
            body = m.group(1).strip().rstrip('"')
            udcap.append(body)
            kv = parse_kv(body)
            if kv.get("event") != "shot":
                continue
            r = {"gpu_raw": kv.get("gpu", ""), "perf": kv.get("perf", ""), "fps": kv.get("fps", ""),
                 "mode": kv.get("mode", "")}
            for f, rx in GPU_RES.items():
                mm = rx.search(r["gpu_raw"])
                r[f] = float(mm.group(1)) if mm else None
            rows[kv.get("label", "")] = r
    return rows, udcap


def label_panel(img: Image.Image, title: str, sub: str = "", scale_font=1.0):
    img = img.convert("RGB").copy()
    d = ImageDraw.Draw(img)
    f1 = font(int(20 * scale_font))
    f2 = font(int(15 * scale_font))
    pad = 6
    tw = d.textbbox((0, 0), title, font=f1)
    d.rectangle([4, 4, 4 + tw[2] + 2 * pad, 4 + tw[3] + 2 * pad], fill=(0, 0, 0))
    d.text((4 + pad, 4 + pad - 2), title, fill=(255, 255, 255), font=f1)
    if sub:
        sw = d.textbbox((0, 0), sub, font=f2)
        y = img.height - (sw[3] + 2 * pad) - 4
        d.rectangle([4, y, 4 + sw[2] + 2 * pad, y + sw[3] + 2 * pad], fill=(0, 0, 0))
        d.text((4 + pad, y + pad - 2), sub, fill=(255, 255, 255), font=f2)
    return img


def sheet(panels, cols, header="", gap=6, bg=(24, 24, 24)):
    """panels: list of PIL images (same size). Returns a grid with an optional header line."""
    w, h = panels[0].size
    rows = (len(panels) + cols - 1) // cols
    head = 40 if header else 0
    out = Image.new("RGB", (cols * w + (cols - 1) * gap, head + rows * h + (rows - 1) * gap), bg)
    if header:
        ImageDraw.Draw(out).text((10, 8), header, fill=(235, 235, 235), font=font(22))
    for i, p in enumerate(panels):
        r, c = divmod(i, cols)
        out.paste(p, (c * (w + gap), head + r * (h + gap)))
    return out


def crop2x(img: Image.Image, box):
    """Crop box (x0, y0, x1, y1) and enlarge 2x with nearest neighbour (pixels stay visible)."""
    c = img.crop(box)
    return c.resize((c.width * 2, c.height * 2), Image.NEAREST)


def gpu_sub(t):
    if not t or t.get("frame") is None:
        return "GPU frame n/a"
    s = f"GPU frame {t['frame']:.2f} ms"
    if t.get("composite") is not None:
        s += f" | post {t.get('post') or 0:.2f} (composite+TAA {t['composite']:.2f})"
    return s


def save_json(path, obj):
    Path(path).write_text(json.dumps(obj, indent=2), encoding="utf-8")
