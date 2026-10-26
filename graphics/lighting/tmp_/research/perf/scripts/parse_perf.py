"""Parse UDPERF lines from game_stdout.log into a table (markdown + csv).

usage: parse_perf.py <game_stdout.log> [<out_prefix>]
"""
import csv
import re
import sys
from pathlib import Path

log = Path(sys.argv[1])
prefix = Path(sys.argv[2]) if len(sys.argv) > 2 else None

ANSI = re.compile(r"\x1b\[[0-9;]*m")
rows = []
for raw in log.read_text(errors="replace").splitlines():
    line = ANSI.sub("", raw)
    i = line.find("UDPERF;view=")
    if i < 0:
        continue
    body = line[i:].rstrip('"').rstrip()
    gpu = body.split(";gpu=", 1)[1] if ";gpu=" in body else ""
    head = body.split(";gpu=", 1)[0]
    kv = {}
    for part in head.split(";")[1:]:
        if "=" in part:
            k, v = part.split("=", 1)
            kv[k] = v
    perf = kv.get("perf", "")
    pm = {}
    for tok in perf.split():
        if "=" in tok:
            k, v = tok.split("=", 1)
            pm[k] = v.split("/")[0]
    g = {}
    m = re.search(r"frame ([\d.]+) ms", gpu)
    g["gpu_frame"] = m.group(1) if m else ""
    m = re.search(r"world ([\d.]+)", gpu)
    g["gpu_world"] = m.group(1) if m else ""
    m = re.search(r"shadow ([\d.]+)", gpu)
    g["gpu_shadow"] = m.group(1) if m else ""
    m = re.search(r"post ([\d.]+)", gpu)
    g["gpu_post"] = m.group(1) if m else ""
    for k in ("ao", "bloom", "shafts", "composite"):
        m = re.search(k + r" ([\d.]+)", gpu)
        g["gpu_" + k] = m.group(1) if m else ""
    ts = line[1:24] if line.startswith("[") else ""
    rows.append({
        "time": ts,
        "view": kv.get("view", ""),
        "cfg": kv.get("cfg", ""),
        "args": kv.get("args", ""),
        "lit": kv.get("lit", ""),
        "sat": kv.get("sat", ""),
        **g,
        "cpu_fps": pm.get("fps", ""),
        "cpu_frame": pm.get("frame", ""),
        "cpu_draw": pm.get("draw", ""),
        "land_gnd": pm.get("land:gnd", ""),
        "land_obj": pm.get("land:obj", ""),
        "drw_land": pm.get("drw:land", ""),
        "drw_obj": pm.get("drw:obj", ""),
        "calls": pm.get("calls", ""),
    })

cols = ["time", "view", "cfg", "gpu_frame", "gpu_world", "gpu_shadow", "gpu_post", "gpu_ao", "cpu_fps", "cpu_frame",
        "cpu_draw", "land_obj", "drw_land", "calls", "lit", "sat", "args"]
md = ["| " + " | ".join(cols) + " |", "|" + "---|" * len(cols)]
for r in rows:
    md.append("| " + " | ".join(str(r.get(c, "")) for c in cols) + " |")
text = "\n".join(md)
print(text)
if prefix:
    prefix.with_suffix(".md").write_text(text + "\n")
    with prefix.with_suffix(".csv").open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0].keys()) if rows else cols)
        w.writeheader()
        w.writerows(rows)
