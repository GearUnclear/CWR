"""timing.py <run dir>: per view, the GPU frame and the air section of every base/fog shot (triGpuTimes)."""
import sys, glob, re, statistics
log = glob.glob(sys.argv[1] + "/tri/*/game_stdout.log")
rows = {}
for f in log:
    for line in open(f, encoding="utf-8", errors="replace"):
        m = re.search(r"label=(\w+?)__(\w+);.*gpu=frame ([0-9.]+) ms.*?(?:air ([0-9.]+))?", line)
        if not m or "event=shot" not in line:
            continue
        air = re.search(r"\bair ([0-9.]+)", line)
        view, lab = m.group(1), m.group(2)
        kind = "fog" if lab.startswith("fog") else ("base" if lab.startswith("base") else lab)
        rows.setdefault(view, {}).setdefault(kind, []).append((float(m.group(3)), float(air.group(1)) if air else 0.0))
print("| view | base frame ms (each round) | fog frame ms (each round) | air ms (each round) | frame median fog - base | air median |")
print("|---|---|---|---|---|---|")
for v, d in rows.items():
    b = [x[0] for x in d.get("base", [])]
    fz = [x[0] for x in d.get("fog", [])]
    a = [x[1] for x in d.get("fog", [])]
    if not b or not fz:
        continue
    print(f"| {v} | {', '.join(f'{x:.2f}' for x in b)} | {', '.join(f'{x:.2f}' for x in fz)} | "
          f"{', '.join(f'{x:.2f}' for x in a)} | {statistics.median(fz) - statistics.median(b):+.2f} | {statistics.median(a):.2f} |")
