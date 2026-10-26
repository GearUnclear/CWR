"""Copy every NNN_<view>__<mode>.png out of a run's tri tree (make_pairs.py
keeps only its own mode names), newest wins, and print the camera readout of
each shot from udcap.log.

    python collect.py <run dir>
"""
import re
import shutil
import sys
from pathlib import Path

run = Path(sys.argv[1])
best = {}
for p in (run / "tri").rglob("*.png"):
    m = re.match(r"^(\d{3})_(.+)__([A-Za-z0-9_]+)\.png$", p.name)
    if not m:
        continue
    key = (m.group(2), m.group(3))
    if key not in best or p.stat().st_mtime > best[key].stat().st_mtime:
        best[key] = p
for (view, mode), src in best.items():
    shutil.copy2(src, run / f"{view}__{mode}.png")
print(f"copied {len(best)}")
log = run / "udcap.log"
if log.exists():
    for line in log.read_text(encoding="utf-8", errors="replace").splitlines():
        if "event=shot" not in line:
            continue
        lab = re.search(r"label=([^;]+)", line)
        cam = re.search(r"\| cam ([^|;]*)", line)
        gpu = re.search(r"gpu=(.*)$", line)
        frame = re.search(r"frame ([0-9.]+)", gpu.group(1)) if gpu else None
        comp = re.search(r"composite ([0-9.]+)", gpu.group(1)) if gpu else None
        print(f"{lab.group(1) if lab else '?':36s} {cam.group(1).strip() if cam else '-':100s} "
              f"frame={frame.group(1) if frame else '-'} composite={comp.group(1) if comp else '-'}")
