"""Median and minimum over the interleaved rounds of a timing run.

    python summarize_timing.py <run dir> [<run dir> ...] --out <timing.md> [--json <timing.json>]

Each run dir is a run-captures.ps1 output folder whose timings.json was written
by make_pairs.py from the gen_timing.py tests (view labels <view>_r<k>);
run-timing.ps1 calls this at the end. Rows group by (view, mode); every GPU
section and the CPU frame get the median and the minimum over the rounds; draws
and objects are medians. A second table gives the same-config repeat noise of
the Remastered shots (pixel difference between rounds, per row band, the
banddiff.py measure at 1080 rows).

If a run dir holds gpu_samples.csv (gpu_sampler.ps1), every shot also gets the
GPU graphics clock and P-state of the samples stamped from CLOCK_WINDOW seconds
before it up to the shot (the shot's timer window), joined on the udcap.log shot
time. gpu_sampler.ps1 stamps its rows in whole seconds (HH:mm:ss, truncated) and
writes one every 2 to 3 s, so the window edges are good to about a second: the
row stamped in the shot's own second may have been read just after the shot.
Compare a light view's milliseconds only at a matching clock (plan section 8
item 7): the GPU picks its clock from the load.
"""
import argparse
import csv
import datetime as dt
import itertools
import json
import re
import statistics as st
from pathlib import Path

import numpy as np
from PIL import Image

SECTIONS = ("frame", "world", "shadow", "post", "resolve", "lindepth", "ao", "bloom", "shafts", "composite")
ROUND_RE = re.compile(r"^(.+)_r(\d+)$")
# banddiff.py's 640-row bands (0-250 sky, 250-350 horizon, 350-640 ground) at 1080 rows
BANDS = ((0, 422), (422, 591), (591, 1080))
# seconds before a shot whose GPU samples describe its timer window (90 frames);
# the window ends at the shot time (round 1 also took rows up to 0.5 s after it)
CLOCK_WINDOW = 3.0
SHOT_RE = re.compile(r"^\[(\d{4}-\d\d-\d\d \d\d:\d\d:\d\d\.\d+)\].*UDCAP;view=[^;]*;event=shot;label=([^;\"]+)")


def shot_clocks(run_dir):
    """label -> (median MHz, sorted P-states, sample count) from gpu_samples.csv."""
    d = Path(run_dir)
    sp, lp = d / "gpu_samples.csv", d / "udcap.log"
    if not sp.exists() or not lp.exists():
        return {}
    shots = {}
    for line in lp.read_text(encoding="utf-8", errors="replace").splitlines():
        m = SHOT_RE.match(line)
        if m:
            shots[m.group(2)] = dt.datetime.strptime(m.group(1)[:23], "%Y-%m-%d %H:%M:%S.%f")
    if not shots:
        return {}
    day = min(shots.values()).date()
    samples = []
    with sp.open(encoding="ascii", errors="replace") as f:
        for r in csv.DictReader(f):
            try:
                t = dt.datetime.combine(day, dt.datetime.strptime(r["time"], "%H:%M:%S").time())
                samples.append((t, float(r["clock_gr"]), r["pstate"], r.get("poseidon_games", "")))
            except (ValueError, KeyError, TypeError):
                continue
    out = {}
    for label, t in shots.items():
        win = [s for s in samples if t - dt.timedelta(seconds=CLOCK_WINDOW) <= s[0] <= t]
        if win:
            out[label] = (st.median(s[1] for s in win), sorted({s[2] for s in win}), len(win),
                          sorted({s[3] for s in win}))
    return out


def load(run_dirs):
    rows = []
    for d in run_dirs:
        p = Path(d) / "timings.json"
        for r in json.loads(p.read_text(encoding="utf-8")):
            m = ROUND_RE.match(r["view"])
            if not m:
                continue
            r = dict(r)
            r["base"], r["round"], r["dir"] = m.group(1), int(m.group(2)), str(d)
            rows.append(r)
    return rows


def med(vals):
    vals = [v for v in vals if v is not None]
    return st.median(vals) if vals else None


def mn(vals):
    vals = [v for v in vals if v is not None]
    return min(vals) if vals else None


def f2(v):
    return "" if v is None else f"{v:.2f}"


