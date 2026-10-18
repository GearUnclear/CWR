"""Collect a lighting capture run into pairs, a contact sheet and timing tables.

    python make_pairs.py <out_dir> [--tri-dir <tri output dir>]

<out_dir> is the folder run-captures.ps1 made (tmp/captures/<timestamp>). With
--tri-dir (default <out_dir>/tri) this script first copies the screenshots
out of Trident's output tree and reads the UDCAP log lines; without new
screenshots there it just rebuilds from what is already in <out_dir>.

Writes into <out_dir>:
    <view>__classic.png, <view>__remastered.png   copied captures
    <view>__pair.png                              Classic left, Remastered right
    contact_sheet.png                             every pair on one page
    timings.csv, timings.json                     one row per shot (GPU + CPU, draws, load wait)
    timings.md                                    the same as a table, with deltas
    udcap.log                                     every raw UDCAP line

Needs Pillow (PIL). Only the standard library otherwise.
"""

import argparse
import csv
import json
import os
import re
import shutil
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

MODES = ("classic", "remastered")
SHOT_RE = re.compile(r"^(\d{3})_(.+)__(classic|remastered)\.png$")
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")
UDCAP_RE = re.compile(r"UDCAP;(.*)$")
GPU_FIELDS = ("frame", "world", "shadow", "post", "ao", "bloom", "shafts", "composite")
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
PERF_RE = re.compile(r"\bfps=([\d.]+) frame=([\d.]+)/([\d.]+)/([\d.]+)")
CALLS_RE = re.compile(r"\bcalls=([\d.]+)")
# A pair whose shot frames differ by more than this share of drawn objects
# did not see the same scene (a view still loading, a different object
# detail). Objects, not draw calls: the two lighting models can issue a few
# more or fewer draw calls for the same objects.
OBJECTS_MISMATCH = 0.05


def collect_shots(tri_dir: Path, out_dir: Path):
    """Copy NNN_<view>__<mode>.png out of the tri tree; the newest file wins
    (a retried test writes its second attempt under retry-N/)."""
    best = {}
    for p in tri_dir.rglob("*.png"):
        m = SHOT_RE.match(p.name)
        if not m:
            continue
        key = (m.group(2), m.group(3))
        if key not in best or p.stat().st_mtime > best[key].stat().st_mtime:
            best[key] = p
    for (view, mode), src in sorted(best.items()):
        shutil.copy2(src, out_dir / f"{view}__{mode}.png")
    return len(best)


def parse_kv(body: str):
    """'a=1;b=2;gpu=frame 1 | world 2' -> dict. gpu= is always the last field
    and may contain anything except a newline, so it is split off first."""
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


def read_udcap(tri_dir: Path):
    lines = []
    for p in sorted(tri_dir.rglob("*.log")):
        try:
            text = p.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        for raw in text.splitlines():
            line = ANSI_RE.sub("", raw)
            m = UDCAP_RE.search(line)
            if m:
                # logInfo lines arrive wrapped in the SQF string's own quotes
                lines.append((p, line.strip(), m.group(1).strip().rstrip('"')))
            elif "Terrain horizon map" in line:
                lines.append((p, line.strip(), None))
    return lines


def to_int(text):
    try:
        return int(text)
    except (TypeError, ValueError):
        return None


def shot_rows(lines):
    rows = {}
    modes = {}  # (view, mode) -> the latest event=mode line: load wait and pin
    for path, _line, body in lines:
        if body is None:
            continue
        kv = parse_kv(body)
        if kv.get("event") == "mode":
            modes[(kv.get("view", ""), kv.get("mode", ""))] = kv
            continue
        if kv.get("event") != "shot":
            continue
        label = kv.get("label", "")
        if "__" not in label:
            continue
        view, mode = label.rsplit("__", 1)
        row = {"view": view, "mode": mode, "lit": kv.get("lit", ""), "cam": kv.get("cam", ""),
               "fps_now": kv.get("fps", ""), "gpu_raw": kv.get("gpu", ""), "perf_raw": kv.get("perf", ""),
               "log": str(path)}
        # The shot frame's scene (triSceneStats) and the mode's load wait
        # (triSettleScene); empty in runs from before these fields existed.
        row["draws"] = to_int(kv.get("draws"))
        row["objects"] = to_int(kv.get("objects"))
        row["lights"] = to_int(kv.get("lights"))
        row["lod"] = kv.get("lod", "")
        row["pin"] = kv.get("pin", "")
        row["horizon"] = kv.get("horizon", "")
        mode_kv = modes.get((kv.get("view", ""), mode), {})
        row["settle"] = mode_kv.get("settle", "")
        row["settle_draws"] = to_int(mode_kv.get("draws"))
        for f, rx in GPU_RES.items():
            m = rx.search(row["gpu_raw"])
            row[f"gpu_{f}_ms"] = float(m.group(1)) if m else None
        m = PERF_RE.search(row["perf_raw"])
        row["cpu_fps"] = float(m.group(1)) if m else None
        row["cpu_frame_ms"] = float(m.group(2)) if m else None
        row["cpu_frame_p95_ms"] = float(m.group(3)) if m else None
        m = CALLS_RE.search(row["perf_raw"])
        row["calls_avg"] = float(m.group(1)) if m else None
        # a later line for the same label (a retry) replaces the earlier one;
        # dict order keeps the run order of the first appearance
        rows[(view, mode)] = row
    return list(rows.values())


