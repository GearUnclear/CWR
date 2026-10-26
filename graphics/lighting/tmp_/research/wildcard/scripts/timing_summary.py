"""Average interleaved timing shots (labels t<N>_<variant>) of a run into timing_summary.md."""
import sys
from pathlib import Path
run = Path(sys.argv[1])
rows = {}
for line in (run / "timings.md").read_text().splitlines():
    parts = [p.strip() for p in line.split("|")]
    if len(parts) < 7 or not parts[2][:1] == "t" or "_" not in parts[2]:
        continue
    try:
        f, w, sh = float(parts[3]), float(parts[4]), float(parts[5])
    except ValueError:
        continue
    rows.setdefault((parts[1], parts[2].split("_", 1)[1]), []).append((f, w, sh))
out = ["| view | variant | GPU frame ms per round | mean frame | mean world | mean shadow pass incl. sky maps |",
       "|---|---|---|---|---|---|"]
for (v, k), lst in sorted(rows.items()):
    fs = [x[0] for x in lst]
    out.append("| %s | %s | %s | %.2f | %.2f | %.2f |" % (v, k, ", ".join("%.2f" % x for x in fs), sum(fs) / len(fs),
                                                     sum(x[1] for x in lst) / len(lst), sum(x[2] for x in lst) / len(lst)))
(run / "timing_summary.md").write_text("\n".join(out) + "\n")
print("\n".join(out))
