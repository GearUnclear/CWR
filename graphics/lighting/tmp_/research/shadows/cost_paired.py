"""Paired GPU cost: for each alternation k, variant minus the 'rem' shot taken
in the same alternation on the same view; report the median of those deltas.

    python cost_paired.py <run_dir>
"""
import json
import re
import statistics
import sys
from pathlib import Path

run = Path(sys.argv[1])
t = json.loads((run / "timings.json").read_text())
by = {}
for lab, r in t.items():
    m = re.match(r"(.+?)__(.+)_(\d+)$", lab)
    if m:
        by[(m.group(1), m.group(2), int(m.group(3)))] = r
views = sorted({k[0] for k in by})
variants = sorted({k[1] for k in by})
lines = ["| view | variant | n | median frame (ms) | median paired delta vs rem: frame | world | shadow pass | composite |",
         "|---|---|---|---|---|---|---|---|"]
for v in views:
    for var in variants:
        ks = sorted(k[2] for k in by if k[0] == v and k[1] == var)
        if not ks:
            continue
        d = {f: [] for f in ("frame", "world", "shadow", "composite")}
        fr = []
        for k in ks:
            a = by[(v, var, k)]
            b = by.get((v, "rem", k))
            fr.append(a["frame"])
            if not b:
                continue
            for f in d:
                if a.get(f) is not None and b.get(f) is not None:
                    d[f].append(a[f] - b[f])
        med = lambda xs: ("%+.2f" % statistics.median(xs)) if xs else ""
        lines.append(f"| {v} | {var} | {len(ks)} | {statistics.median(fr):.2f} | {med(d['frame'])} | {med(d['world'])} | {med(d['shadow'])} | {med(d['composite'])} |")
out = "\n".join(lines) + "\n"
(run / "cost_paired.md").write_text(out)
print(out)
