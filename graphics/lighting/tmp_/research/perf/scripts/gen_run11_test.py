"""Run 11: where the foliage time goes and cheaper equivalents, forest_edge_golden.
Shader override_lod (mask bits on the saturation knob): 1 shadow off, 2 horizon off,
4 cloud off, 8 fog toward sun off, 64 crown off, 127 all features off, 128 unlit,
256 one shadow tap, 2048 early cutout, 4096 foliage 4 taps, 8192 far tiers 4 taps.
All Remastered configs carry the light gate + UBO dedupe (debug 107). LOD pinned,
observer sampled.

  H  2.0x, 8x MSAA (GPU-heavy), G  1.5x, 4x MSAA (capture settings)"""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from perfgen import Test  # noqa: E402

CUT, FOL4, FAR4 = 2048, 4096, 8192


def rounds(t, regime, configs, n=3, **common):
    for r in range(n):
        for c in configs:
            c = dict(c)
            tag = c.pop("tag")
            kw = dict(common)
            kw.update(c)
            t.cfg(f"{regime}{r}_{tag}", **kw)


H = [dict(tag="classicall", mode="classic"), dict(tag="m0all"), dict(tag="unlit", mask=128),
     dict(tag="cut", mask=CUT), dict(tag="cutfol4", mask=CUT + FOL4), dict(tag="cutfar4", mask=CUT + FAR4),
     dict(tag="cutboth", mask=CUT + FOL4 + FAR4), dict(tag="onetap", mask=256), dict(tag="shadowoff", mask=1),
     dict(tag="crownoff", mask=64), dict(tag="horizonoff", mask=2), dict(tag="cloudoff", mask=4),
     dict(tag="fogoff", mask=8), dict(tag="featuresoff", mask=127)]
t = Test()
t.view("forest_edge_golden", settle=300)
rounds(t, "H", H, vd=1500, scale=2.0, msaa=8, frames=150, debug=107)
rounds(t, "G", [dict(tag="classicall", mode="classic"), dict(tag="m0all"), dict(tag="unlit", mask=128),
                dict(tag="cut", mask=CUT), dict(tag="cutboth", mask=CUT + FOL4 + FAR4)],
       vd=1500, frames=150, debug=107)
for tag, mask in (("m0all", 0), ("cut", CUT), ("cutfol4", CUT + FOL4), ("cutboth", CUT + FOL4 + FAR4),
                  ("m0all_b", 0)):
    t.cfg(f"SG_{tag}", vd=1500, frames=90, shot=True, debug=107, mask=mask)
t.cfg("SG_m0", vd=1500, frames=90, shot=True)
t.cfg("SG_m0all192", vd=1500, frames=90, shot=True, debug=299)
rounds(t, "K", [dict(tag="m0all", debug=107), dict(tag="m0all64", debug=171), dict(tag="m0all192", debug=299),
                dict(tag="m0tocc", debug=356), dict(tag="m0all192tocc", debug=555)],
       n=2, vd=1500, frames=150)
t.clear()

# Flag 128 (with 64): animated (land-following) objects batch again beyond 400 m.
# debug 299 = 107 + 64 + 128. Flag 256 = terrain horizon occlusion: 356 = cull only, 555 = 299 + cull.
for regime, view, vd, settle in (("F", "noon_fields", 2500, 300), ("L", "lowsun_fp", 1500, 300),
                                 ("Q", "sunrise_ridge", 4000, 600)):
    t.view(view, settle=settle)
    rounds(t, regime, [dict(tag="classic", mode="classic"), dict(tag="classicall192", mode="classic", debug=299),
                       dict(tag="m0"), dict(tag="m0all", debug=107), dict(tag="m0all64", debug=171),
                       dict(tag="m0all192", debug=299), dict(tag="classictocc", mode="classic", debug=356),
                       dict(tag="m0tocc", debug=356), dict(tag="m0all192tocc", debug=555)], vd=vd, frames=150)
    t.cfg(f"S{regime}_m0", vd=vd, frames=90, shot=True)
    t.cfg(f"S{regime}_m0all192", vd=vd, frames=90, shot=True, debug=299)
    t.cfg(f"S{regime}_m0_b", vd=vd, frames=90, shot=True)
    t.cfg(f"S{regime}_classic", "classic", vd=vd, frames=90, shot=True)
    t.cfg(f"S{regime}_classicall192", "classic", vd=vd, frames=90, shot=True, debug=299)
    t.cfg(f"S{regime}_classictocc", "classic", vd=vd, frames=90, shot=True, debug=356)
    t.cfg(f"S{regime}_m0tocc", vd=vd, frames=90, shot=True, debug=356)
    t.cfg(f"S{regime}_m0all192tocc", vd=vd, frames=90, shot=True, debug=555)
    t.clear()
t.write(sys.argv[1])
