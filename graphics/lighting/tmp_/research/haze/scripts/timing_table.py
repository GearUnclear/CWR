"""Summarise the interleaved timing runs (labels <view>__t<round>_<mode>).

python timing_table.py <game_stdout.log> [...]  -> markdown table on stdout

The machine shares one GPU between up to four games, so each reading includes
other games' work. Per mode: min (least contended), median, and the per-round
difference against v0 measured in the same round (paired), whose median is the
most robust estimate of the added cost.
"""
import re
import statistics
import sys
from collections import defaultdict

FRAME_RE = re.compile(r"\bframe ([\d.]+) ms")
WORLD_RE = re.compile(r"\bworld ([\d.]+)")
rows = defaultdict(dict)  # (view, mode) -> {round: (frame, world)}
for path in sys.argv[1:]:
    for line in open(path, encoding="utf-8", errors="replace"):
        m = re.search(r"UDCAP;view=([^;]+);event=shot;label=([^;]+);.*;gpu=(.*)$", line)
        if not m:
            continue
        lm = re.match(r"(.+)__t(\d+)_(.+)$", m.group(2))
        if not lm:
            continue
        g = m.group(3)
        fr, wr = FRAME_RE.search(g), WORLD_RE.search(g)
        if not fr:
            continue
        rows[(lm.group(1), lm.group(3))][int(lm.group(2))] = (float(fr.group(1)), float(wr.group(1)) if wr else None)

views = sorted({v for v, _ in rows})
print("| view | mode | rounds | frame min | frame median | world min | world median | paired frame delta vs v0 (median) | paired world delta vs v0 (median) |")
print("|---|---|---|---|---|---|---|---|---|")
for v in views:
    modes = [m for (vv, m) in rows if vv == v]
    order = sorted(modes, key=lambda m: (m != "classic", m != "v0", m))
    base = rows.get((v, "v0"), {})
    for m in order:
        r = rows[(v, m)]
        fr = [x[0] for x in r.values()]
        wr = [x[1] for x in r.values() if x[1] is not None]
        pd = [r[k][0] - base[k][0] for k in r if k in base]
        pw = [r[k][1] - base[k][1] for k in r if k in base and r[k][1] is not None and base[k][1] is not None]
        print(f"| {v} | {m} | {len(fr)} | {min(fr):.2f} | {statistics.median(fr):.2f} | "
              f"{min(wr):.2f} | {statistics.median(wr):.2f} | "
              f"{statistics.median(pd):+.2f} | {statistics.median(pw):+.2f} |" if wr else
              f"| {v} | {m} | {len(fr)} | {min(fr):.2f} | {statistics.median(fr):.2f} | - | - | - | - |")