def band_noise(dir_, base, rounds, mode="remastered"):
    imgs = []
    for k in rounds:
        p = Path(dir_) / f"{base}_r{k}__{mode}.png"
        if p.exists():
            imgs.append(np.asarray(Image.open(p).convert("RGB")).astype(np.int16))
    out = []
    for a, b in itertools.combinations(imgs, 2):
        if a.shape != b.shape:
            continue
        m = np.abs(a - b).max(axis=2)
        h = m.shape[0]
        cells = []
        for lo, hi in BANDS:
            lo2, hi2 = int(lo * h / 1080), int(hi * h / 1080)
            s = m[lo2:hi2]
            cells.append((int(s.max()), int((s > 2).sum()), int((s > 8).sum())))
        cells.append((int(m.max()), int((m > 2).sum()), int((m > 8).sum())))
        out.append(cells)
    return out, (imgs[0].shape if imgs else None)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("runs", nargs="+")
    ap.add_argument("--out", required=True)
    ap.add_argument("--json", default=None)
    a = ap.parse_args()
    rows = load(a.runs)
    clocks = {}
    for d in a.runs:
        for label, c in shot_clocks(d).items():
            clocks[(str(d), label)] = c
    for r in rows:
        c = clocks.get((r["dir"], f"{r['view']}__{r['mode']}"))
        r["gpu_mhz"], r["pstates"], r["games"] = (c[0], c[1], c[3]) if c else (None, [], [])
    groups = {}
    for r in rows:
        groups.setdefault((r["base"], r["mode"]), []).append(r)
    order = list(dict.fromkeys((r["base"], r["mode"]) for r in rows))

    md = ["# Timing: median / minimum over interleaved rounds", "",
          "GPU sections in ms (`triGpuTimes`, smoothed over the 90 frames before each shot), as",
          "`median / min` over the rounds; CPU frame ms from `triPerfStats` (median / min); draws and",
          "objects are the shot frame's medians; n = rounds found. 1920x1080 windowed, render scale 1.0,",
          "4x MSAA, presented frame.", "",
          "GPU MHz: median graphics clock of the nvidia-smi samples stamped from 3 s before each shot",
          "up to the shot, median over the rounds, with the P-states seen (the driver picks the clock",
          "from the load, so light views run at lower clocks than heavy ones). gpu_sampler.ps1 stamps",
          "its rows in whole seconds and writes one every 2 to 3 s, so the window edges are good to",
          "about a second: the row stamped in the shot's own second may have been read just after it.", "",
          "| view | mode | n | " + " | ".join(SECTIONS) + " | CPU frame | draws | objects | GPU MHz | P-states | settle |",
          "|---|---|---|" + "---|" * (len(SECTIONS) + 6)]
    summary = []
    for key in order:
        g = sorted(groups[key], key=lambda r: r["round"])
        cells = []
        rec = {"view": key[0], "mode": key[1], "rounds": [r["round"] for r in g]}
        for s in SECTIONS:
            vals = [r.get(f"gpu_{s}_ms") for r in g]
            m1, m2 = med(vals), mn(vals)
            rec[f"gpu_{s}_median"], rec[f"gpu_{s}_min"] = m1, m2
            cells.append("" if m1 is None else f"{m1:.2f} / {m2:.2f}")
        cpu = [r.get("cpu_frame_ms") for r in g]
        rec["cpu_frame_median"], rec["cpu_frame_min"] = med(cpu), mn(cpu)
        draws = med([r.get("draws") for r in g])
        objs = med([r.get("objects") for r in g])
        rec["draws_median"], rec["objects_median"] = draws, objs
        settles = ",".join(str(r.get("settle", "")) for r in g)
        rec["settle"] = settles
        mhz = med([r.get("gpu_mhz") for r in g])
        ps = sorted({p for r in g for p in r.get("pstates", [])})
        rec["gpu_mhz_median"], rec["pstates"] = mhz, ps
        rec["games_seen"] = sorted({x for r in g for x in r.get("games", [])})
        summary.append(rec)
        md.append("| " + " | ".join([key[0], key[1], str(len(g))] + cells +
                                    ["" if rec["cpu_frame_median"] is None else
                                     f"{rec['cpu_frame_median']:.2f} / {rec['cpu_frame_min']:.2f}",
                                     "" if draws is None else f"{draws:,.0f}",
                                     "" if objs is None else f"{objs:,.0f}",
                                     "" if mhz is None else f"{mhz:,.0f}", "/".join(ps), settles]) + " |")

    md += ["", "## Repeat noise between rounds (Remastered shots)", "",
           "Per pair of rounds of the same view on the same build: max channel difference / pixels over 2 /",
           "pixels over 8, per row band (sky 0-422, horizon 422-591, ground 591-1080, whole frame). capBegin",
           "pins the sim clock to the time each round starts, and the rounds are minutes apart, so sky and",
           "cloud drift, sea animation, fire and smoke differ between rounds. This shows how far one view",
           "moves inside one game; it is NOT the gate item 5 noise floor (that is a pinned same-view repeat).", "",
           "| view | pair | sky | horizon | ground | whole |", "|---|---|---|---|---|---|"]
    for base in dict.fromkeys(r["base"] for r in rows):
        g = [r for r in rows if r["base"] == base and r["mode"] == "remastered"]
        if not g:
            continue
        rounds = sorted(r["round"] for r in g)
        noise, shape = band_noise(g[0]["dir"], base, rounds)
        pairs = list(itertools.combinations(rounds, 2))
        for (i, j), cells in zip(pairs, noise):
            md.append(f"| {base} | r{i}-r{j} | " + " | ".join(f"{c[0]} / {c[1]:,} / {c[2]:,}" for c in cells) + " |")
    Path(a.out).write_text("\n".join(md) + "\n", encoding="utf-8")
    if a.json:
        Path(a.json).write_text(json.dumps(summary, indent=2), encoding="utf-8")
    print(f"rows: {len(rows)}; groups: {len(order)}; wrote {a.out}")


if __name__ == "__main__":
    main()
