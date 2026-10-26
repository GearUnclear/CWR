"""GPU sections per timing shot of b10_timing (UDCAP gpu= field), off against
on per view and round, plus medians.

    python timing_table.py <game_stdout.log> [--md out.md]
"""
import argparse
import re
import statistics
from collections import defaultdict
from pathlib import Path

ap = argparse.ArgumentParser()
ap.add_argument("log")
ap.add_argument("--md", default="")
a = ap.parse_args()

SECTIONS = ("frame", "world", "ground", "sky", "post")
rows = defaultdict(dict)
for line in Path(a.log).read_text(encoding="utf-8", errors="replace").splitlines():
    if "event=shot" not in line:
        continue
    m = re.search(r"label=(\w+)__r(\d)_(off|on);", line)
    g = re.search(r";gpu=(.*?)\"?$", line)
    if not m or not g:
        continue
    gpu = g.group(1)
    vals = {}
    vals["frame"] = re.search(r"frame ([\d.]+) ms", gpu)
    vals["world"] = re.search(r"\| world ([\d.]+)", gpu)
    vals["ground"] = re.search(r"\[ground ([\d.]+)", gpu)
    vals["sky"] = re.search(r"\| sky ([\d.]+)", gpu)
    vals["post"] = re.search(r"\| post ([\d.]+)", gpu)
    mins = re.search(r"min frame=([\d.]+)", gpu)
    out = {k: float(v.group(1)) if v else float("nan") for k, v in vals.items()}
    out["minframe"] = float(mins.group(1)) if mins else float("nan")
    rows[(m.group(1), int(m.group(2)))][m.group(3)] = out

lines = ["| view | round | frame off / on | world off / on | ground off / on | sky off / on | post off / on | min frame off / on |",
         "|---|---|---|---|---|---|---|---|"]
delta = defaultdict(lambda: defaultdict(list))
for (view, r), d in sorted(rows.items()):
    if "off" not in d or "on" not in d:
        continue
    cells = [view, str(r)]
    for k in SECTIONS + ("minframe",):
        cells.append(f"{d['off'][k]:.2f} / {d['on'][k]:.2f}")
        delta[view][k].append(d["on"][k] - d["off"][k])
    lines.append("| " + " | ".join(cells) + " |")
lines.append("")
lines.append("| view | median on - off: frame | world | ground | sky | post | min frame |")
lines.append("|---|---|---|---|---|---|---|")
for view, d in delta.items():
    lines.append("| " + view + " | " + " | ".join(f"{statistics.median(d[k]):+.2f}" for k in SECTIONS + ("minframe",)) + " |")
text = "\n".join(lines)
print(text)
if a.md:
    Path(a.md).write_text(text + "\n", encoding="utf-8")
