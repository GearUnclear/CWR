"""GPU timings of the timed shots (classic, remastered default, timed variants) per run."""
import re, sys
from pathlib import Path
ANSI = re.compile(r"\x1b\[[0-9;]*m")
for run in sys.argv[1:]:
    log = Path(run) / "udcap.log"
    print(f"\n### {run}\n\n| view | label | GPU frame ms | world | post | shafts | composite | CPU fps |\n|---|---|---|---|---|---|---|---|")
    for line in log.read_text(encoding="utf-8", errors="replace").splitlines():
        line = ANSI.sub("", line)
        if "event=shot" not in line or "gpu=frame" not in line:
            continue
        v = re.search(r"view=([^;]+)", line).group(1)
        lab = re.search(r"label=([^;]+)", line).group(1).split("__", 1)[1]
        g = lambda k: (re.search(k, line).group(1) if re.search(k, line) else "")
        print(f"| {v} | {lab} | {g(r'gpu=frame ([\d.]+) ms')} | {g(r'world ([\d.]+)')} | {g(r'post ([\d.]+):')} | {g(r'shafts ([\d.]+)')} | {g(r'composite ([\d.]+)')} | {g(r'perf=fps=([\d.]+)')} |")
