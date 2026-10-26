"""Run 6: visual identity of the instancing / UBO prototypes, with a same-config
noise floor (each reference shot twice), plus timings at night among lamps and
in a forest. Scratch build, LOD pinned, observer sampled, stock shading."""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from perfgen import Test  # noqa: E402

t = Test()
t.view("sunrise_ridge", settle=600)
for tag, mode, dbg in [("classic_a", "classic", 0), ("classic_b", "classic", 0), ("classicall", "classic", 107),
                       ("classicall8", "classic", 115), ("m0_a", "remastered", 0), ("m0_b", "remastered", 0),
                       ("m0all", "remastered", 107), ("m0all8", "remastered", 115)]:
    t.cfg(f"SR_{tag}", mode, frames=60, shot=True, debug=dbg, vd=4000)

t.view("town_night", settle=300)
T = [("classic", "classic", 0), ("classicall", "classic", 107), ("m0", "remastered", 0),
     ("m0all", "remastered", 107), ("m0all8", "remastered", 115)]
for r in range(2):
    for tag, mode, dbg in T:
        t.cfg(f"T{r}_{tag}", mode, debug=dbg, vd=800)
for tag, mode, dbg in [("classic_a", "classic", 0), ("classic_b", "classic", 0), ("classicall", "classic", 107),
                       ("classicall8", "classic", 115), ("m0_a", "remastered", 0), ("m0_b", "remastered", 0),
                       ("m0all", "remastered", 107), ("m0all8", "remastered", 115)]:
    t.cfg(f"TN_{tag}", mode, frames=60, shot=True, debug=dbg, vd=800)

t.view("forest_edge_golden", settle=300)
for r in range(2):
    for tag, mode, dbg in T:
        t.cfg(f"G{r}_{tag}", mode, debug=dbg, vd=1500)
t.clear()
t.write(sys.argv[1])