def font(size):
    for name in ("arialbd.ttf", "arial.ttf", "segoeui.ttf", "DejaVuSans-Bold.ttf", "DejaVuSans.ttf"):
        for base in (Path(os.environ.get("WINDIR", "C:/Windows")) / "Fonts", Path("/usr/share/fonts/truetype/dejavu")):
            p = base / name
            if p.exists():
                return ImageFont.truetype(str(p), size)
    return ImageFont.load_default()


def label_box(draw, xy, text, fnt):
    x, y = xy
    box = draw.textbbox((x, y), text, font=fnt)
    draw.rectangle([box[0] - 6, box[1] - 4, box[2] + 6, box[3] + 4], fill=(0, 0, 0))
    draw.text((x, y), text, font=fnt, fill=(255, 255, 255))


def make_pair(view, out_dir: Path, rows_by_key):
    imgs = {}
    for mode in MODES:
        p = out_dir / f"{view}__{mode}.png"
        if p.exists():
            imgs[mode] = Image.open(p).convert("RGB")
    if not imgs:
        return None
    w = max(i.width for i in imgs.values())
    h = max(i.height for i in imgs.values())
    gap = 8
    title_h = 40
    pair = Image.new("RGB", (w * 2 + gap, h + title_h), (24, 24, 24))
    d = ImageDraw.Draw(pair)
    big = font(22)
    small = font(15)
    d.text((10, 8), view, font=big, fill=(235, 235, 235))
    for idx, mode in enumerate(MODES):
        x0 = idx * (w + gap)
        if mode in imgs:
            pair.paste(imgs[mode], (x0, title_h))
        else:
            d.rectangle([x0, title_h, x0 + w, title_h + h], fill=(60, 0, 0))
            d.text((x0 + 20, title_h + h // 2), "missing", font=big, fill=(255, 160, 160))
        label_box(d, (x0 + 12, title_h + 10), mode.capitalize(), big)
        row = rows_by_key.get((view, mode))
        foot = []
        if row and row.get("gpu_frame_ms") is not None:
            foot.append(f"GPU frame {row['gpu_frame_ms']:.2f} ms")
        if row and row.get("draws") is not None:
            foot.append(f"{row['draws']:,} draws")
        if row and row.get("objects") is not None:
            foot.append(f"{row['objects']:,} objects")
        if foot:
            label_box(d, (x0 + 12, title_h + h - 30), ", ".join(foot), small)
        if row and row.get("lit") and row["lit"] != "OK":
            label_box(d, (x0 + 12, title_h + 46), f"triSetLighting: {row['lit']}", small)
    dst = out_dir / f"{view}__pair.png"
    pair.save(dst)
    return dst


def contact_sheet(pairs, out_dir: Path, cols=2, thumb_w=960):
    if not pairs:
        return None
    thumbs = []
    for p in pairs:
        im = Image.open(p).convert("RGB")
        s = thumb_w / im.width
        thumbs.append(im.resize((thumb_w, int(im.height * s)), Image.LANCZOS))
    th = max(t.height for t in thumbs)
    pad = 10
    rows = (len(thumbs) + cols - 1) // cols
    sheet = Image.new("RGB", (cols * thumb_w + (cols + 1) * pad, rows * th + (rows + 1) * pad + 40), (16, 16, 16))
    d = ImageDraw.Draw(sheet)
    d.text((pad, 10), f"Lighting captures: Classic (left) | Remastered (right), {out_dir.name}", font=font(20),
           fill=(230, 230, 230))
    for i, t in enumerate(thumbs):
        r, c = divmod(i, cols)
        sheet.paste(t, (pad + c * (thumb_w + pad), 40 + pad + r * (th + pad)))
    dst = out_dir / "contact_sheet.png"
    sheet.save(dst)
    return dst


def fmt(v):
    return "" if v is None else f"{v:.2f}"


def share_delta(a, b, key):
    """Remastered minus Classic `key` (draws, objects) as a share of the larger, or None."""
    x, y = (a or {}).get(key), (b or {}).get(key)
    if x is None or y is None or max(x, y) <= 0:
        return None
    return (y - x) / max(x, y)


def write_tables(rows, out_dir: Path):
    cols = ["view", "mode", "lit", "cam"] + [f"gpu_{f}_ms" for f in GPU_FIELDS] + \
           ["cpu_fps", "cpu_frame_ms", "cpu_frame_p95_ms", "fps_now", "draws", "objects", "lights", "calls_avg", "lod",
            "pin", "horizon", "settle", "settle_draws", "gpu_raw", "perf_raw"]
    with open(out_dir / "timings.csv", "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=cols, extrasaction="ignore")
        w.writeheader()
        for r in rows:
            w.writerow(r)
    with open(out_dir / "timings.json", "w", encoding="utf-8") as f:
        json.dump(rows, f, indent=2)

    by = {(r["view"], r["mode"]): r for r in rows}
    views = sorted({r["view"] for r in rows}, key=lambda v: min(i for i, r in enumerate(rows) if r["view"] == v))
    md = ["# Lighting capture timings", "",
          "GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view",
          "before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns",
          "are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame",
          "(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =",
          "Remastered minus Classic (draws and objects: share of the larger).", "",
          "| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | "
          "load | lighting |",
          "|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|"]
    for v in views:
        for mode in MODES:
            r = by.get((v, mode))
            if not r:
                md.append(f"| {v} | {mode} | missing |  |  |  |  |  |  |  |  |  |  |  |  |")
                continue
            counts = ["" if r.get(k) is None else f"{r[k]:,}" for k in ("draws", "objects")]
            md.append("| " + " | ".join([v, mode] + [fmt(r[f"gpu_{f}_ms"]) for f in GPU_FIELDS] +
                                        [fmt(r["cpu_fps"])] + counts + [r.get("settle", ""), r["lit"]]) + " |")
        a, b = by.get((v, "classic")), by.get((v, "remastered"))
        if a and b:
            cells = []
            for f in GPU_FIELDS:
                x, y = a[f"gpu_{f}_ms"], b[f"gpu_{f}_ms"]
                cells.append("" if (x is None or y is None) else f"{y - x:+.2f}")
            shares = [share_delta(a, b, k) for k in ("draws", "objects")]
            md.append("| " + " | ".join([v, "delta"] + cells + [""] +
                                        ["" if d is None else f"{d:+.1%}" for d in shares] + ["", ""]) + " |")
    (out_dir / "timings.md").write_text("\n".join(md) + "\n", encoding="utf-8")


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("out_dir")
    ap.add_argument("--tri-dir", default=None)
    a = ap.parse_args(argv)
    out_dir = Path(a.out_dir).resolve()
    tri_dir = Path(a.tri_dir).resolve() if a.tri_dir else out_dir / "tri"
    out_dir.mkdir(parents=True, exist_ok=True)

    copied = collect_shots(tri_dir, out_dir) if tri_dir.exists() else 0
    lines = read_udcap(tri_dir) if tri_dir.exists() else []
    if lines:
        (out_dir / "udcap.log").write_text("\n".join(l for _p, l, _b in lines) + "\n", encoding="utf-8")
    rows = shot_rows(lines)
    if not rows and (out_dir / "timings.json").exists():
        rows = json.loads((out_dir / "timings.json").read_text(encoding="utf-8"))
    if rows:
        write_tables(rows, out_dir)

    rows_by_key = {(r["view"], r["mode"]): r for r in rows}
    views = []
    for p in sorted(out_dir.glob("*__*.png")):
        stem = p.stem
        if stem.endswith("__pair"):
            continue
        view, mode = stem.rsplit("__", 1)
        if mode in MODES and view not in views:
            views.append(view)
    # keep the run order when the log has it
    order = [r["view"] for r in rows]
    views.sort(key=lambda v: order.index(v) if v in order else len(order))
    pairs = [p for p in (make_pair(v, out_dir, rows_by_key) for v in views) if p]
    sheet = contact_sheet(pairs, out_dir)

    print(f"screenshots copied: {copied}; shot log lines: {len(rows)}; pairs: {len(pairs)}")
    missing = [f"{v}__{m}" for v in views for m in MODES if not (out_dir / f"{v}__{m}.png").exists()]
    if missing:
        print("missing captures: " + ", ".join(missing))
    bad = [f"{r['view']}__{r['mode']}={r['lit']}" for r in rows if r.get("lit") not in ("OK", "")]
    if bad:
        print("triSetLighting did not answer OK for: " + ", ".join(bad))
    unsettled = [f"{r['view']}__{r['mode']}={r['settle']}" for r in rows if r.get("settle", "").startswith("UNSETTLED")]
    if unsettled:
        print("view still loading when its timers started (triSettleScene ran out of frames): " + ", ".join(unsettled))
    by = {(r["view"], r["mode"]): r for r in rows}
    apart = []
    for v in dict.fromkeys(r["view"] for r in rows):
        dd = share_delta(by.get((v, "classic")), by.get((v, "remastered")), "objects")
        if dd is not None and abs(dd) > OBJECTS_MISMATCH:
            apart.append(f"{v} ({by[(v, 'classic')]['objects']:,} vs {by[(v, 'remastered')]['objects']:,})")
    if apart:
        print(f"pairs whose shot frames differ by more than {OBJECTS_MISMATCH:.0%} in drawn objects: " +
              ", ".join(apart))
    if sheet:
        print(f"contact sheet: {sheet}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
