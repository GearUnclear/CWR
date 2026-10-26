"""Run 5: scratch build v3. Perf flags (debug=100+flags): 1 lit UBO dirty range,
2 VS UBO skip-unchanged, 4 per-object light gate for instancing. LOD pinned,
observer sampled. Shader: stock look (mask override, mask 0)."""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from perfgen import Test  # noqa: E402

t = Test()
t.view("sunrise_ridge", settle=600)
t.raw('call { pfShapes = triPerfDumpShapes 0 }\ntriSimFrames 5\n')
A = [dict(tag="classic", mode="classic"), dict(tag="classicinst", mode="classic", debug=104),
     dict(tag="classicall", mode="classic", debug=107),
     dict(tag="m0"), dict(tag="m0ubo", debug=103), dict(tag="m0inst", debug=104), dict(tag="m0all", debug=107),
     dict(tag="m0band", debug=108), dict(tag="m0all8", debug=115), dict(tag="m0all16", debug=123),
     dict(tag="classicall16", mode="classic", debug=123)]
for r in range(3):
    for c in A:
        c = dict(c)
        tag = c.pop("tag")
        t.cfg(f"A{r}_{tag}", scale=1.5, msaa=4, vd=4000, **c)
# Visual identity: the prototypes must not change a pixel.
t.cfg("S_classic", "classic", frames=60, shot=True)
t.cfg("S_classicall", "classic", frames=60, shot=True, debug=107)
t.cfg("S_m0", frames=60, shot=True)
t.cfg("S_m0all", frames=60, shot=True, debug=107)
t.cfg("S_m0all16", frames=60, shot=True, debug=123)
t.cfg("S_classicall16", "classic", frames=60, shot=True, debug=123)
# Same view at noon: no lamps lit.
t.raw('call { capDate = [1985, 7, 5, 12, 0]; [] call capPin }\ntriSimFrames 120\n')
for r in range(2):
    for c in [dict(tag="classic", mode="classic"), dict(tag="m0"), dict(tag="m0all", debug=107), dict(tag="m0all16", debug=123)]:
        c = dict(c)
        tag = c.pop("tag")
        t.cfg(f"N{r}_{tag}", scale=1.5, msaa=4, vd=4000, **c)
t.view("noon_fields", settle=300)
for r in range(2):
    for c in [dict(tag="classic", mode="classic"), dict(tag="classicall", mode="classic", debug=107),
              dict(tag="m0"), dict(tag="m0all", debug=107), dict(tag="m0all16", debug=123)]:
        c = dict(c)
        tag = c.pop("tag")
        t.cfg(f"F{r}_{tag}", scale=1.5, msaa=4, vd=2500, **c)
t.clear()
t.view("lowsun_fp", settle=300)
for r in range(2):
    for c in [dict(tag="classic", mode="classic"), dict(tag="classicall", mode="classic", debug=107),
              dict(tag="m0"), dict(tag="m0all", debug=107), dict(tag="m0all16", debug=123)]:
        c = dict(c)
        tag = c.pop("tag")
        t.cfg(f"L{r}_{tag}", scale=1.5, msaa=4, vd=1500, **c)
t.cfg("SL_m0", frames=60, shot=True)
t.cfg("SL_m0all", frames=60, shot=True, debug=107)
t.cfg("SL_m0all16", frames=60, shot=True, debug=123)
t.clear()
t.write(sys.argv[1])
