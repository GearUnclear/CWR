"""Per-shot CPU and GPU numbers of the valley bisect runs, with whether a
foreign game ran in the 10 s before each shot."""
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from pool_quiet import shots_clean  # noqa: E402

CAPS = Path(r"D:\Arma_CWA_worktrees\graphics-remaster\tmp\captures")
FIELDS = {
    "cpu": r"perf=fps=[^ ]+ frame=([0-9.]+)",
    "land_obj": r"land:obj=([0-9.]+)",
    "drw_obj": r"drw:obj=([0-9.]+)",
    "gpu": r"gpu=frame ([0-9.]+)",
    "gpu_min": r"min frame=([0-9.]+)",
    "pass1": r"\[ground [0-9.]+ pass1 ([0-9.]+)",
    "litKB": r"litKB=([0-9.]+)",
    "vsKB": r"vsKB=([0-9.]+)",
    "runs": r"runs=([0-9]+)",
}
names = sys.argv[1:] or ["A3", "A4lane", "B1lane", "B2lane", "B4before", "final"]
print("| build | shot | clean | CPU frame | land:obj | drw:obj | GPU frame | GPU min | pass1 | litKB | vsKB | runs |")
print("|---|---|---|---|---|---|---|---|---|---|---|---|")
for n in names:
    d = CAPS / f"final-bisect-{n}-timing"
    clean = shots_clean(d)
    for line in (d / "udcap.log").read_text(encoding="utf-8", errors="replace").splitlines():
        m = re.search(r"event=shot;label=([^;]+);", line)
        if not m:
            continue
        lab = m.group(1)
        vals = []
        for k, rx in FIELDS.items():
            mm = re.search(rx, line)
            vals.append(mm.group(1) if mm else "")
        print(f"| {n} | {lab} | {'yes' if clean.get(lab, (False,))[0] else 'no'} | " + " | ".join(vals) + " |")
