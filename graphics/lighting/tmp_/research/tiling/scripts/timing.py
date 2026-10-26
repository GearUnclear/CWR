"""Parse the interleaved timing test (tiling_timing.test.sqf): per view, the GPU
frame / world ms of every rep of every variant, the mean, and the paired delta
against v0 in the same rep (robust to other games loading the GPU over time).

    python timing.py <run_dir> [> timings_interleaved.md]
"""
import re
import statistics as st
import sys
from pathlib import Path

ANSI = re.compile(r"\x1b\[[0-9;]*m")
SHOT = re.compile(r"UDCAP;view=([^;]+);event=shot;label=([^;]+);.*?;gpu=(.*)$")


def main():
    run = Path(sys.argv[1])
    logs = [p for p in run.rglob("game_stdout.log") if "timing" in str(p)]
    rows = {}
    for p in logs:
        for raw in p.read_text(encoding="utf-8", errors="replace").splitlines():
            m = SHOT.search(ANSI.sub("", raw))
            if not m:
                continue
            view, label, gpu = m.group(1), m.group(2), m.group(3)
            fr = re.search(r"\bframe ([\d.]+) ms", gpu)
            wo = re.search(r"\bworld ([\d.]+)", gpu)
            if not fr:
                continue
            parts = label.split("__")
            if len(parts) == 3:
                rep, var = parts[1], parts[2]
            else:
                rep, var = "-", parts[-1]
            rows.setdefault(view, []).append((rep, var, float(fr.group(1)), float(wo.group(1)) if wo else None))
    print("# Interleaved GPU timing (tiling)\n")
    print("Each rep shoots v0 and the variants back to back on the same view (120 frames each);")
    print("the delta is variant minus v0 within the same rep. Other agents' games share the GPU,")
    print("so absolute numbers are inflated; the paired median delta is the cost estimate.\n")
    for view, rs in rows.items():
        print(f"## {view}\n")
        print("| rep | mode | GPU frame ms | world ms |")
        print("|---|---|---|---|")
        for rep, var, f, w in rs:
            print(f"| {rep} | {var} | {f:.2f} | {w if w is None else round(w, 2)} |")
        by = {}
        for rep, var, f, w in rs:
            by.setdefault(rep, {})[var] = (f, w)
        variants = sorted({v for _, v, _, _ in rs if v not in ("v0", "classic")})
        print()
        for v in variants:
            dF = [by[r][v][0] - by[r]["v0"][0] for r in by if v in by[r] and "v0" in by[r]]
            dW = [by[r][v][1] - by[r]["v0"][1] for r in by if v in by[r] and "v0" in by[r] and by[r][v][1] is not None]
            if dF:
                print(f"- {v} minus v0: frame median {st.median(dF):+.2f} ms (n={len(dF)}, "
                      f"range {min(dF):+.2f}..{max(dF):+.2f}); world median {st.median(dW):+.2f} ms")
        v0s = [by[r]["v0"][0] for r in by if "v0" in by[r]]
        if v0s:
            print(f"- v0 frame median {st.median(v0s):.2f} ms")
        if "-" in by and "classic" in by["-"]:
            print(f"- classic frame {by['-']['classic'][0]:.2f} ms (single read)")
        print()


if __name__ == "__main__":
    main()
