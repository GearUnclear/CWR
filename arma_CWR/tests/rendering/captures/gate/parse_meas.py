"""Parse UDCAP event=meas lines from run dirs' game logs into CSV + markdown (legacy mode).

    python -I parse_meas.py <run dir> [<run dir> ...] --out <prefix> [--quiet]

Moved from tmp/perf-audit/measure2/parse_meas.py (round 1 and M2 used the same file) on
2026-10-31; only argv paths, nothing machine-specific. Writes <prefix>.csv and <prefix>.md.

One row per event=meas line (one 256-frame window): the CPU frame avg/p95/max and every
profiler phase from triPerfStats (`perf=`), the scene stats, the counters after `||`, and the
GPU sections of triGpuTimes (`gpu=`). Every row carries source=ema: triGpuTimes is an
exponential average with weight 0.1, and triPerfStats is a per-window mean, so these numbers
must never be compared with ring (per-frame median) numbers (PERF-PLAN 3.1 P0-3).
"""
import argparse
import csv
import os
import re
from pathlib import Path

PHASES = ["setup", "draw", "drw:init", "drw:prep", "land:gnd", "land:obj", "drw:land", "drw:obj", "drw:post", "hud",
          "ai+veh", "sound", "swap", "gm"]
GPU = ["frame", "world", "shadow", "ground", "pass1", "rest", "sky", "skytable", "canopy", "combatmask", "post",
       "resolve", "lindepth", "tags", "ao", "ssr", "air", "bloom", "shafts", "scatter", "composite", "taa", "temporal",
       "sharpen"]
KEYS = ["run", "label", "draws", "objects", "frames", "cpu_avg", "cpu_p95", "cpu_max", "land:gnd", "land:obj",
        "drw:land", "drw:obj", "drw:post", "swap", "hud", "ai+veh", "gpu_frame", "gmin_frame", "gpu_world",
        "gpu_shadow", "gpu_ground", "gpu_pass1", "gpu_rest", "gpu_post"]


def parse_line(line):
    i = line.find("UDCAP;")
    if i < 0 or "event=meas" not in line:
        return None
    body = line[i + 6:].rstrip().rstrip('"')
    ts = re.match(r"^\[([^\]]+)\]", line)
    head, _, gpu = body.partition(";gpu=")
    head, _, perf = head.partition(";perf=")
    d = {"time": ts.group(1) if ts else "", "source": "ema"}
    for kv in head.split(";"):
        if "=" in kv:
            k, v = kv.split("=", 1)
            d[k] = v
    pm = re.match(r"fps=([\d.]+) frame=([\d.]+)/([\d.]+)/([\d.]+)", perf)
    if pm:
        d["cpu_fps"], d["cpu_avg"], d["cpu_p95"], d["cpu_max"] = pm.groups()
    for name, a, p in re.findall(r"(?:^| )([a-z:+]+)=([\d.]+)/([\d.]+)", perf.split(" || ")[0]):
        if name in PHASES:
            d[name] = a
            d[name + "_p95"] = p
    cm = re.search(r"calls=(\d+) frames=(\d+)", perf)
    if cm:
        d["calls"], d["frames"] = cm.groups()
    if " || " in perf:
        d["counters"] = perf.split(" || ", 1)[1].strip()
    g, _, gmin = gpu.partition("| min")
    for name, ms in re.findall(r"([a-z0-9]+) (\d+\.\d+)", g):
        if name in GPU and "gpu_" + name not in d:
            d["gpu_" + name] = ms
    for name, ms in re.findall(r"([a-z0-9]+)=(\d+\.\d+)", gmin):
        d["gmin_" + name] = ms
    d["gpu_raw"] = gpu.strip().strip('"')
    return d


def logs_of(rd):
    rd = Path(rd)
    return list(rd.rglob("game_stdout.log")) or list(rd.rglob("udcap.log"))


def collect(run_dirs):
    rows = []
    for rd in run_dirs:
        rd = Path(rd)
        seen = set()
        for lp in logs_of(rd):
            for line in lp.read_text(encoding="utf-8", errors="replace").splitlines():
                d = parse_line(line)
                if d and (d.get("label"), d.get("time")) not in seen:
                    seen.add((d.get("label"), d.get("time")))
                    d["run"] = rd.name
                    d["log"] = str(lp)
                    rows.append(d)
    return rows


def write(rows, out, quiet=False):
    allk = KEYS + sorted({k for r in rows for k in r} - set(KEYS) - {"gpu_raw", "counters"}) + ["counters", "gpu_raw"]
    os.makedirs(os.path.dirname(os.path.abspath(out)) or ".", exist_ok=True)
    with open(out + ".csv", "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=allk, extrasaction="ignore")
        w.writeheader()
        w.writerows(rows)
    with open(out + ".md", "w", encoding="utf-8", newline="\n") as f:
        f.write("| " + " | ".join(KEYS[1:]) + " |\n|" + "---|" * (len(KEYS) - 1) + "\n")
        for r in rows:
            f.write("| " + " | ".join(str(r.get(k, "")) for k in KEYS[1:]) + " |\n")
    if not quiet:
        print(open(out + ".md", encoding="utf-8").read())


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("runs", nargs="+")
    ap.add_argument("--out", required=True)
    ap.add_argument("--quiet", action="store_true")
    a = ap.parse_args()
    write(collect(a.runs), a.out, a.quiet)


if __name__ == "__main__":
    main()
