"""A5 lane: the cost split table from a cost run's udcap.log (gen_cost.py).

    python cost_table.py <run folder>

Per view and variant, the median and minimum over the rounds of the GPU frame,
world, post, taa and sharpen sections (ms), and the median CPU fps.
"""
import re
import statistics
import sys
from collections import defaultdict
from pathlib import Path


def sections(gpu):
    out = {}
    m = re.search(r"frame ([\d.]+) ms", gpu)
    if m:
        out["frame"] = float(m.group(1))
    m = re.search(r"world ([\d.]+)", gpu)
    if m:
        out["world"] = float(m.group(1))
    m = re.search(r"post ([\d.]+):", gpu)
    if m:
        out["post"] = float(m.group(1))
    for name in ("taa", "sharpen", "resolve"):
        m = re.search(rf"\b{name} ([\d.]+)", gpu)
        if m:
            out[name] = float(m.group(1))
    return out


def main():
    run = Path(sys.argv[1])
    data = defaultdict(lambda: defaultdict(list))
    for line in (run / "udcap.log").read_text(errors="replace").splitlines():
        if "event=shot" not in line:
            continue
        label = re.search(r"label=([^;]+)", line).group(1)
        m = re.match(r"(\w+?)__r(\d)_(\w+)", label)
        if not m:
            continue
        view, variant = m.group(1), m.group(3)
        gpu = re.search(r"gpu=([^;\"]*)", line)
        fps = re.search(r"fps=([\d.]+)", line)
        s = sections(gpu.group(1) if gpu else "")
        if fps:
            s["fps"] = float(fps.group(1))
        for k, v in s.items():
            data[(view, variant)][k].append(v)
    cols = ["frame", "world", "post", "resolve", "taa", "sharpen"]
    print("| view | variant | " + " | ".join(f"{c} med / min" for c in cols) + " | fps med | n |")
    print("|---|---|" + "---|" * (len(cols) + 2))
    for (view, variant), d in sorted(data.items(), key=lambda kv: (kv[0][0], ["off", "on", "mip0", "nomask"].index(kv[0][1]) if kv[0][1] in ["off", "on", "mip0", "nomask"] else 9)):
        cells = []
        for c in cols:
            v = d.get(c, [])
            cells.append(f"{statistics.median(v):.2f} / {min(v):.2f}" if v else "")
        fps = d.get("fps", [])
        n = len(d.get("frame", []))
        print(f"| {view} | {variant} | " + " | ".join(cells) + f" | {statistics.median(fps):.1f} | {n} |" if fps else " |")


main()
