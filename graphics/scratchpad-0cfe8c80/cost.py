"""cost.py RUN_DIR OUT.md : per-view GPU cost, tip vs overhaul, from a look_timing run's udcap.log."""
import re
import sys
from collections import defaultdict
from pathlib import Path

run = Path(sys.argv[1])
rows = defaultdict(lambda: defaultdict(list))
order = []
for line in (run / "udcap.log").read_text(encoding="utf-8", errors="replace").splitlines():
    m = re.search(r"label=([^;]+)__(tip|ov)\d;", line)
    g = re.search(r"gpu=frame ([\d.]+) ms \| world ([\d.]+).*?\| post ([\d.]+):(.*?)\|", line)
    if not m or not g:
        continue
    view, mode = m.group(1), m.group(2)
    if view not in order:
        order.append(view)
    d = rows[view][mode]
    stages = dict(re.findall(r"(\w+) ([\d.]+)", g.group(4)))
    d.append({"frame": float(g.group(1)), "world": float(g.group(2)), "post": float(g.group(3)),
              **{k: float(v) for k, v in stages.items()}})
mean = lambda xs, k: sum(x.get(k, 0.0) for x in xs) / max(len(xs), 1)
samples = run / "gpu_samples.csv"
foreign = "not logged"
if samples.exists():
    lines = samples.read_text().splitlines()[1:]
    games = [l.split(",")[7] for l in lines if l.count(",") >= 7]
    nf = sum(1 for gm in games if not gm.endswith("foreign0"))
    foreign = f"{nf} of {len(games)} two-second samples had a foreign PoseidonGame running"
out = ["| view | GPU frame tip -> overhaul (ms) | delta | 3060 est. overhaul | world delta | post delta | ao | air | composite |",
       "|---|---|---|---|---|---|---|---|---|"]
for v in order:
    t, o = rows[v]["tip"], rows[v]["ov"]
    if not t or not o:
        continue
    f0, f1 = mean(t, "frame"), mean(o, "frame")
    out.append(f"| {v} | {f0:.2f} -> {f1:.2f} | {f1 - f0:+.2f} | {f1 * 1.4:.1f} | {mean(o, 'world') - mean(t, 'world'):+.2f} | "
               f"{mean(o, 'post') - mean(t, 'post'):+.2f} | {mean(t, 'ao'):.2f} -> {mean(o, 'ao'):.2f} | "
               f"{mean(t, 'air'):.2f} -> {mean(o, 'air'):.2f} | {mean(t, 'composite'):.2f} -> {mean(o, 'composite'):.2f} |")
text = "\n".join(out) + f"\n\nForeign GPU load during the run: {foreign}.\n"
Path(sys.argv[2]).write_text(text, encoding="utf-8")
print(text)
