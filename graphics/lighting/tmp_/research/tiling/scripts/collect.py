"""Collect a tiling research run: copy the shots out of the tri tree, read the
GPU timings from the UDCAP shot lines, and build labelled comparisons.

    python collect.py <run_dir> [--names v1=anti-tile+macro,...] [--crop x0,y0,x1,y1]

Writes into <run_dir>/cmp/:
    <view>__<mode>.png              copied shots (mode: classic, v0..v9)
    <view>__3way_v<k>.png           Classic | Remastered (v0) | Remastered + variant k
    <view>__all.png                 every mode of the view on one sheet
    <view>__crop_<mode...>.png      2x crops of one region, all modes side by side
    timings.csv                     view, mode, GPU frame / world ms
"""
import argparse
import csv
import re
import shutil
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

SHOT_RE = re.compile(r"^(\d{3})_(.+)__(classic|v\d)\.png$")
UDCAP_RE = re.compile(r"UDCAP;(.*)$")
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")


def font(size):
    for f in ("arialbd.ttf", "arial.ttf", "DejaVuSans-Bold.ttf"):
        try:
            return ImageFont.truetype(f, size)
        except OSError:
            pass
    return ImageFont.load_default()


def collect(tri_dir, out):
    best = {}
    for p in tri_dir.rglob("*.png"):
        m = SHOT_RE.match(p.name)
        if not m:
            continue
        key = (m.group(2), m.group(3))
        if key not in best or p.stat().st_mtime > best[key].stat().st_mtime:
            best[key] = p
    for (view, mode), src in best.items():
        shutil.copy2(src, out / f"{view}__{mode}.png")
    return sorted(best)


def timings(tri_dir):
    rows = {}
    for p in sorted(tri_dir.rglob("*.log")):
        try:
            text = p.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        for raw in text.splitlines():
            m = UDCAP_RE.search(ANSI_RE.sub("", raw))
            if not m:
                continue
            body = m.group(1).strip().rstrip('"')
            gpu = ""
            if ";gpu=" in body:
                body, gpu = body.split(";gpu=", 1)
            kv = dict(part.split("=", 1) for part in body.split(";") if "=" in part)
            if kv.get("event") != "shot":
                continue
            label = kv.get("label", "")
            if "__" not in label:
                continue
            view, mode = label.rsplit("__", 1)
            fr = re.search(r"\bframe ([\d.]+) ms", gpu)
            wo = re.search(r"\bworld ([\d.]+)", gpu)
            rows[(view, mode)] = (float(fr.group(1)) if fr else None, float(wo.group(1)) if wo else None, gpu)
    return rows


def label_img(im, title, ms, f_big, f_small):
    im = im.convert("RGB").copy()
    d = ImageDraw.Draw(im)
    tw = d.textlength(title, font=f_big)
    d.rectangle([6, 6, 16 + tw, 38], fill=(0, 0, 0))
    d.text((11, 8), title, fill=(255, 255, 255), font=f_big)
    if ms:
        d.rectangle([6, im.height - 30, 16 + d.textlength(ms, font=f_small), im.height - 6], fill=(0, 0, 0))
        d.text((11, im.height - 28), ms, fill=(255, 255, 255), font=f_small)
    return im


def ms_text(t):
    if not t or t[0] is None:
        return ""
    return f"GPU frame {t[0]:.2f} ms  world {t[1]:.2f} ms" if t[1] is not None else f"GPU frame {t[0]:.2f} ms"


def row(images, header, f_head):
    w = sum(i.width for i in images) + 6 * (len(images) - 1)
    h = max(i.height for i in images) + 40
    sheet = Image.new("RGB", (w, h), (24, 24, 24))
    d = ImageDraw.Draw(sheet)
    d.text((10, 8), header, fill=(240, 240, 240), font=f_head)
    x = 0
    for i in images:
        sheet.paste(i, (x, 40))
        x += i.width + 6
    return sheet


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run_dir")
    ap.add_argument("--names", default="v0=Remastered (current),v1=+ anti-tiling + macro,v2=+ relief,"
                                        "v3=+ strong + terrain cues,v4=relief only")
    ap.add_argument("--crop", default="", help="x0,y0,x1,y1 in the 960x640 frame; 2x crops of all modes")
    ap.add_argument("--crop-views", default="", help="view=x0,y0,x1,y1;view2=...")
    a = ap.parse_args()
    run = Path(a.run_dir)
    tri = run / "tri"
    out = run / "cmp"
    out.mkdir(exist_ok=True)
    names = dict(kv.split("=", 1) for kv in a.names.split(",") if "=" in kv)
    names["classic"] = "Classic"
    keys = collect(tri, out)
    tims = timings(tri)
    with open(out / "timings.csv", "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["view", "mode", "gpu_frame_ms", "gpu_world_ms", "gpu_raw"])
        for (v, m), t in sorted(tims.items()):
            w.writerow([v, m, t[0], t[1], t[2]])
    fb, fs, fh_ = font(22), font(16), font(20)
    views = sorted({v for v, _ in keys})
    crops = {}
    if a.crop_views:
        for part in a.crop_views.split(";"):
            v, box = part.split("=")
            crops[v] = tuple(int(c) for c in box.split(","))
    for v in views:
        modes = [m for vv, m in keys if vv == v]
        order = (["classic"] if "classic" in modes else []) + sorted(m for m in modes if m != "classic")
        imgs = {m: Image.open(out / f"{v}__{m}.png").convert("RGB") for m in order}
        lab = {m: label_img(imgs[m], names.get(m, m), ms_text(tims.get((v, m))), fb, fs) for m in order}
        for m in order:
            if m in ("classic", "v0"):
                continue
            trio = [lab[x] for x in ("classic", "v0", m) if x in lab]
            row(trio, f"{v}: Classic | Remastered (current) | Remastered + {names.get(m, m)}", fh_).save(
                out / f"{v}__3way_{m}.png")
        # all modes, two rows
        cols = 3
        tiles = [lab[m] for m in order]
        tw, th = tiles[0].width, tiles[0].height
        rows_n = (len(tiles) + cols - 1) // cols
        sheet = Image.new("RGB", (cols * tw + (cols - 1) * 6, rows_n * th + (rows_n - 1) * 6 + 40), (24, 24, 24))
        ImageDraw.Draw(sheet).text((10, 8), v, fill=(240, 240, 240), font=fh_)
        for i, t in enumerate(tiles):
            sheet.paste(t, ((i % cols) * (tw + 6), 40 + (i // cols) * (th + 6)))
        sheet.save(out / f"{v}__all.png")
        box = crops.get(v) or (tuple(int(c) for c in a.crop.split(",")) if a.crop else None)
        if box:
            cw, ch = box[2] - box[0], box[3] - box[1]
            cr = [label_img(imgs[m].crop(box).resize((cw * 2, ch * 2), Image.LANCZOS), names.get(m, m), "", fs, fs)
                  for m in order]
            row(cr, f"{v}: 2x crop {box}", fh_).save(out / f"{v}__crop.png")
    print(f"{len(keys)} shots, {len(views)} views -> {out}")
    for (v, m), t in sorted(tims.items()):
        print(f"  {v:22s} {m:8s} {ms_text(t)}")


if __name__ == "__main__":
    main()
