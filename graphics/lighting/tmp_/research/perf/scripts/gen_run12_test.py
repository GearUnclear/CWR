"""Run 12: the exact combination (light gate + UBO dedupe + flag 64 + terrain horizon
occlusion = debug 427) against the near-exact one (+ flag 128 = debug 555), and the
horizon-table rebuild cost with a moving camera (walking 0.5 m per frame, so the
table is rebuilt every frame). LOD pinned, observer sampled, override_mask (mask 0)."""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from perfgen import Test  # noqa: E402


def rounds(t, regime, configs, n=3, **common):
    for r in range(n):
        for c in configs:
            c = dict(c)
            tag = c.pop("tag")
            kw = dict(common)
            kw.update(c)
            t.cfg(f"{regime}{r}_{tag}", **kw)


def walk(t, label, mode, debug, vd, frames=120, step=0.5):
    """Configure, then move the eye `step` m along the heading every frame."""
    t.cfg(label + "_setup", mode, vd=vd, frames=10, debug=debug)
    t.raw("call { capOX0 = capOX; capOZ0 = capOZ; pfG0 = triGpuTimes false; pfG1 = triGpuTimes true; pfPR = triPerfReset 0 }\n")
    for i in range(1, frames + 1):
        t.raw(f"call {{ capOX = capOX0 + {i * step} * (sin capH); capOZ = capOZ0 + {i * step} * (cos capH); [] call capPinView }}\n")
        t.raw("triSimFrames 1\n")
    t.raw(f'call {{ pfLabel = "{label}"; "{label}" call pfRead; capOX = capOX0; capOZ = capOZ0; [] call capPinView }}\n')
    t.raw("triSimFrames 5\n")


BASE = [dict(tag="classic", mode="classic"), dict(tag="classic427", mode="classic", debug=427),
        dict(tag="m0"), dict(tag="m0all", debug=107), dict(tag="m0all427", debug=427),
        dict(tag="m0all555", debug=555)]
t = Test()
for regime, view, vd, settle in (("Q", "sunrise_ridge", 4000, 600), ("F", "noon_fields", 2500, 300)):
    t.view(view, settle=settle)
    rounds(t, regime, BASE, vd=vd, frames=150)
    for tag, mode, dbg in (("m0", "remastered", 0), ("m0all427", "remastered", 427), ("m0_b", "remastered", 0),
                           ("classic", "classic", 0), ("classic427", "classic", 427)):
        t.cfg(f"S{regime}_{tag}", mode, vd=vd, frames=90, shot=True, debug=dbg)
    if regime == "Q":
        for r in range(2):
            walk(t, f"MQ{r}_m0", "remastered", 0, vd)
            walk(t, f"MQ{r}_m0all427", "remastered", 427, vd)
            walk(t, f"MQ{r}_classic", "classic", 0, vd)
            walk(t, f"MQ{r}_classic427", "classic", 427, vd)
    t.clear()
t.write(sys.argv[1])
