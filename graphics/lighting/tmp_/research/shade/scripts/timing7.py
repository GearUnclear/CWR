"""Interleaved GPU timing (runs 7a/7b): per view and mode, the GPU frame / world /
post ms of each round, then the median and the delta to Classic and to V1.

    python timing7.py <run_dir> [<run_dir> ...]
"""
import re
import statistics
import sys
from collections import defaultdict
from pathlib import Path

ANSI = re.compile(r"\x1b\[[0-9;]*m")


def num(rx, line):
    m = re.search(rx, line)
    return float(m.group(1)) if m else None


def main():
    for run in sys.argv[1:]:
        data = defaultdict(lambda: defaultdict(list))
        for line in Path(run, "udcap.log").read_text(encoding="utf-8", errors="replace").splitlines():
            line = ANSI.sub("", line)
            if "event=shot" not in line:
                continue
            v = re.search(r"view=([^;]+)", line).group(1)
            lab = re.search(r"label=([^;]+)", line).group(1).split("__", 1)[1]
            m = re.match(r"t(\d)(\w+)", lab)
            if not m:
                continue
            mode = m.group(2)
            data[v][mode].append((num(r"gpu=frame ([\d.]+) ms", line), num(r"world ([\d.]+)", line),
                                  num(r"post ([\d.]+):", line), num(r"perf=fps=([\d.]+)", line)))
        print(f"\n### {run}\n")
        print("| view | mode | GPU frame ms (rounds) | median frame | median world | median post | delta vs classic | delta vs v1 |")
        print("|---|---|---|---|---|---|---|---|")
        for v, modes in data.items():
            med = {k: statistics.median([r[0] for r in rows if r[0] is not None]) for k, rows in modes.items() if any(r[0] for r in rows)}
            for mode, rows in modes.items():
                fr = [r[0] for r in rows if r[0] is not None]
                wo = [r[1] for r in rows if r[1] is not None]
                po = [r[2] for r in rows if r[2] is not None]
                mf = statistics.median(fr) if fr else float("nan")
                mw = statistics.median(wo) if wo else float("nan")
                mp = statistics.median(po) if po else float("nan")
                dc = mf - med.get("cl", float("nan"))
                d1 = mf - med.get("v1", float("nan"))
                print(f"| {v} | {mode} | {', '.join(f'{x:.2f}' for x in fr)} | {mf:.2f} | {mw:.2f} | {mp:.2f} | {dc:+.2f} | {d1:+.2f} |")


if __name__ == "__main__":
    main()
