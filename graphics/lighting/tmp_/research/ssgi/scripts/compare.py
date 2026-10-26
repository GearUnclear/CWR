"""Builds labelled comparison PNGs and a timing table from a research capture run.

python compare.py <run dir> <out dir> [--best view=variant ...]

Reads <run dir>/tri/**/NNN_<view>__<label>.png and the UDCAP shot lines in
game_stdout.log (GPU summary per shot). Writes:
  <view>__3way_<variant>.png   Classic | Remastered | Remastered + <variant>
  <view>__all.png              every shot of the view in a grid
  <view>__crop_<variant>.png   2x crops of the regions that change most
  timings_<run>.md             GPU ms per shot
"""
import re
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

LABELS = {
    "classic": "Classic",
    "remastered": "Remastered (current)",
    "gi_m1s1": "+SSGI excess bounce x1",
    "gi_m1s4": "+SSGI excess bounce x4",
    "gi_m1s4_dbg": "SSGI x4: added light only (x4)",
    "gi_m1s4_notemporal": "+SSGI x4, no temporal",
    "gi_m3s1": "+SSGI traced sky+bounce (replaces AO)",
    "gi_m3s1_dbg": "SSGI traced: |change| only (x4)",
    "gi_m2s3": "+SSDO colour bleed x3",
    # v7 (resume): chroma albedo / share 1
    "gi_m1s1_chroma": "+SSGI excess bounce x1 (hue albedo)",
    "gi_m1s2_chroma": "+SSGI excess bounce x2 (hue albedo)",
    "gi_m6s1_chroma": "+SSGI full hit radiance (hue albedo)",
    "gi_m1s1_share1": "+SSGI excess bounce x1 (share albedo)",
    "gi_m1s1_chroma_dbg": "SSGI x1: added light only (x4)",
    "gi_m1s1_fb1": "+SSGI excess bounce x1 (object share 1)",
    "gi_m6s1_fb1": "+SSGI full hit radiance (object share 1)",
    "gi_m6s1_fb1_dbg": "SSGI full hit, share 1: added only (x4)",
}


def font(size):
    for f in ("arialbd.ttf", "arial.ttf", "DejaVuSans-Bold.ttf"):
        try:
            return ImageFont.truetype(f, size)
        except OSError:
            pass
    return ImageFont.load_default()


def parse_gpu(log):
    out = {}
    for line in log.read_text(encoding="utf-8", errors="replace").splitlines():
        if "event=shot" not in line or "UDCAP" not in line:
            continue
        m = re.search(r"label=([^;]+);", line)
        g = re.search(r"gpu=(.*?)\"?$", line)
        if not m or not g:
            continue
        gpu = g.group(1)
        d = {}
        fm = re.search(r"frame ([\d.]+) ms", gpu)
        if fm:
            d["frame"] = float(fm.group(1))
        for key in ("world", "post", "ao", "bloom", "shafts", "composite"):
            km = re.search(rf"\b{key} ([\d.]+)", gpu)
            if km:
                d[key] = float(km.group(1))
        d["raw"] = gpu
        out[m.group(1)] = d
    return out


def panel(img, title, ms, f1, f2):
    w, h = img.size
    bar = 34
    canvas = Image.new("RGB", (w, h + bar), (18, 18, 18))
    canvas.paste(img, (0, bar))
    d = ImageDraw.Draw(canvas)
    d.text((8, 6), title, fill=(240, 240, 240), font=f1)
    if ms is not None:
        txt = f"GPU {ms:.2f} ms"
        tw = d.textlength(txt, font=f2)
        d.text((w - tw - 8, 8), txt, fill=(255, 210, 120), font=f2)
    return canvas


def hstack(panels, gap=6):
    w = sum(p.size[0] for p in panels) + gap * (len(panels) - 1)
    h = max(p.size[1] for p in panels)
    out = Image.new("RGB", (w, h), (0, 0, 0))
    x = 0
    for p in panels:
        out.paste(p, (x, 0))
        x += p.size[0] + gap
    return out


def main():
    run = Path(sys.argv[1])
    out = Path(sys.argv[2])
    out.mkdir(parents=True, exist_ok=True)
    shots = {}
    gpu = {}
    for log in run.glob("tri/**/game_stdout.log"):
        gpu.update(parse_gpu(log))
    for png in sorted(run.glob("tri/**/*.png")):
        m = re.match(r"\d+_(.+?)__(.+)\.png$", png.name)
        if m:
            shots.setdefault(m.group(1), {})[m.group(2)] = png
    f1, f2 = font(18), font(16)
    rows = ["| view | shot | GPU frame ms | world | post | ao (+GI) | delta frame vs remastered |", "|---|---|---|---|---|---|---|"]
    for view, by in shots.items():
        base = gpu.get(f"{view}__remastered", {}).get("frame")
        for label, path in by.items():
            g = gpu.get(f"{view}__{label}", {})
            delta = "" if base is None or "frame" not in g else f"{g['frame'] - base:+.2f}"
            rows.append(
                f"| {view} | {label} | {g.get('frame', '')} | {g.get('world', '')} | {g.get('post', '')} | {g.get('ao', '')} | {delta} |"
            )
        imgs = {k: Image.open(v).convert("RGB") for k, v in by.items()}
        if "classic" in imgs and "remastered" in imgs:
            for label, im in imgs.items():
                if label in ("classic", "remastered"):
                    continue
                ps = [
                    panel(imgs["classic"], "Classic", gpu.get(f"{view}__classic", {}).get("frame"), f1, f2),
                    panel(imgs["remastered"], "Remastered (current)", base, f1, f2),
                    panel(im, LABELS.get(label, label), gpu.get(f"{view}__{label}", {}).get("frame"), f1, f2),
                ]
                hstack(ps).save(out / f"{view}__3way_{label}.png")
                # 2x crops where the variant differs most from Remastered.
                if not label.endswith("_dbg"):
                    a = np.asarray(imgs["remastered"]).astype(float)
                    b = np.asarray(im).astype(float)
                    diff = np.abs(b - a).sum(axis=2)
                    k = 160
                    best, by_ = -1, (0, 0)
                    for y in range(0, a.shape[0] - k, 40):
                        for x in range(0, a.shape[1] - k, 40):
                            s = diff[y:y + k, x:x + k].sum()
                            if s > best:
                                best, by_ = s, (x, y)
                    x, y = by_
                    crops = [
                        panel(imgs[n].crop((x, y, x + k, y + k)).resize((k * 3, k * 3), Image.NEAREST), t, None, f1, f2)
                        for n, t in (("remastered", "Remastered"), (label, LABELS.get(label, label)))
                    ]
                    hstack(crops).save(out / f"{view}__crop_{label}.png")
        # grid of everything
        names = list(imgs)
        cols = 3
        thumbs = [panel(imgs[n].resize((480, 320), Image.LANCZOS), LABELS.get(n, n), gpu.get(f"{view}__{n}", {}).get("frame"), font(13), font(12)) for n in names]
        tw, th = thumbs[0].size
        r = (len(thumbs) + cols - 1) // cols
        grid = Image.new("RGB", (cols * tw + (cols - 1) * 4, r * th + (r - 1) * 4), (0, 0, 0))
        for i, t in enumerate(thumbs):
            grid.paste(t, ((i % cols) * (tw + 4), (i // cols) * (th + 4)))
        grid.save(out / f"{view}__all.png")
    (out / f"timings_{run.name}.md").write_text("\n".join(rows) + "\n", encoding="utf-8")
    print("views:", list(shots), "->", out)


if __name__ == "__main__":
    main()
