"""GPU and CPU timing from the shots that had the GPU to themselves.

  python clean_timing.py <procs.csv> <run dir> [<run dir> ...] [--window 30] [--md out.md]

A shot is clean when the watcher (procs.csv: time, count of other lanes'
PoseidonGame processes, every 5 s) saw no other game from `window` seconds
before its PNG up to the PNG. Per view and mode family (the label suffix
without its round digits): the number of clean shots and the median of the
GPU frame, world, ao stage, composite, the minimum frame, and the CPU frame and
drw:obj (ms).
"""
import argparse
import csv
import datetime as dt
import re
import statistics
from collections import defaultdict
from pathlib import Path

PNG = re.compile(r"^\d+_(?P<label>.+)\.png$")
SHOT = re.compile(r"event=shot;label=(?P<label>[^;]+);")


def num(pat, s):
    m = re.search(pat, s)
    return float(m.group(1)) if m else None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("procs")
    ap.add_argument("runs", nargs="+")
    ap.add_argument("--window", type=float, default=30.0)
    ap.add_argument("--md")
    a = ap.parse_args()
    samples = []
    with open(a.procs, encoding="utf-8-sig") as f:
        for row in csv.DictReader(f):
            t = dt.datetime.strptime(row["time"], "%H:%M:%S").time()
            samples.append((t.hour * 3600 + t.minute * 60 + t.second, int(row["count_foreign"])))
    fam = defaultdict(list)
    for run in a.runs:
        for log in Path(run).rglob("game_stdout.log"):
            pngs = {}
            for p in log.parent.rglob("*.png"):
                m = PNG.match(p.name)
                if m:
                    pngs[m.group("label")] = p
            for line in log.read_text(encoding="utf-8", errors="replace").splitlines():
                m = SHOT.search(line)
                if not m or m.group("label") not in pngs:
                    continue
                label = m.group("label")
                t = dt.datetime.fromtimestamp(pngs[label].stat().st_mtime)
                s = t.hour * 3600 + t.minute * 60 + t.second
                near = [c for (ts, c) in samples if s - a.window <= ts <= s]
                if not near or max(near) != 0:
                    continue
                g = line[line.find(";gpu=") + 5:]
                view, suf = label.split("__", 1)
                fam[(view, suf.rstrip("0123456789"))].append(dict(
                    frame=num(r"frame ([\d.]+) ms", g), world=num(r"world ([\d.]+)", g), ao=num(r" ao ([\d.]+)", g),
                    composite=num(r"composite ([\d.]+)", g), minframe=num(r"min frame=([\d.]+)", g),
                    cpu=num(r"perf=fps=[\d.]+ frame=([\d.]+)", line), drwobj=num(r"drw:obj=([\d.]+)", line)))
    out = ["| view | mode | clean shots | GPU frame | world | ao stage | composite | min frame | CPU frame | CPU drw:obj |",
           "|---|---|---|---|---|---|---|---|---|---|"]
    for (view, mode), rs in sorted(fam.items()):
        def med(k):
            v = [x[k] for x in rs if x[k] is not None]
            return f"{statistics.median(v):.2f}" if v else "-"
        out.append(f"| {view} | {mode} | {len(rs)} | {med('frame')} | {med('world')} | {med('ao')} | {med('composite')} | "
                   f"{med('minframe')} | {med('cpu')} | {med('drwobj')} |")
    text = "\n".join(out)
    print(text)
    if a.md:
        Path(a.md).write_text(text + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
