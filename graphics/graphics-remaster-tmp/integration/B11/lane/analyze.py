"""B11 lane analysis: off equals before, on against off, crops and the cost rounds.

    python analyze.py <before_run> <new_run> [--out <dir>]

  - gate item 5: every <view>__off.png of the new run against <view>__on.png and
    <view>__off.png of the before run (banddiff bands), plus the before run's
    own off against on (its repeat noise in one game: there the switches are
    inert);
  - gate item 6/8: on against off in the new run: band differences, and on the
    ground band (rows 591-1080) the mean luma, the R/G and G/B ratios of the
    mean colour, the 5th / 95th luma percentiles and a sharpness proxy (mean
    absolute Laplacian of the luma), on against off;
  - crops: <view>__crop.png, off | on of a box at 2x nearest, and
    <view>__sbs.png, the two full frames side by side at half size;
  - cost: the b11cost_* rounds of each run (gpu frame, ground section, post,
    sharpen), on minus off per round and the median.
"""
import argparse
import re
import statistics
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

sys.path.insert(0, r"D:\Arma_CWA_worktrees\gr-B11\arma_CWR\tests\rendering\captures")
import banddiff  # noqa: E402

GROUND = (591, 1080)
# Crop boxes (x0, y0, x1, y1) per view, on the terrain the step is about.
CROPS = {
    "texsharp_forest_edge": (700, 380, 1340, 740),
    "b11_sharpen_forest_edge": (700, 380, 1340, 740),
    "b11_noon_fields": (640, 560, 1280, 920),
    "b11_coastline_sunset": (600, 520, 1240, 880),
    "b11_lowsun_fp": (600, 600, 1240, 960),
    "b11_overcast_broken": (640, 560, 1280, 920),
}


def load(p):
    return np.asarray(Image.open(p).convert("RGB")).astype(np.float64)


def luma(a):
    return a[..., 0] * 0.2126 + a[..., 1] * 0.7152 + a[..., 2] * 0.0722


def ground_metrics(a):
    g = a[GROUND[0]:GROUND[1]]
    y = luma(g)
    lap = np.abs(4 * y[1:-1, 1:-1] - y[:-2, 1:-1] - y[2:, 1:-1] - y[1:-1, :-2] - y[1:-1, 2:])
    m = g.reshape(-1, 3).mean(axis=0)
    return {"luma": y.mean(), "rg": m[0] / m[1], "gb": m[1] / m[2], "p5": np.percentile(y, 5),
            "p95": np.percentile(y, 95), "sharp": lap.mean()}


def band_line(d):
    return " | ".join(f"{b['max']} / {b['over2']} / {b['over8']}" for b in d)


def views(run, label):
    return sorted(p.name[: -len(f"__{label}.png")] for p in Path(run).glob(f"*__{label}.png"))


GPU_RE = re.compile(r"label=([^;]+);.*?gpu=frame ([\d.]+) ms \| world ([\d.]+).*?\[ground ([\d.]+).*?\| post ([\d.]+):"
                    r"(.*?)\| min")


