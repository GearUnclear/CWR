"""Agent 3: the air stage's GPU time per frame from a probe's game log (the gpu= field of the frame log and
the final view=gpu line). usage: python gpucost.py <run dir> [...]
Prints median / min of air ms, and air relative to ao+ssr+taa (a contention-normalised cost)."""
import glob, re, statistics, sys

for d in sys.argv[1:]:
    logs = glob.glob(d + "/tri/*/game_stdout.log")
    if not logs:
        print(d, "no log")
        continue
    txt = open(logs[0], encoding="utf-8", errors="replace").read()
    air, rel, frame = [], [], []
    for m in re.finditer(r"gpu=frame ([\d.]+) ms .*?\| post [\d.]+: (.*?)\|", txt):
        f = float(m.group(1))
        parts = dict(re.findall(r"(\w+) ([\d.]+)", m.group(2)))
        if "air" not in parts:
            continue
        a = float(parts["air"])
        ref = sum(float(parts.get(k, 0)) for k in ("ao", "ssr", "taa"))
        air.append(a)
        frame.append(f)
        if ref > 0:
            rel.append(a / ref)
    last = re.findall(r"view=gpu;event=gpu;t=(.*?)\"", txt)
    if air:
        print(f"{d}: n={len(air)} air median {statistics.median(air):.2f} min {min(air):.2f} ms; "
              f"air/(ao+ssr+taa) median {statistics.median(rel):.2f}; frame median {statistics.median(frame):.1f}")
    if last:
        print("   final:", last[-1][:400])
