"""Agent 4: per-shot camera readout from a capture log (udcap.log).

usage: python parse_cam.py <udcap.log> <out.csv>
Columns: k, frame (engine render counter), dframe, ev, target, pre, mean, high,
skytop, ring, ringw, free, reads.
"""
import re, sys

src, out = sys.argv[1], sys.argv[2]
rows = []
for line in open(src, encoding="utf-8", errors="replace"):
    if "event=frame;" not in line or "cam ev=" not in line:
        continue
    k = int(re.search(r";k=(\d+);", line).group(1))
    fr = int(re.search(r" frame=(\d+) ", line).group(1))
    cam = re.search(r"cam (ev=[^;\"]*)", line).group(1)
    d = dict(kv.split("=") for kv in cam.split() if "=" in kv)
    rows.append((k, fr, d))
rows.sort()
keys = ["ev", "target", "pre", "mean", "high", "skytop", "ring", "ringw", "free", "reads"]
with open(out, "w") as f:
    f.write("k,frame,dframe," + ",".join(keys) + "\n")
    prev = None
    for k, fr, d in rows:
        f.write("%d,%d,%d,%s\n" % (k, fr, 0 if prev is None else fr - prev, ",".join(d.get(x, "") for x in keys)))
        prev = fr
print(len(rows), "rows ->", out)
