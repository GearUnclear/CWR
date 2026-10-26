"""Gate item 5 table for A2: per hero view and band, the A2 build's __off
(`remastered,<keys>=0`) against the A1 build's __on (`remastered`), both shot
this session with the fixed per-view clock and held clouds, next to the
same-config noise: A1 off against A1 on (one game), A2 off against A2 on (one
game), A2 on against A1 on (across builds), and the A1 build's on against A1's
own run of the same build (an earlier boot). Cells: max channel difference /
pixels over 2 levels."""
import json
import subprocess
import sys
from pathlib import Path

A2 = Path(__file__).resolve().parent
BD = A2.parents[2] / "arma_CWR" / "tests" / "rendering" / "captures" / "banddiff.py"
N = A2 / "20261018-024320-ab-A2"
P = A2 / "20261018-022516-ab-A1build"
OLD = A2.parent / "A1" / "20261018-015544-ab-A1"


def run(*args):
    out = A2 / "_tmp_bd.json"
    subprocess.run([sys.executable, str(BD), *map(str, args), "--json", str(out)], check=True,
                   stdout=subprocess.DEVNULL)
    rows = json.loads(out.read_text())
    out.unlink()
    return {r["view"]: r.get("bands") for r in rows}


gate = run(N, P, "--label-a", "off", "--label-b", "on")
onon = run(N, P, "--label-a", "on", "--label-b", "on")
a1 = run("--pairs", P)
a2 = run("--pairs", N)
boots = run(P, OLD, "--label-a", "on", "--label-b", "on")
names = ["sky", "horizon", "ground"]


def cell(b, i):
    return "" if not b else f"{b[i]['max']} / {b[i]['over2']:,}"


def zero(b):
    return bool(b) and all(x["max"] == 0 for x in b)


md = ["# Gate item 5 (off equals baseline), A2", "",
      "Cells: max channel difference / pixels over 2 levels (1920x1080; bands sky 0-422, horizon 422-591,",
      "ground 591-1080). gate = A2 `__off` (`remastered,<keys>=0`; no switch is read yet, so this is the",
      "plain remastered look) against the A1 build's `__on` (`remastered`), both this session",
      "(20261018-024320-ab-A2, 20261018-022516-ab-A1build) with the fixed per-view clock and held clouds.",
      "A2 on/A1 on = the same across builds for the `__on` shots. A1 noise and A2 noise = off against on",
      "in one game of that build. A1 boots = the A1 build's `__on` against A1's own run of the same code",
      "(tmp/integration/A1/20261018-015544-ab-A1), an earlier boot.", ""]
for i, band in enumerate(names):
    md += [f"## {band}", "", "| view | gate | A2 on / A1 on | A1 noise | A2 noise | A1 boots |", "|---|---|---|---|---|---|"]
    for v in sorted(gate):
        md.append(f"| {v} | {cell(gate[v], i)} | {cell(onon.get(v), i)} | {cell(a1.get(v), i)} | "
                  f"{cell(a2.get(v), i)} | {cell(boots.get(v), i)} |")
    md.append("")
exact = [v for v in sorted(gate) if zero(gate[v]) and zero(onon.get(v))]
repeat = [v for v in sorted(gate) if zero(boots.get(v))]
md += ["## Pixel-identical views", "",
       f"Views where A1 boot-to-boot is identical in every band (the repeatable views): {len(repeat)} "
       f"({', '.join(repeat)}).", "",
       f"Views where A2 (off and on) is identical to the A1 build in every band: {len(exact)} "
       f"({', '.join(exact)}).", "",
       f"Repeatable views that are not identical across builds: "
       f"{', '.join(v for v in repeat if v not in exact) or 'none'}.", ""]
(A2 / "gate5-table.md").write_text("\n".join(md), encoding="utf-8")
json.dump({"gate": gate, "onon": onon, "a1": a1, "a2": a2, "boots": boots}, open(A2 / "gate5.json", "w"), indent=1)
print("\n".join(md[-6:]))
