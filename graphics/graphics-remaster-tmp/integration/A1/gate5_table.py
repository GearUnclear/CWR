"""Gate item 5 table for A1: per hero view and band, A1's off against the A0
build's on (fixed clocks), next to the same-config noise references:
A0 off against A0 on (one game, both plain remastered), A1 off against A1 on
(one game), and A1 boot 2 against A1 boot 1 (both with the clouds held).
Cells: max channel difference / pixels over 2 levels."""
import json
import subprocess
import sys
from pathlib import Path

A1 = Path(__file__).resolve().parent
BD = A1.parents[2] / "arma_CWR" / "tests" / "rendering" / "captures" / "banddiff.py"
N = A1 / "20261018-015544-ab-A1"
P = A1 / "20261018-012542-ab-A0build-fixedclock"
N1 = A1 / "20261018-012542-ab-A1"


def run(*args):
    out = A1 / "_tmp_bd.json"
    subprocess.run([sys.executable, str(BD), *map(str, args), "--json", str(out)], check=True,
                   stdout=subprocess.DEVNULL)
    rows = json.loads(out.read_text())
    out.unlink()
    return {r["view"]: r.get("bands") for r in rows}


gate = run(N, P, "--label-a", "off", "--label-b", "on")
a0 = run("--pairs", P)
a1 = run("--pairs", N)
boots = run(N, N1, "--label-a", "on", "--label-b", "on")
names = ["sky", "horizon", "ground"]


def cell(b, i):
    return "" if not b else f"{b[i]['max']} / {b[i]['over2']:,}"


md = ["# Gate item 5 (off equals baseline), A1", "",
      "Cells: max channel difference / pixels over 2 levels (1920x1080; bands sky 0-422, horizon 422-591,",
      "ground 591-1080). gate = A1 `__off` (`remastered,<keys>=0`) against the A0 build's `__on`",
      "(`remastered`), both with the fixed per-view sim clock; A0 noise = the A0 build's two shots of the",
      "view in one game (identical settings); A1 noise = A1 off against A1 on in one game (20261018-015544); boots = A1 boot 2",
      "against A1 boot 1 (20261018-012542), `__on`, both with the clouds held and the fixed clock.", ""]
for i, band in enumerate(names):
    md += [f"## {band}", "", "| view | gate | A0 noise | A1 noise | A1 boots |", "|---|---|---|---|---|"]
    for v in sorted(gate):
        md.append(f"| {v} | {cell(gate[v], i)} | {cell(a0.get(v), i)} | {cell(a1.get(v), i)} | "
                  f"{cell(boots.get(v), i)} |")
    md.append("")
(A1 / "gate5-table.md").write_text("\n".join(md), encoding="utf-8")
print("wrote", A1 / "gate5-table.md")
