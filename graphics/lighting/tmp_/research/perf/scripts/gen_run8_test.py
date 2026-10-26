"""Run 8: QUIET machine (no other games, CPU ~2%). The cost table again without
contention, LOD pinned, observer sampled (POSEIDON_OBSERVER_SAMPLE=1).

  Q  sunrise_ridge VD 4000: classic, classic+gate+UBO, remastered, remastered+gate+UBO,
     then shader feature masks on top of remastered+gate+UBO (the GPU-bound regime),
     and post off.
  F  noon_fields VD 2500, L lowsun_fp VD 1500, G forest_edge_golden VD 1500: the four base configs.
  Shots per view for the three-way comparisons (classic, remastered, remastered+gate+UBO).
Shader: override_mask (mask 0 = stock look). debug=100+flags (1 lit UBO, 2 VS UBO, 4 gate)."""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from perfgen import Test  # noqa: E402

BASE = [dict(tag="classic", mode="classic"), dict(tag="classicall", mode="classic", debug=107),
        dict(tag="m0"), dict(tag="m0all", debug=107)]
MASKS = [("shadowoff", 1), ("horizonoff", 2), ("cloudoff", 4), ("fogoff", 8), ("crownoff", 64),
         ("featuresoff", 127), ("unlit", 128), ("onetap", 256), ("primarycascade", 512)]

t = Test()
t.view("sunrise_ridge", settle=600)
for r in range(3):
    for c in BASE:
        c = dict(c)
        tag = c.pop("tag")
        t.cfg(f"Q{r}_{tag}", vd=4000, frames=150, **c)
    for tag, mask in MASKS:
        t.cfg(f"Q{r}_{tag}", vd=4000, frames=150, debug=107, mask=mask)
    t.cfg(f"Q{r}_postoff", vd=4000, frames=150, debug=107, post=0)
t.cfg("SQ_classic", "classic", vd=4000, frames=90, shot=True)
t.cfg("SQ_m0", vd=4000, frames=90, shot=True)
t.cfg("SQ_m0all", vd=4000, frames=90, shot=True, debug=107)

for regime, view, vd, settle in (("F", "noon_fields", 2500, 300), ("L", "lowsun_fp", 1500, 300),
                                 ("G", "forest_edge_golden", 1500, 300)):
    t.view(view, settle=settle)
    for r in range(3):
        for c in BASE:
            c = dict(c)
            tag = c.pop("tag")
            t.cfg(f"{regime}{r}_{tag}", vd=vd, frames=150, **c)
    t.cfg(f"S{regime}_classic", "classic", vd=vd, frames=90, shot=True)
    t.cfg(f"S{regime}_m0", vd=vd, frames=90, shot=True)
    t.cfg(f"S{regime}_m0all", vd=vd, frames=90, shot=True, debug=107)
    t.clear()
t.write(sys.argv[1])
