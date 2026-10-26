"""Run 4: scratch build, LOD pinned, observer sampled (POSEIDON_OBSERVER_SAMPLE=1),
amplification override: cost of one lit-shading evaluation by slope."""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from perfgen import Test  # noqa: E402

t = Test()
t.view("sunrise_ridge", settle=600)
# A: the capture's own settings.
A = [dict(tag="classic", mode="classic"), dict(tag="classicubo", mode="classic", debug=103),
     dict(tag="m0"), dict(tag="m0ubo", debug=103), dict(tag="unlit", mask=128),
     dict(tag="amp4", amp=4), dict(tag="amp4shadow", amp=4, mask=4096)]
for r in range(3):
    for c in A:
        c = dict(c)
        tag = c.pop("tag")
        t.cfg(f"A{r}_{tag}", scale=1.5, msaa=4, vd=4000, **c)
# C: GPU-heavy, few draw calls.
C = [dict(tag="classic", mode="classic"), dict(tag="m0"), dict(tag="unlit", mask=128),
     dict(tag="amp2", amp=2), dict(tag="amp4", amp=4), dict(tag="amp8", amp=8),
     dict(tag="amp8shadow", amp=8, mask=4096), dict(tag="amp8terrain", amp=8, mask=8192)]
for r in range(3):
    for c in C:
        c = dict(c)
        tag = c.pop("tag")
        t.cfg(f"C{r}_{tag}", scale=2.0, msaa=8, vd=1000, **c)
# D: capture settings, short view distance.
D = [dict(tag="classic", mode="classic"), dict(tag="m0"), dict(tag="unlit", mask=128), dict(tag="amp4", amp=4)]
for r in range(3):
    for c in D:
        c = dict(c)
        tag = c.pop("tag")
        t.cfg(f"D{r}_{tag}", scale=1.5, msaa=4, vd=1000, **c)
t.cfg("S_m0_amp8", amp=8, scale=1.5, msaa=4, vd=4000, frames=60, shot=True)
t.write(sys.argv[1])
