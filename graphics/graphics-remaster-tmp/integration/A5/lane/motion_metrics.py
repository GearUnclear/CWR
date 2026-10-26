"""A5 lane: metrics of the motion clips (a5strafe, a5pan, a5fire), off pass against on pass.

    python motion_metrics.py <run folder> [--sheets <out dir>]

  a5pan    steadiness: consecutive frames registered by the best integer shift
           (searched over dx 0..6, dy -1..1) in a central crop, mean absolute
           luma residual (lower = steadier), and the sharpness (mean absolute
           neighbour difference) of the same crop; the prototype's pan metric.
  a5strafe ghosting: per frame, the pixels where on and off differ by more
           than 24 levels in the Jeep's rows, against the same count on the
           still part of the frame; a ghost trail shows as extra on-only
           pixels behind the Jeep in the fast segment (frames 20 to 39).
  a5fire   the particle mask: in the flame and smoke region (pixels whose
           luma changes over the off frames, temporal std above 0.06), the
           temporal std, the mean, the mean of the brightest 1% and the
           frame-to-frame change, off against on: a smeared or dimmed fire
           shows as less change and a lower peak. Also the frame-to-frame
           change of the still part of the frame (TAA's jitter residual).
"""
import argparse
import glob
import json
import os
import re

import numpy as np
from PIL import Image


def frames(run, clip, pas):
    out = {}
    for p in glob.glob(os.path.join(run, "tri", "**", f"*_{clip}__{pas}__f*.png"), recursive=True):
        m = re.search(r"__f(\d+)\.png$", p)
        out[int(m.group(1))] = p
    return [out[k] for k in sorted(out)]


def luma(p):
    a = np.asarray(Image.open(p).convert("RGB")).astype(np.float64) / 255.0
    return 0.2126 * a[..., 0] + 0.7152 * a[..., 1] + 0.0722 * a[..., 2]


def rgb(p):
    return np.asarray(Image.open(p).convert("RGB")).astype(np.int32)


def sharp(l):
    return 0.5 * (np.abs(np.diff(l, axis=0)).mean() + np.abs(np.diff(l, axis=1)).mean())


def pan(run, box=(760, 200, 1160, 900)):
    x0, y0, x1, y1 = box
    res = {}
    for pas in ("off", "on"):
        fs = frames(run, "a5pan", pas)
        resid, shifts, sh = [], [], []
        prev = None
        for p in fs:
            cur = luma(p)
            sh.append(sharp(cur[y0:y1, x0:x1]))
            if prev is not None:
                best = None
                for dx in range(0, 7):
                    for dy in (-1, 0, 1):
                        a = cur[y0:y1, x0:x1]
                        b = prev[y0 + dy:y1 + dy, x0 + dx:x1 + dx]
                        r = np.abs(a - b).mean()
                        if best is None or r < best[0]:
                            best = (r, dx, dy)
                resid.append(best[0])
                shifts.append((best[1], best[2]))
            prev = cur
        res[pas] = dict(frames=len(fs), residual=float(np.mean(resid)) if resid else None,
                        sharpness=float(np.mean(sh)) if sh else None,
                        shifts=sorted(set(map(tuple, shifts))))
    if res["off"]["residual"] and res["on"]["residual"]:
        res["residual_change_pct"] = 100.0 * (res["on"]["residual"] / res["off"]["residual"] - 1.0)
        res["sharpness_change_pct"] = 100.0 * (res["on"]["sharpness"] / res["off"]["sharpness"] - 1.0)
    return res


def strafe(run, rows=(520, 700)):
    off = frames(run, "a5strafe", "off")
    on = frames(run, "a5strafe", "on")
    n = min(len(off), len(on))
    per = []
    for k in range(n):
        a, b = rgb(off[k]), rgb(on[k])
        d = np.abs(a - b).max(axis=2)
        band = d[rows[0]:rows[1]]
        rest = np.concatenate([d[:rows[0]], d[rows[1]:]])
        per.append(dict(k=k + 1, band_over24=int((band > 24).sum()),
                        rest_over24_per_band=float((rest > 24).sum() * band.size / rest.size)))
    slow = [p["band_over24"] for p in per if p["k"] <= 20]
    fast = [p["band_over24"] for p in per if p["k"] > 21]
    return dict(frames=n, slow_mean=float(np.mean(slow)) if slow else None,
                fast_mean=float(np.mean(fast)) if fast else None, per_frame=per)


def fire(run, thr=0.06):
    off = frames(run, "a5fire", "off")
    on = frames(run, "a5fire", "on")
    n = min(len(off), len(on))
    lo = np.stack([luma(p) for p in off[:n]])
    lon = np.stack([luma(p) for p in on[:n]])
    # The flames and smoke: the pixels that change over the off frames (the
    # camera is still), by their temporal standard deviation.
    mask = lo.std(axis=0) > thr
    still = lo.std(axis=0) <= 0.03
    out = {"region_px": int(mask.sum()), "threshold": thr}
    k = max(1, int(mask.sum()) // 100)
    for name, ls in (("off", lo), ("on", lon)):
        out[name] = dict(temporal_std=float(ls.std(axis=0)[mask].mean()), mean=float(ls[:, mask].mean()),
                         peak_top1pct=float(np.mean([np.sort(l[mask])[-k:].mean() for l in ls])),
                         frame_change=float(np.abs(np.diff(ls[:, mask], axis=0)).mean()),
                         still_frame_change=float(np.abs(np.diff(ls[:, still], axis=0)).mean()))
    return out


def sheet(run, out_dir):
    os.makedirs(out_dir, exist_ok=True)
    off = frames(run, "a5strafe", "off")
    on = frames(run, "a5strafe", "on")
    rows = []
    for k in (10, 25, 30, 35, 39):
        if k > min(len(off), len(on)):
            continue
        a = Image.open(off[k - 1]).convert("RGB").crop((700, 470, 1500, 720))
        b = Image.open(on[k - 1]).convert("RGB").crop((700, 470, 1500, 720))
        r = Image.new("RGB", (a.width * 2 + 6, a.height), (255, 0, 255))
        r.paste(a, (0, 0))
        r.paste(b, (a.width + 6, 0))
        rows.append(r)
    if rows:
        h = sum(r.height + 6 for r in rows)
        o = Image.new("RGB", (rows[0].width, h), (40, 40, 40))
        y = 0
        for r in rows:
            o.paste(r, (0, y))
            y += r.height + 6
        o.save(os.path.join(out_dir, "a5strafe_jeep_off_on.png"))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--sheets", default="")
    a = ap.parse_args()
    res = dict(pan=pan(a.run), strafe=strafe(a.run), fire=fire(a.run))
    short = json.loads(json.dumps(res))
    short["strafe"].pop("per_frame", None)
    print(json.dumps(short, indent=1))
    with open(os.path.join(a.run, "motion_metrics.json"), "w") as f:
        json.dump(res, f, indent=1)
    if a.sheets:
        sheet(a.run, a.sheets)


main()
