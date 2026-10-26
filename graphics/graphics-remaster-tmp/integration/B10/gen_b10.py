"""B10 light-gate capture tests: the clouds step's hero views plus the time and
weather checks, each shot with the clouds switches off and on in one game.

    python gen_b10.py <captures dir of the build's worktree> [--drift] [--timing]

Writes b10_abel.test.sqf / .toml and b10_eden.test.sqf / .toml into that
captures dir (Trident resolves the #include of capture_lib.sqf against the test
file). Same rig and settings as gen_features_ab.py (1920x1080, scale 1.0, 4x
MSAA, presented frame, clouds pinned at capgen.CLOUD_DRIFT, a fixed sim clock
per view). On a build without the feature the switch keys are inert, so off and
on draw the same frame (the repeat noise).

--drift adds, on Eden's broken_high, on-shots with the sky clouds pinned at
further positions (the dome turns with Landscape::CloudsPosition), so the
shadows can be seen to move with the sky.
--timing writes b10_timing.test.sqf instead: coast_oc_up, broken_high and
noon_fields off then on, three interleaved rounds, triGpuTimes per shot.
"""
import argparse
import sys
from pathlib import Path

ap = argparse.ArgumentParser()
ap.add_argument("captures")
ap.add_argument("--drift", action="store_true")
ap.add_argument("--timing", action="store_true")
a = ap.parse_args()
sys.path.insert(0, a.captures)
import capgen  # noqa: E402
import views  # noqa: E402

SIM_T0, SIM_DT = 2000, 600
OFF = "remastered,litClouds=0,skyCloudShadows=0"


def timing(base, table, **changes):
    for v in table:
        if v["name"] == base:
            out = dict(v)
            for k in ("f10", "note", "props", "refresh", "cleanup"):
                out.pop(k, None)
            out.update(changes)
            return out
    raise KeyError(base)


ABEL = [
    timing("coast_oc_up", views.TIMING_ABEL, name="clouds_coast_oc_up", feature="clouds hero (sun 8 deg, overcast 0.5)"),
    dict(name="clouds_sunrise_oc", x=4400, z=3300, eye=1.75, h=60, p=2, date=[1985, 7, 5, 5, 8], vd=4000,
         overcast=0.5, feature="dawn, overcast 0.5"),
    dict(name="clouds_coast_oc", x=1550, z=5150, eye=3, h=278, p=-4, date=[1985, 7, 5, 18, 35], vd=4000,
         overcast=0.5, feature="eye level, sea and beach under the sky, overcast 0.5"),
    timing("coastline_sunset", views.TIMING_ABEL, name="clouds_coastline_clear", h=250,
           feature="clear sky (gates off: on equals off)"),
    timing("noon_fields", views.TIMING_ABEL, name="clouds_noon_fields_oc", overcast=0.5,
           feature="noon, overcast 0.5 (relief faded out)"),
]
EDEN = [
    timing("broken_high", views.TIMING_EDEN, name="clouds_broken_high", feature="clouds hero (shadows from 250 m)"),
    dict(name="clouds_overcast_broken", x=9200, z=3700, eye=1.75, h=288, p=-4, date=[1985, 7, 5, 14, 30], vd=3500,
         overcast=0.5, props=['[["Jeep"], 12, -5, 70, "veh"] call capSpawn;'], feature="eye level, overcast 0.5"),
    dict(name="clouds_broken_up", x=9200, z=3700, eye=1.75, h=250, p=22, date=[1985, 7, 5, 14, 30], vd=3500,
         overcast=0.5, feature="midday looking up (no emboss)"),
    dict(name="clouds_overcast_heavy", x=9200, z=3700, eye=1.75, h=288, p=-4, date=[1985, 7, 5, 14, 30], vd=3500,
         overcast=0.9, feature="overcast 0.9 deck"),
]


