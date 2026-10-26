"""Summarise the B3 timing run: per view, off and on over the interleaved
rounds (median and minimum of each GPU section, the triGpuTimes averages of
90 frames), and the foreign games and GPU clocks the sampler saw.

    python timing_b3.py <run dir> <sampler csv> > timing.md
"""
import csv
import re
import statistics
import sys
from collections import defaultdict
from pathlib import Path

run, sampler = Path(sys.argv[1]), Path(sys.argv[2])
shots = defaultdict(lambda: defaultdict(list))
times = {}
for line in (run / "udcap.log").read_text(encoding="utf-8", errors="replace").splitlines():
    if "event=shot" not in line:
        continue
    label = re.search(r"label=([^;]+)", line).group(1)
    view, mode = label.split("__")
    mode = re.sub(r"\d+$", "", mode)
    gpu = line.split("gpu=", 1)[1]
    vals = {"frame": re.search(r"frame ([\d.]+) ms", gpu), "world": re.search(r"world ([\d.]+)", gpu),
            "pass1": re.search(r"pass1 ([\d.]+)", gpu), "shadow": re.search(r"shadow ([\d.]+)", gpu),
            "post": re.search(r"post ([\d.]+)", gpu)}
    for k, m in vals.items():
        shots[view][mode].append((k, float(m.group(1)) if m else 0.0))
    t = re.match(r"^(\d\d:\d\d:\d\d)", line)
rows = list(csv.DictReader(sampler.open(encoding="ascii")))
print("| view | section | off median | on median | on - off (median) | off min | on min | on - off (min) |")
print("|---|---|---|---|---|---|---|---|")
for view, modes in shots.items():
    for sec in ("frame", "world", "pass1", "shadow", "post"):
        off = [v for k, v in modes["off"] if k == sec]
        on = [v for k, v in modes["on"] if k == sec]
        if not off or not on:
            continue
        mo, mn = statistics.median(off), statistics.median(on)
        print(f"| {view} | {sec} | {mo:.2f} | {mn:.2f} | {mn - mo:+.2f} | {min(off):.2f} | {min(on):.2f} | "
              f"{min(on) - min(off):+.2f} |")
clocks = [int(r["clock_gr"]) for r in rows if r["clock_gr"].isdigit()]
foreign = sorted({r["poseidon_games"] for r in rows})
print()
print(f"sampler: {len(rows)} rows, games seen {foreign}, graphics clock median {statistics.median(clocks)} MHz, "
      f"min {min(clocks)}, max {max(clocks)}, P-states {sorted({r['pstate'] for r in rows})}")
