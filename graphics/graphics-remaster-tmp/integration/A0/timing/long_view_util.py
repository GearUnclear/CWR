"""nvidia-smi rows around the A0 timing shots.

    python long_view_util.py <run dir>

Part 1: utilisation, board power and P-states in several windows before the
long-view shots (sunrise_ridge, valley_0515, broken_high). Part 2: per shot,
the rows in the -3..+0 s window, the window summarize_timing.py uses for its
GPU MHz column (rows stamped from 3 s before the shot up to the shot; the
stamps are whole seconds, so the edges are good to about a second), for the
long views and the sky and sea views (coastline_sunset, coast_oc_up, sea_noon).
"""
import csv
import datetime as dt
import re
import statistics as st
import sys

run = sys.argv[1]
SHOT = re.compile(r"^\[(\d{4}-\d\d-\d\d \d\d:\d\d:\d\d\.\d+)\].*UDCAP;view=[^;]*;event=shot;label=([^;\"]+)")
shots = {}
for line in open(run + "/udcap.log", encoding="utf-8", errors="replace"):
    m = SHOT.match(line)
    if m:
        shots[m.group(2)] = dt.datetime.strptime(m.group(1)[:23], "%Y-%m-%d %H:%M:%S.%f")
day = min(shots.values()).date()
S = []
for r in csv.DictReader(open(run + "/gpu_samples.csv")):
    t = dt.datetime.combine(day, dt.datetime.strptime(r["time"], "%H:%M:%S").time())
    S.append((t, r))


def rows(t, a, b):
    return [r for (ts, r) in S if t - dt.timedelta(seconds=a) <= ts <= t + dt.timedelta(seconds=b)]


LONG = ["sunrise_ridge", "valley_0515", "broken_high"]
SKYSEA = ["coastline_sunset", "coast_oc_up", "sea_noon"]
print("# part 1: long views (sunrise_ridge, valley_0515, broken_high), windows before each shot")
for (a, b) in [(3, 0), (3, 0.5), (4, 0), (5, 0), (6, 0)]:
    for mode in ("classic", "remastered", ""):
        u, p, ps = [], [], set()
        for lab, t in shots.items():
            if not any(lab.startswith(v + "_r") for v in LONG) or not lab.endswith(mode):
                continue
            for r in rows(t, a, b):
                u.append(int(r["util_gpu"]))
                p.append(float(r["power_w"]))
                ps.add(r["pstate"])
        print(f"window -{a}..+{b} mode={mode or 'both'}: n={len(u)} util {min(u)}-{max(u)} "
              f"power {min(p):.1f}-{max(p):.1f} pstates {sorted(ps)}")

for title, views in (("long views", LONG), ("sky and sea views", SKYSEA)):
    print()
    print(f"# part 2: {title}, per shot, window -3..+0 s (summarize_timing.py GPU MHz): "
          "(util %, W, P-state, MHz)")
    for v in views:
        for mode in ("classic", "remastered"):
            meds = []
            for lab in sorted(l for l in shots if l.startswith(v + "_r") and l.endswith("__" + mode)):
                w = rows(shots[lab], 3, 0)
                print(lab, [(r["util_gpu"], r["power_w"], r["pstate"], r["clock_gr"]) for r in w])
                if w:
                    meds.append(st.median(float(r["clock_gr"]) for r in w))
            if meds:
                print(f"  {v} {mode}: per-round medians {[round(m) for m in meds]}, "
                      f"median over rounds {st.median(meds):,.0f} MHz")