def gpu_rows(run):
    rows = {}
    log = Path(run) / "udcap.log"
    if not log.exists():
        return rows
    for line in log.read_text(encoding="utf-8", errors="replace").splitlines():
        if "event=shot" not in line or "gpu=frame" not in line:
            continue
        m = GPU_RE.search(line)
        if not m:
            continue
        sharpen = re.search(r"sharpen ([\d.]+)", m.group(6))
        rows[m.group(1)] = {"frame": float(m.group(2)), "world": float(m.group(3)), "ground": float(m.group(4)),
                            "post": float(m.group(5)), "sharpen": float(sharpen.group(1)) if sharpen else 0.0}
    return rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("before")
    ap.add_argument("new")
    ap.add_argument("--out", default=None)
    a = ap.parse_args()
    before, new = Path(a.before), Path(a.new)
    out = Path(a.out) if a.out else new / "analysis"
    out.mkdir(parents=True, exist_ok=True)
    md = []

    md.append(f"# B11 lane analysis\n\nbefore: `{before.name}`, new: `{new.name}`\n")
    md.append("Band columns: sky | horizon | ground | whole, each `max / >2 / >8` levels.\n")
    md.append("## Off equals before (gate item 5)\n")
    md.append("| view | new off vs before on | new off vs before off | before off vs before on (noise) |")
    md.append("|---|---|---|---|")
    for v in views(new, "off"):
        nb = before / f"{v}__on.png"
        if not nb.exists():
            continue
        off = banddiff.load(new / f"{v}__off.png")
        d1 = banddiff.diff(off, banddiff.load(nb))
        d2 = banddiff.diff(off, banddiff.load(before / f"{v}__off.png"))
        d3 = banddiff.diff(banddiff.load(before / f"{v}__off.png"), banddiff.load(nb))
        md.append(f"| {v} | {band_line(d1)} | {band_line(d2)} | {band_line(d3)} |")

    md.append("\n## On against off in the new run (gate items 6 and 8)\n")
    md.append("| view | bands on vs off | ground luma off / on | R/G off / on | G/B off / on | p5 off / on | "
              "p95 off / on | sharpness off / on (change) |")
    md.append("|---|---|---|---|---|---|---|---|")
    for v in views(new, "off"):
        pon = new / f"{v}__on.png"
        if not pon.exists():
            continue
        aoff, aon = load(new / f"{v}__off.png"), load(pon)
        d = banddiff.diff(aoff.astype(np.int16), aon.astype(np.int16))
        mo, mn = ground_metrics(aoff), ground_metrics(aon)
        md.append(f"| {v} | {band_line(d)} | {mo['luma']:.2f} / {mn['luma']:.2f} | {mo['rg']:.3f} / {mn['rg']:.3f} | "
                  f"{mo['gb']:.3f} / {mn['gb']:.3f} | {mo['p5']:.1f} / {mn['p5']:.1f} | "
                  f"{mo['p95']:.1f} / {mn['p95']:.1f} | {mo['sharp']:.2f} / {mn['sharp']:.2f} "
                  f"({(mn['sharp'] / mo['sharp'] - 1) * 100:+.1f}%) |")
        box = CROPS.get(v)
        im_off, im_on = Image.open(new / f"{v}__off.png").convert("RGB"), Image.open(pon).convert("RGB")
        if box:
            w, h = box[2] - box[0], box[3] - box[1]
            c = Image.new("RGB", (w * 4 + 8, h * 2), (20, 20, 20))
            c.paste(im_off.crop(box).resize((w * 2, h * 2), Image.NEAREST), (0, 0))
            c.paste(im_on.crop(box).resize((w * 2, h * 2), Image.NEAREST), (w * 2 + 8, 0))
            dr = ImageDraw.Draw(c)
            dr.text((8, 8), f"{v} off (2x)", fill=(255, 255, 0))
            dr.text((w * 2 + 16, 8), f"{v} on (2x)", fill=(255, 255, 0))
            c.save(out / f"{v}__crop.png")
        s = Image.new("RGB", (1920 + 8, 540), (20, 20, 20))
        s.paste(im_off.resize((960, 540), Image.LANCZOS), (0, 0))
        s.paste(im_on.resize((960, 540), Image.LANCZOS), (968, 0))
        dr = ImageDraw.Draw(s)
        dr.text((8, 8), f"{v} off", fill=(255, 255, 0))
        dr.text((976, 8), f"{v} on", fill=(255, 255, 0))
        s.save(out / f"{v}__sbs.png")

    md.append("\n## Cost rounds (GPU ms, smoothed over the 90 frames before each shot)\n")
    for run in (before, new):
        rows = gpu_rows(run)
        md.append(f"\n### {run.name}\n")
        md.append("| view | round | frame off / on | ground off / on | post off / on | sharpen off / on |")
        md.append("|---|---|---|---|---|---|")
        for view in sorted({k.split("__")[0] for k in rows if k.startswith("b11cost_")}):
            df, dg = [], []
            for r in range(3):
                o, n = rows.get(f"{view}__r{r}_off"), rows.get(f"{view}__r{r}_on")
                if not o or not n:
                    continue
                df.append(n["frame"] - o["frame"])
                dg.append(n["ground"] - o["ground"])
                md.append(f"| {view} | {r} | {o['frame']:.2f} / {n['frame']:.2f} | {o['ground']:.2f} / {n['ground']:.2f}"
                          f" | {o['post']:.2f} / {n['post']:.2f} | {o['sharpen']:.2f} / {n['sharpen']:.2f} |")
            if df:
                md.append(f"| {view} | median on - off | {statistics.median(df):+.2f} | {statistics.median(dg):+.2f} | | |")
    (out / "analysis.md").write_text("\n".join(md) + "\n", encoding="utf-8")
    print("\n".join(md))


main()
