"""Run 10: the baseline capture flow replayed with the draw count logged. Run WITHOUT
CWR_PERF_PIN_LOD and WITHOUT POSEIDON_OBSERVER_SAMPLE and with no shader override,
i.e. exactly what run-captures.ps1 measures (adaptive LOD, frame observer every frame).

  B0  150 settle frames, Classic 90 frames, then Remastered 90 frames (the capture order)
  B1  the same pair again after 600 more frames (streaming finished)
  B2  Remastered first, then Classic
  B3  Classic + gate + UBO, Remastered + gate + UBO (debug 107)
Two rounds of B1..B3."""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from perfgen import Test  # noqa: E402

t = Test()
t.view("sunrise_ridge", settle=150)
t.cfg("B0_classic", "classic", vd=4000, frames=90, shot=True, sat=1.12)
t.cfg("B0_m0", vd=4000, frames=90, shot=True, sat=1.12)
t.raw("triSimFrames 600\n")
for r in (1, 2):
    t.cfg(f"P{r}_classic", "classic", vd=4000, frames=90, sat=1.12)
    t.cfg(f"P{r}_m0", vd=4000, frames=90, sat=1.12)
    t.cfg(f"R{r}_m0", vd=4000, frames=90, sat=1.12)
    t.cfg(f"R{r}_classic", "classic", vd=4000, frames=90, sat=1.12)
    t.cfg(f"X{r}_classicall", "classic", vd=4000, frames=90, debug=107, sat=1.12)
    t.cfg(f"X{r}_m0all", vd=4000, frames=90, debug=107, sat=1.12)
t.cfg("S1_classic", "classic", vd=4000, frames=90, shot=True, sat=1.12)
t.cfg("S1_m0", vd=4000, frames=90, shot=True, sat=1.12)
t.cfg("S1_m0all", vd=4000, frames=90, shot=True, debug=107, sat=1.12)
t.write(sys.argv[1])
