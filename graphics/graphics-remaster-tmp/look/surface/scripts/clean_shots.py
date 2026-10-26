"""Which shots of a timing run had the GPU to themselves.

  python clean_shots.py <run dir> <procs.csv> [--window 100]

procs.csv: the watcher's log (time HH:MM:SS, count of other lanes' games).
A shot is clean when every watcher sample from `window` seconds before its
PNG was written (the timers average the 90 frames before the shot, the
settle before them) up to the PNG itself saw no other game. Prints one line
per shot: label, png time, clean, and the timing of the shot.
"""
import argparse
import csv
import datetime as dt
import re
from pathlib import Path

PAT = re.compile(r"^\d+_(?P<label>.+)\.png$")
SHOT = re.compile(r'event=shot;label=(?P<label>[^;]+);.*;gpu=frame (?P<frame>[\d.]+) ms \| world (?P<world>[\d.]+)')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("procs")
    ap.add_argument("--window", type=float, default=100.0)
    a = ap.parse_args()
    samples = []
    with open(a.procs, encoding="utf-8-sig") as f:
        for row in csv.DictReader(f):
            t = dt.datetime.strptime(row["time"], "%H:%M:%S").time()
            samples.append((t.hour * 3600 + t.minute * 60 + t.second, int(row["count_foreign"])))
    timing = {}
    for log in Path(a.run).rglob("game_stdout.log"):
        for line in log.read_text(encoding="utf-8", errors="replace").splitlines():
            m = SHOT.search(line)
            if m:
                timing[m.group("label")] = (float(m.group("frame")), float(m.group("world")))
    for p in sorted(Path(a.run).rglob("*.png"), key=lambda q: q.stat().st_mtime):
        m = PAT.match(p.name)
        if not m:
            continue
        t = dt.datetime.fromtimestamp(p.stat().st_mtime)
        s = t.hour * 3600 + t.minute * 60 + t.second
        near = [c for (ts, c) in samples if s - a.window <= ts <= s]
        clean = bool(near) and max(near) == 0
        fr = timing.get(m.group("label"), (None, None))
        print(f"{m.group('label')}\t{t:%H:%M:%S}\t{'clean' if clean else 'LOADED'}\t{fr[0]}\t{fr[1]}")


if __name__ == "__main__":
    main()
