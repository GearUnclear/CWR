"""Gate item 5 table for A3: per hero view and band, the A3 build's __off
(`remastered,<keys>=0`) against the A2 build's __on (`remastered`), both shot
this session with the fixed per-view clock and held clouds, next to the
same-config noise: A2 off against A2 on (one game), A3 off against A3 on (one
game), A3 on against A2 on (across builds), and the A2 build's on against A2's
own run of the same code (an earlier boot). Cells: max channel difference /
pixels over 2 levels."""
import json
import subprocess
import sys
from pathlib import Path

A3 = Path(__file__).resolve().parent
BD = A3.parents[2] / "arma_CWR" / "tests" / "rendering" / "captures" / "banddiff.py"
N = A3 / "20261018-040749-ab-A3"
P = A3 / "20261018-040811-ab-A2build"
OLD = A3.parent / "A2" / "20261018-024320-ab-A2"


def run(*args):
    out = A3 / "_tmp_bd.json"
    subprocess.run([sys.executable, str(BD), *map(str, args), "--json", str(out)], check=True,
                   stdout=subprocess.DEVNULL)
    rows = json.loads(out.read_text())
    out.unlink()
    return {r["view"]: r.get("bands") for r in rows}


gate = run(N, P, "--label-a", "off", "--label-b", "on")
onon = run(N, P, "--label-a", "on", "--label-b", "on")
a2 = run("--pairs", P)
a3 = run("--pairs", N)
boots = run(P, OLD, "--label-a", "on", "--label-b", "on")
names = ["sky", "horizon", "ground"]


def cell(b, i):
    return "" if not b else f"{b[i]['max']} / {b[i]['over2']:,}"


def zero(b):
    return bool(b) and all(x["max"] == 0 for x in b)


md = ["# Gate item 5 (off equals baseline), A3", "",
      "Cells: max channel difference / pixels over 2 levels (1920x1080; bands sky 0-422, horizon 422-591,",
      "ground 591-1080). gate = A3 `__off` (`remastered,<keys>=0`; no switch is read yet, so this is the",
      "plain remastered look) against the A2 build's `__on` (`remastered`), both this session",
      f"({N.name}, {P.name}) with the fixed per-view clock and held clouds.",
      "A3 on/A2 on = the same across builds for the `__on` shots. A2 noise and A3 noise = off against on",
      "in one game of that build. A2 boots = the A2 build's `__on` against A2's own run of the same code",
      "(tmp/integration/A2/20261018-024320-ab-A2), an earlier boot.", ""]
for i, band in enumerate(names):
    md += [f"## {band}", "", "| view | gate | A3 on / A2 on | A2 noise | A3 noise | A2 boots |", "|---|---|---|---|---|---|"]
    for v in sorted(gate):
        md.append(f"| {v} | {cell(gate[v], i)} | {cell(onon.get(v), i)} | {cell(a2.get(v), i)} | "
                  f"{cell(a3.get(v), i)} | {cell(boots.get(v), i)} |")
    md.append("")
exact = [v for v in sorted(gate) if zero(gate[v]) and zero(onon.get(v))]
repeat = [v for v in sorted(gate) if zero(boots.get(v))]
md += ["## Pixel-identical views", "",
       f"Views where A2 boot-to-boot is identical in every band (the repeatable views): {len(repeat)} "
       f"({', '.join(repeat)}).", "",
       f"Views where A3 (off and on) is identical to the A2 build in every band: {len(exact)} "
       f"({', '.join(exact)}).", "",
       f"Repeatable views that are not identical across builds: "
       f"{', '.join(v for v in repeat if v not in exact) or 'none'}.", ""]
(A3 / "gate5-table.md").write_text(chr(10).join(md), encoding="utf-8")
json.dump({"gate": gate, "onon": onon, "a2": a2, "a3": a3, "boots": boots}, open(A3 / "gate5.json", "w"), indent=1)
print(chr(10).join(md[-6:]))
