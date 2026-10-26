"""Per-shot animation time from the agent 3 probes' event=dt lines.

tms    = (time - capSimT) * 1000 read right after the shot
pinned = triSetSimTime result for that shot ("OK:<abs ms>")
advance per shot = tms - (pinned - capSimT*1000): the sim time the shot got
on top of its pinned clock (exactly 1/60 s -> 17 ms after Time's ms rounding).
"""
import re, sys, statistics
log = sys.argv[1]
simT = float(sys.argv[2]) if len(sys.argv) > 2 else 2000.0
adv = []
for line in open(log, encoding="utf-8", errors="replace"):
    m = re.search(r"event=dt;tms=([-0-9.e]+);pinned=OK:(\d+);acc=([-0-9.e]+)", line)
    if m:
        tms = float(m.group(1)); pinned = int(m.group(2)) - simT * 1000
        adv.append(tms - pinned)
if not adv:
    print("no event=dt lines"); sys.exit(1)
r = [round(a) for a in adv]
print(f"shots={len(adv)} advance_ms min={min(adv):.2f} median={statistics.median(adv):.2f} max={max(adv):.2f}")
from collections import Counter
print("rounded histogram:", dict(sorted(Counter(r).items())))
bad = [i for i, a in enumerate(r) if a != 17]
print("shots not at 17 ms (1/60 s):", bad[:40], "count", len(bad))
