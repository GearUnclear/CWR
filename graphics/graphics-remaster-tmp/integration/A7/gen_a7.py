"""A7 light-gate capture test: the sky table's hero views plus the time and
weather checks, each shot with the atmosphere switch off and on in one game.

    python gen_a7.py <captures dir of the build's worktree> [--name a7_sky] [--check]

Writes <name>.test.sqf / .toml into that captures dir (Trident resolves the
#include of capture_lib.sqf against the test file). Same rig and settings as
gen_features_ab.py (1920x1080, scale 1.0, 4x MSAA, presented frame, clouds
pinned, a fixed sim clock per view). On a build without the feature the switch
key is inert, so off and on draw the same frame (the repeat noise).

--check adds, per view after the on shot, triSkyTableCheck (the GPU sky-view
table read back against the CPU mirror) and logs its answer. Only the A7 build
has that verb (an unknown verb is a script error on older builds).
"""
import argparse
import sys
from pathlib import Path

ap = argparse.ArgumentParser()
ap.add_argument("captures")
ap.add_argument("--name", default="a7_sky")
ap.add_argument("--check", action="store_true")
a = ap.parse_args()
sys.path.insert(0, a.captures)
import capgen  # noqa: E402
import views  # noqa: E402

SIM_T0, SIM_DT = 2000, 600


def timing(base, **changes):
    for v in views.TIMING_ABEL:
        if v["name"] == base:
            out = dict(v)
            for k in ("f10", "note", "props", "refresh", "cleanup"):
                out.pop(k, None)
            out.update(changes)
            return out
    raise KeyError(base)


VIEWS = [
    timing("coastline_sunset", name="atmosphere_coastline_sunset", h=250, feature="sky table"),
    dict(name="atmosphere_firefight_dusk", x=7600, z=10850, eye=2, h=0, p=0, date=[1985, 7, 5, 19, 35], vd=1000,
         feature="sky table (dusk, sun -1.7 deg)"),
    timing("noon_fields", name="atmosphere_noon_fields", feature="sky table (noon cirrus)"),
    timing("sunrise_ridge", name="atmosphere_sunrise_ridge", feature="sky table (dawn)"),
    timing("coast_oc_up", name="atmosphere_coast_oc_up", feature="sky table (overcast 0.5)"),
    timing("town_night", name="atmosphere_town_night", feature="sky table off at night (gate)"),
]

lines = [capgen.header("Abel", VIEWS[0], "A7 sky table light gate: atmosphere off then on per view.")]
for i, v in enumerate(VIEWS):
    lines.append(f"\n// ---- {v['name']}: {v['feature']}")
    lines += capgen.begin(v, v["name"], False, sim_time=SIM_T0 + SIM_DT * i)
    lines.append("triSimFrames 150")
    lines += capgen.shot(v, v["name"], "remastered,atmosphere=0", f"{v['name']}__off")
    lines += capgen.shot(v, v["name"], "remastered", f"{v['name']}__on")
    if a.check:
        lines.append('call { capSkyChk = triSkyTableCheck 0; format ["view=%1;event=skycheck;res=%2", capView, '
                     'capSkyChk] call capLog }')
    lines += capgen.end(v, False)
lines.append('\ncall { "view=done;event=done" call capLog }')
lines.append("triEndTest")
out = Path(a.captures)
capgen.write(out / f"{a.name}.test.sqf", "\n".join(lines) + "\n")
capgen.write(out / f"{a.name}.test.toml",
             capgen.toml("Abel", "A7 sky table light gate", views.WORLDS, timeout=3600))
print(f"{a.name}: {len(VIEWS)} views")
