"""Shared pieces of the capture test generators (gen_timing.py, gen_features_ab.py,
video/gen_video_tests.py): the test header, the per-view staging block and the
toml. Every generated test runs at 1920x1080 windowed, render scale 1.0, 4x MSAA
and captures the presented frame (plan section 7.1), with the repo rig
capture_lib.sqf.

Harness rules the output keeps (capture_lib.sqf header): one statement per line
at top level, every verb that can answer FAIL inside a helper, and triScreenshot
always a top-level statement with a string-literal label.

Clouds: the remastered cloud-shadow field integrates the wind every frame and
the sky dome clouds move with the simulation step, so re-pinning the sim clock
does not put either back. Every view holds both at CLOUD_DRIFT
(triPinCloudDrift, from A1 on) and releases them at the end, so both shots of a
pair, and builds from A1 on, see the same clouds and cloud shadows.
legacy=True leaves the pin out for builds older than A1, which do not have the
verb (an unknown verb is a script error there).
"""

from pathlib import Path

HERE = Path(__file__).resolve().parent
# Where every view holds the clouds: the remastered cloud-shadow field at its
# start offset (EngineGL33_Lighting.cpp s_cloudDrift) and the sky dome clouds
# at position 0 (Weather::_cloudsPos at load).
CLOUD_DRIFT = (1830, 970, 0)


def num(v):
    return f"{v:g}" if isinstance(v, float) else str(v)


def header(world, v0, title, include="capture_lib.sqf", extra_includes=()):
    """Boot, camera, probe and the 1080p capture settings. `v0` places the
    probe and the camera before the first view."""
    incs = "\n".join(f'#include "{i}"' for i in (include,) + tuple(extra_includes))
    return f"""// {title}
// Generated, do not edit: 1920x1080 windowed, render scale 1.0, 4x MSAA, presented frame.
{incs}

triSimUntil {{ alive player }}

capWorld = "{world}"
capFov = 0.7
capSimT = 0
capPinTime = false
capUseView = false
capProps = []
capView = "setup"
capModeName = "none"
capLit = "none"
capGpu = "none"
capPerf = "none"
capFire = objNull
capBurnV = objNull
capJeep = objNull
capDrv = objNull

call {{ format ["view=setup;event=window_boot;size=%1", triGetWindowSize] call capLog }}
triCaptureAtPresent true
triSetMsaa 4
triSetRenderScale 1.0
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false

createCenter west
createCenter east
capGrpW = createGroup west
capGrpE = createGroup east
capProbe = "HeliHEmpty" createVehicle [{num(v0['x'])}, {num(v0['z'])}, 0]
capCam = "camera" camCreate [{num(v0['x'])}, {num(v0['z'])}, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call {{ format ["view=setup;event=setup;world={world};msaa=4;scale=1.0;window=%1;capture=present;lobo=%2", triGetWindowSize, "LoBo_camnet" call capHas] call capLog }}
"""


def begin(v, label, legacy=False, sim_time=None):
    """capBegin for view `v` under `label`, with its rain and eye mode, the
    clouds held (unless legacy), and its props. With `sim_time` the view's sim
    clock is that many seconds (capSimT, re-pinned before every shot) instead
    of the run's elapsed time at capBegin, so the sea and anything else keyed
    on Glob.time look the same in every run and build; give each view its own
    value, later views higher (time jumps forward between views, never far back)."""
    d = v["date"]
    out = [f"call {{ capRain = {num(v.get('rain', 0))}; capEyeAbs = {'true' if v.get('eyeAbs') else 'false'} }}",
           f'["{label}", {num(v["x"])}, {num(v["z"])}, {num(v["eye"])}, {num(v["h"])}, {num(v["p"])}, '
           f'[{", ".join(str(x) for x in d)}], {num(v.get("overcast", 0))}, {num(v.get("fog", 0))}, '
           f'{num(v["vd"])}] call capBegin']
    if sim_time is not None:
        out.append(f"call {{ capSimT = {num(sim_time)}; capSimRes = triSetSimTime capSimT; "
                   f'format ["view=%1;event=clock;simT=%2;res=%3", capView, capSimT, capSimRes] call capLog }}')
    if not legacy:
        out.append(f'call {{ capDriftRes = triPinCloudDrift [{CLOUD_DRIFT[0]}, {CLOUD_DRIFT[1]}, {CLOUD_DRIFT[2]}]; '
                   f'format ["view=%1;event=clouds;drift=%2", capView, capDriftRes] call capLog }}')
    if v.get("props"):
        out.append("call {")
        out += ["    " + s for s in v["props"]]
        out.append("}")
    return out


def shot(v, label, mode_setting, shot_label):
    """One timed, settled shot of the current view with `mode_setting` (a
    triSetLighting setting), saved as <seq>_<shot_label>.png."""
    out = [f'"{mode_setting}" call capMode',
           "triSimFrames 90",
           "[] call capReadTimers",
           "[] call capPin"]
    if v.get("refresh"):
        out.append(v["refresh"])
    out += ["triSimFrames 3",
            f'triScreenshot "{shot_label}"',
            f'"{shot_label}" call capRecord']
    return out


def end(v, legacy=False):
    out = ["[] call capClear"]
    if not legacy:
        out.append("call { capDriftRes = triPinCloudDrift [] }")
    if v.get("cleanup"):
        out.append(v["cleanup"])
    return out


def toml(world, title, worlds, timeout=5400, assert_timeout=240):
    w = worlds[world]
    args = ['"--no-strict"'] + w["mods"] + ['"--display-mode"', '"windowed"', '"--width"', '"1920"', '"--height"',
                                           '"1080"']
    return f"""# {title}
# Generated, do not edit. A photo and timing generator, not a regression test:
# run it with run-captures.ps1.
binary = "PoseidonGame"
mission = "{w['mission']}"
tags = {w['tags']}
timeout = {timeout}
assert_timeout = {assert_timeout}
extra_args = [{", ".join(args)}]
"""


def write(path: Path, text: str):
    path.write_text(text, encoding="utf-8", newline="\n")
