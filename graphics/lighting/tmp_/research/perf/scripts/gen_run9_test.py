"""Run 9: early cutout (alpha test before lighting, mask bit 2048 of override_cutout),
the shader feature masks on the foliage-heavy view, and perf flag 64 (objects whose
vertices are animated per object, Object::IsAnimated, stay out of instanced runs).
LOD pinned, observer sampled.

  G  forest_edge_golden VD 1500 at the capture settings (1.5x, 4x MSAA)
  H  forest_edge_golden VD 1500 at 2.0x, 8x MSAA (GPU-heavy: fragment cost amplified)
  F  noon_fields VD 2500: identity of the gate (repeat shots) and flag 64
  L  lowsun_fp VD 1500, Q sunrise_ridge VD 4000: stock vs early cutout vs flag 64
debug 107 = lit UBO dirty range + VS UBO skip + light gate; 171 = 107 + flag 64."""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from perfgen import Test  # noqa: E402

CUT = 2048


def rounds(t, regime, configs, n=3, **common):
    for r in range(n):
        for c in configs:
            c = dict(c)
            tag = c.pop("tag")
            kw = dict(common)
            kw.update(c)
            t.cfg(f"{regime}{r}_{tag}", **kw)


t = Test()
t.view("forest_edge_golden", settle=300)
rounds(t, "G", [dict(tag="classic", mode="classic"), dict(tag="classicall", mode="classic", debug=107),
                dict(tag="classicall64", mode="classic", debug=171),
                dict(tag="m0"), dict(tag="m0all", debug=107), dict(tag="m0all64", debug=171),
                dict(tag="cut", debug=107, mask=CUT),
                dict(tag="shadowoff", debug=107, mask=1), dict(tag="horizonoff", debug=107, mask=2),
                dict(tag="crownoff", debug=107, mask=64), dict(tag="featuresoff", debug=107, mask=127),
                dict(tag="unlit", debug=107, mask=128), dict(tag="postoff", debug=107, post=0)],
       vd=1500, frames=150)
rounds(t, "H", [dict(tag="classicall", mode="classic", debug=107), dict(tag="m0all", debug=107),
                dict(tag="cut", debug=107, mask=CUT), dict(tag="unlit", debug=107, mask=128)],
       vd=1500, scale=2.0, msaa=8, frames=150)
t.cfg("SG_m0", vd=1500, frames=90, shot=True)
t.cfg("SG_m0all", vd=1500, frames=90, shot=True, debug=107)
t.cfg("SG_m0all64", vd=1500, frames=90, shot=True, debug=171)
t.cfg("SG_cut", vd=1500, frames=90, shot=True, debug=107, mask=CUT)
t.cfg("SG_m0all_b", vd=1500, frames=90, shot=True, debug=107)
t.cfg("SG_cut_b", vd=1500, frames=90, shot=True, debug=107, mask=CUT)
t.clear()

t.view("noon_fields", settle=300)
rounds(t, "F", [dict(tag="classic", mode="classic"), dict(tag="classicall", mode="classic", debug=107),
                dict(tag="classicall64", mode="classic", debug=171),
                dict(tag="m0"), dict(tag="m0all", debug=107), dict(tag="m0all64", debug=171)],
       vd=2500, frames=150)
t.cfg("SF_classic", "classic", vd=2500, frames=90, shot=True)
t.cfg("SF_classicall64", "classic", vd=2500, frames=90, shot=True, debug=171)
t.cfg("SF_m0", vd=2500, frames=90, shot=True)
t.cfg("SF_m0all", vd=2500, frames=90, shot=True, debug=107)
t.cfg("SF_m0all64", vd=2500, frames=90, shot=True, debug=171)
t.cfg("SF_m0_b", vd=2500, frames=90, shot=True)
t.clear()

t.view("lowsun_fp", settle=300)
rounds(t, "L", [dict(tag="m0"), dict(tag="m0all", debug=107), dict(tag="m0all64", debug=171),
                dict(tag="cut", debug=107, mask=CUT)], vd=1500, frames=150)
t.cfg("SL_m0", vd=1500, frames=90, shot=True)
t.cfg("SL_m0all64", vd=1500, frames=90, shot=True, debug=171)
t.cfg("SL_cut", vd=1500, frames=90, shot=True, debug=107, mask=CUT)
t.clear()

t.view("sunrise_ridge", settle=600)
rounds(t, "Q", [dict(tag="m0"), dict(tag="m0all", debug=107), dict(tag="m0all64", debug=171),
                dict(tag="cut", debug=107, mask=CUT)], vd=4000, frames=150)
t.cfg("SQ_m0", vd=4000, frames=90, shot=True)
t.cfg("SQ_m0all64", vd=4000, frames=90, shot=True, debug=171)
t.cfg("SQ_cut", vd=4000, frames=90, shot=True, debug=107, mask=CUT)
t.write(sys.argv[1])
