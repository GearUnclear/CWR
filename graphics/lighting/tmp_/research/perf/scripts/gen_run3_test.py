"""Run 3: scratch build, LOD pinned (CWR_PERF_PIN_LOD), UBO-flush prototype (debug=103)."""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from perfgen import Test  # noqa: E402

t = Test()
t.view("sunrise_ridge", settle=600)
A = [("classic", "classic", 0, 0), ("classicubo", "classic", 0, 103), ("m0", "remastered", 0, 0),
     ("m0ubo", "remastered", 0, 103), ("unlit", "remastered", 128, 0), ("allfeatoff", "remastered", 127, 0),
     ("shadowoff", "remastered", 1, 0)]
for r in range(4):
    for tag, mode, mask, dbg in A:
        t.cfg(f"A{r}_{tag}", mode, mask=mask, scale=1.5, msaa=4, vd=4000, debug=dbg)
B = [("classic", "classic", 0, 0), ("m0", "remastered", 0, 0), ("m0ubo", "remastered", 0, 103),
     ("unlit", "remastered", 128, 0)]
for r in range(3):
    for tag, mode, mask, dbg in B:
        t.cfg(f"B{r}_{tag}", mode, mask=mask, scale=2.0, msaa=8, vd=4000, debug=dbg)
C = [("classic", "classic", 0, 0), ("m0", "remastered", 0, 0), ("unlit", "remastered", 128, 0)]
for r in range(3):
    for tag, mode, mask, dbg in C:
        t.cfg(f"C{r}_{tag}", mode, mask=mask, scale=1.0, msaa=0, vd=4000, debug=dbg)
t.cfg("S_classic", "classic", frames=60, shot=True)
t.cfg("S_m0", "remastered", frames=60, shot=True)
t.cfg("S_m0ubo", "remastered", frames=60, shot=True, debug=103)

N = [("classic", "classic", 0, 0), ("m0", "remastered", 0, 0), ("m0ubo", "remastered", 0, 103),
     ("unlit", "remastered", 128, 0)]
t.view("noon_fields", settle=300)
for r in range(3):
    for tag, mode, mask, dbg in N:
        t.cfg(f"N{r}_{tag}", mode, mask=mask, scale=1.5, msaa=4, vd=2500, debug=dbg)
t.clear()
t.view("lowsun_fp", settle=300)
for r in range(3):
    for tag, mode, mask, dbg in N:
        t.cfg(f"L{r}_{tag}", mode, mask=mask, scale=1.5, msaa=4, vd=1500, debug=dbg)
t.clear()
t.write(sys.argv[1])
