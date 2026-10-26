"""Copy every NNN_<label>.png out of a capture run's tri tree to <run>/<label>.png."""
import re
import shutil
import sys
from pathlib import Path

run = Path(sys.argv[1])
n = 0
for p in sorted((run / "tri").rglob("*.png")):
    m = re.match(r"^\d+_(.+\.png)$", p.name)
    if not m:
        continue
    shutil.copy2(p, run / m.group(1))
    n += 1
print(f"copied {n} captures into {run}")
