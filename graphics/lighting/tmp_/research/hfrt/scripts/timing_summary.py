"""Cross-run GPU timing summary: for every view, the minimum GPU frame time of
each mode over every run's interleaved timing rounds (runs/*/timing_rounds.md),
with the run it came from. The GPU (RTX 4060 Ti) was shared with up to three
other games the whole time, so a minimum is the closest thing to the
uncontended cost this machine allowed, and every number is an upper bound."""
from pathlib import Path

R = Path(r"D:\Arma_CWA_worktrees\lighting\tmp\research\hfrt")
best = {}
for p in sorted((R / "runs").glob("*/timing_rounds.md")):
    run = p.parent.name
    for line in p.read_text().splitlines()[2:]:
        c = [x.strip() for x in line.strip("|").split("|")]
        if len(c) < 4:
            continue
        view, mode, rounds, fmin = c[0], c[1], int(c[2]), float(c[3])
        key = (view, mode)
        if key not in best or fmin < best[key][0]:
            best[key] = (fmin, run, rounds)
out = ["| view | mode | best GPU frame ms | run | rounds in that run |", "|---|---|---|---|---|"]
for (view, mode), (t, run, n) in sorted(best.items()):
    out.append(f"| {view} | {mode} | {t:.2f} | {run} | {n} |")
text = "\n".join(out) + "\n"
(R / "timings_best_across_runs.md").write_text(text, encoding="utf-8")
print(text)