def write_test(name, world, vs, title, drift=False):
    lines = [capgen.header(world, vs[0], title)]
    for i, v in enumerate(vs):
        lines.append(f"\n// ---- {v['name']}: {v['feature']}")
        lines += capgen.begin(v, v["name"], False, sim_time=SIM_T0 + SIM_DT * i)
        lines.append("triSimFrames 150")
        lines += capgen.shot(v, v["name"], OFF, f"{v['name']}__off")
        lines += capgen.shot(v, v["name"], "remastered", f"{v['name']}__on")
        lines.append('call { capFs = triFeatureState; format ["view=%1;event=featstate;fs=%2", capView, capFs] call capLog }')
        if drift and v["name"] == "clouds_broken_high":
            for pos in (25, 50):
                lines.append(f"call {{ capDriftRes = triPinCloudDrift [{capgen.CLOUD_DRIFT[0]}, {capgen.CLOUD_DRIFT[1]}, "
                             f'{pos}]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }}')
                lines += capgen.shot(v, v["name"], "remastered", f"{v['name']}__drift{pos}")
        lines += capgen.end(v, False)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = Path(a.captures)
    capgen.write(out / f"{name}.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / f"{name}.test.toml", capgen.toml(world, title, views.WORLDS, timeout=3600))
    print(f"{name}: {len(vs)} views")


def write_timing():
    vs = [ABEL[0], timing("noon_fields", views.TIMING_ABEL, name="clouds_noon_fields_clear", feature="gate off")]
    lines = [capgen.header("Abel", vs[0], "B10 timing: clouds off then on, three interleaved rounds.")]
    for r in range(3):
        for i, v in enumerate(vs):
            lines.append(f"\n// ---- round {r}: {v['name']}")
            lines += capgen.begin(v, v["name"], False, sim_time=SIM_T0 + SIM_DT * (r * len(vs) + i))
            lines.append("triSimFrames 150")
            lines += capgen.shot(v, v["name"], OFF, f"{v['name']}__r{r}_off")
            lines += capgen.shot(v, v["name"], "remastered", f"{v['name']}__r{r}_on")
            lines.append('call { capFs = triFeatureState; format ["view=%1;event=featstate;fs=%2", capView, capFs] call capLog }')
            lines += capgen.end(v, False)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = Path(a.captures)
    capgen.write(out / "b10_timing.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "b10_timing.test.toml", capgen.toml("Abel", "B10 timing", views.WORLDS, timeout=3600))
    print("b10_timing")


def write_sweep():
    """Eden at 14:30, overcast 0.5, the sky turned through half a turn (sky
    position 0 to 600 = 0 to 180 deg): once looking up at the sun (azimuth
    about 253 deg, 54 deg up), once at the ground from the same spot. The sun
    disk dims when a dome cloud crosses it, and the ground at the camera falls
    into shadow at the same positions."""
    sun_up = dict(name="clouds_sun_up", x=9200, z=3700, eye=1.75, h=253, p=50, date=[1985, 7, 5, 14, 30], vd=3500,
                  overcast=0.5, feature="the sun disk under the turning sky")
    ground = dict(EDEN[1], name="clouds_ground_sweep", feature="the ground at the camera under the turning sky")
    lines = [capgen.header("Eden", sun_up, "B10 sky sweep: the dome turned through 180 deg, clouds on.")]
    for i, v in enumerate((sun_up, ground)):
        lines.append(f"\n// ---- {v['name']}: {v['feature']}")
        lines += capgen.begin(v, v["name"], False, sim_time=SIM_T0 + SIM_DT * i)
        lines.append("triSimFrames 150")
        for pos in range(0, 601, 30):
            lines.append(f"call {{ capDriftRes = triPinCloudDrift [{capgen.CLOUD_DRIFT[0]}, {capgen.CLOUD_DRIFT[1]}, "
                         f'{pos}]; format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }}')
            lines += capgen.shot(v, v["name"], "remastered", f"{v['name']}__sky{pos:03d}")
        lines += capgen.end(v, False)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = Path(a.captures)
    capgen.write(out / "b10_sweep.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "b10_sweep.test.toml", capgen.toml("Eden", "B10 sky sweep", views.WORLDS, timeout=3600))
    print("b10_sweep")


def write_seam():
    """The sun_up view (sky position 0) under each switch combination, to place
    a straight edge seen high in the sky."""
    v = dict(name="clouds_sun_up", x=9200, z=3700, eye=1.75, h=253, p=50, date=[1985, 7, 5, 14, 30], vd=3500,
             overcast=0.5, feature="edge check")
    lines = [capgen.header("Eden", v, "B10 edge check: the sky high up under each switch combination.")]
    lines += capgen.begin(v, v["name"], False, sim_time=SIM_T0)
    lines.append("triSimFrames 150")
    for mode, label in (("remastered", "on"), (OFF, "off"), ("remastered,litClouds=0", "nolit"),
                        ("remastered,skyCloudShadows=0", "noshadow"),
                        ("remastered,atmosphere=0,litClouds=0,skyCloudShadows=0", "noatmo"), ("classic", "classic")):
        lines += capgen.shot(v, v["name"], mode, f"{v['name']}__{label}")
    lines += capgen.end(v, False)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = Path(a.captures)
    capgen.write(out / "b10_seam.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "b10_seam.test.toml", capgen.toml("Eden", "B10 edge check", views.WORLDS, timeout=3600))
    print("b10_seam")


ap2 = sys.argv
if "--seam" in ap2:
    pass
if a.timing:
    write_timing()
    write_sweep()
    write_seam()
else:
    write_test("b10_abel", "Abel", ABEL, "B10 clouds light gate (Abel): clouds off then on per view.")
    write_test("b10_eden", "Eden", EDEN, "B10 clouds light gate (Eden): clouds off then on per view.", drift=a.drift)
