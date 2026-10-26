"""Run 7: the capture harness's per-frame frame observer (centre-pixel readback =
CPU/GPU sync + an extra MSAA resolve every frame under --autotest) against
normal-play sampling (perf flag 32). Run WITHOUT POSEIDON_OBSERVER_SAMPLE."""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from perfgen import Test  # noqa: E402

t = Test()
t.view("sunrise_ridge", settle=600)
C = [("classic", "classic", 0), ("classicsampled", "classic", 132), ("m0", "remastered", 0),
     ("m0sampled", "remastered", 132), ("m0all", "remastered", 107), ("m0allsampled", "remastered", 139)]
for r in range(3):
    for tag, mode, dbg in C:
        t.cfg(f"A{r}_{tag}", mode, debug=dbg, vd=4000)
t.view("lowsun_fp", settle=300)
for r in range(3):
    for tag, mode, dbg in C:
        t.cfg(f"L{r}_{tag}", mode, debug=dbg, vd=1500)
t.clear()
t.write(sys.argv[1])
