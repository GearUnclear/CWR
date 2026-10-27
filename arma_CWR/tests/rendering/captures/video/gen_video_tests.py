"""Generate the motion-capture tests for the side-by-side feature videos
(INTEGRATION-PLAN.md sections 7.1 to 7.3). Skeleton from step A1: the clip
table holds every row of plan section 7.3, the S and P modes of 7.2 are
written out, and make_video.py assembles the frames; step V refines the paths
and shoots.

    python gen_video_tests.py --list
    python gen_video_tests.py --clip perf_sunrise_ridge [--clip ...] [--frames 12] [--out <dir>]
    python gen_video_tests.py --all
    python gen_video_tests.py --paused-check

Each clip becomes video/<clip>.test.sqf + .test.toml (run them with
run-captures.ps1 -Tests tests/rendering/captures/video/<clip>.test.sqf). Frames
are labelled <clip>__off__f0001 .. and <clip>__on__f0001 ..; every frame logs
an event=frame line with triFeatureState (and every 10th frame triGpuTimes).

S mode (sequential passes; deterministic scenes and every temporal feature):
the whole path with the feature's keys off, then again with "remastered". Per
frame: capClockAt (sim clock, time-lapse date), capPathAt (camera),
capPropsAt (movers), triSimFrames 2, triScreenshot. AI props are captive with
MOVE off; vehicles move only by teleport. The clouds are held for the whole
clip (triPinCloudDrift), so both passes see the same sky and cloud shadows.

P mode (paired, paused; combat and rain): triSetFixedStep 1/30 and per frame:
triPauseGame, camera and movers, the off setting, triSimFrames 6, shot,
"remastered", triSimFrames 6, shot, triUnpauseGame, triSimFrames 1. Both shots
of a pair share the simulation state, provided the paused frames render and
hold fire and smoke still (--paused-check writes the test that checks it).

Every frame writes a PNG and a BMP twin (about 6 MB at 1080p; run-captures.ps1
deletes the BMPs at the end), so a 240-frame clip holds about 3 GB until then.
"""
import argparse
import math
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))

import capgen  # noqa: E402
import views  # noqa: E402

FPS = 30


def rel(x, z, h, forward, right):
    """capRel in Python: [east, north] at [forward, right] from (x, z) heading h."""
    r = math.radians(h)
    return (x + forward * math.sin(r) + right * math.cos(r), z + forward * math.cos(r) - right * math.sin(r))


def along(v, dist, h=None, eye=None, p=None):
    """The end key `dist` metres along the start heading (negative: back)."""
    x, z = rel(v[0], v[1], v[3], dist, 0)
    return [round(x, 2), round(z, 2), v[2] if eye is None else eye, v[3] if h is None else h,
            v[4] if p is None else p]


def strafe(v, right, h=None, p=None):
    x, z = rel(v[0], v[1], v[3], 0, right)
    return [round(x, 2), round(z, 2), v[2], v[3] if h is None else h, v[4] if p is None else p]


def line(a, b):
    return ("line", a, b)


def orbit(stage, centre_rel, radius, eye, a0, a1, pitch):
    cx, cz = rel(stage["x"], stage["z"], stage["h"], centre_rel[0], centre_rel[1])
    return ("orbit", [round(cx, 2), round(cz, 2), radius, eye, a0, pitch], [a1])


def stage_of(a, **kw):
    s = dict(x=a[0], z=a[1], eye=a[2], h=a[3], p=a[4])
    s.update(kw)
    return s


def _view(name):
    for v in views.TIMING_ABEL + views.TIMING_EDEN + views.TIMING_SINAI:
        if v["name"] == name:
            return v
    raise KeyError(name)


SINAI_SET = _view("sinai_lowsun_fp")["props"]
DRIVER = ('"SoldierWB" createUnit [[capStageX, capStageZ, 0], capGrpW, "capNewUnit = this", 0.6, "PRIVATE"]; '
          'capNewUnit setCaptive true; capNewUnit allowDamage false; capNewUnit moveInDriver {veh}; '
          'capNewUnit disableAI "MOVE"; capNewUnit setBehaviour "SAFE";')


def jeep_with_driver(var, forward, right, rdir):
    """A Jeep with a SAFE AI driver (headlights at night), MOVE off: it only moves by teleport."""
    return [f'{var} = [["Jeep"], {forward}, {right}, {rdir}, "veh"] call capSpawn;',
            f'if (not (isNull {var})) then {{ {var} lock false; ' + DRIVER.format(veh=var) + ' };']


# Plan section 7.3, one dict per row. path keys are [east, north, eye, heading,
# pitch]; stage is the capBegin view the props are placed from (the path start
# unless an orbit needs another); movers are (SQF object, [[frame, forward,
# right, relative dir], ...]) from the stage; events run SQF at a frame.
A_SUNRISE = [4400, 3300, 1.75, 60, 2]
A_FOREST = [4674, 10156, 1.75, 75, 3]
A_TOWN = [7152, 8915, 1.75, 345, 3]
A_COAST_SKY = [1550, 5150, 3, 250, -4]
A_WOODS = [6940, 9964, 1.75, 240, -2]
A_ROAD = [6895.2, 8836.6, 1.5, 69.6, -2]
A_ROAD_HEAD = [6891.9, 8835.4, 3, 69.6, -6]
A_FFH = [7600, 10850, 5, 0, -12]
A_COAST_SEA = [1550, 5150, 3, 278, -4]
A_SEA_NOON = [1550, 5150, 12, 278, -12]
A_BAY = [9240, 6950, 3, 272, -3]
A_TOWN_RAIN_ST = [7162, 8931, 1.75, 320, 0]
A_PINE_MORN = [6830, 9905, 1.75, 72, 6]
A_PINE_EVE = [4355, 3275, 1.75, 288, 5]
A_HL_FRONT = [7266.4, 8355.9, 1.6, 200, -1]
A_VALLEY = [6100, 5000, 1.75, 40, -6]
A_HILL = [5500, 6250, 15, 185, -14]
A_SLOPE = [5500, 6250, 15, 200, -12]
A_COAST_UP = [1550, 5150, 3, 260, 14]
E_HIGH = [9200, 3700, 250, 288, -12]
E_MEADOW = [9200, 3700, 1.75, 288, -4]
S_CAMP = [8350, 2950, 1.75, 338.8, -3]
LOWSUN_STAGE = dict(x=7300, z=10900, eye=1.75, h=152.5, p=-3)
SINAI_STAGE = dict(x=8350, z=2950, eye=1.75, h=338.8, p=-3)

CLIPS = [
    # The video's opening: the Remaster's lighting as it was before this work
    # ("baseline": every switch off) against the full new lighting.
    dict(clip="opening_pine_eve", feature="original look vs new lighting", keys=[], off="baseline",
         world="Abel", mode="S", frames=240, date=[1985, 7, 5, 18, 15], vd=1500, stage=stage_of(A_PINE_EVE),
         path=line(A_PINE_EVE, strafe(A_PINE_EVE, -10, h=300, p=3)),
         props=['[["SoldierWB"], 14, -3, 160, "man"] call capSpawn;', '[["Jeep"], 22, 4, 250, "veh"] call capSpawn;'],
         note="opening: baseline (all 26 switches off) against remastered; strafe 10 m left with a 12 deg pan"),
    dict(clip="perf_sunrise_ridge", feature="perf", keys=["drawGrouping", "hillCull"], world="Abel", mode="S",
         frames=240, date=[1985, 7, 5, 5, 8], vd=4000, stage=stage_of(A_SUNRISE),
         path=line(A_SUNRISE, [4426, 3315, 1.75, 75, 2])),
    dict(clip="temporal_forest_edge", feature="temporal", keys=["taa"], world="Abel", mode="S", frames=180,
         date=[1985, 7, 5, 18, 15], vd=1500, stage=stage_of(A_FOREST),
         path=line(A_FOREST, strafe(A_FOREST, 5.4, h=80)),
         props=['[["SoldierWB"], 12, 2, 215, "man"] call capSpawn;',
                'capMover1 = [["Jeep"], 25, -15, 90, "veh"] call capSpawn;'],
         movers=[("capMover1", [[0, 25, -15, 90], [120, 25, -0.6, 90], [180, 25, 29.4, 90]])],
         note="Jeep across at 25 m: 0.12 m per frame for 120 frames, then 0.5 m per frame"),
    dict(clip="temporal_sinai_camp", feature="temporal", keys=["taa"], world="Sinai", mode="S", frames=180,
         date=[2005, 7, 21, 16, 10], vd=2000, stage=stage_of(S_CAMP),
         path=line(S_CAMP, [8350, 2950, 1.75, 338.8 + 18, -3]), props=SINAI_SET,
         note="yaw pan 0.1 deg per frame"),
    dict(clip="lighttiles_town_night", feature="light list", keys=["lightTiles"], world="Abel", mode="S",
         frames=240, date=[1985, 2, 27, 22, 0], vd=800, stage=stage_of(A_TOWN), path=line(A_TOWN, [7144, 8944, 1.75, 345, 3]),
         props=views.TOWN_FIRE + jeep_with_driver("capMover1", 60, -2.5, 180) + jeep_with_driver("capMover2", 5, 2.5, 0),
         movers=[("capMover1", [[0, 60, -2.5, 180], [239, -12, -2.5, 180]]),
                 ("capMover2", [[0, 5, 2.5, 0], [239, 77, 2.5, 0]])],
         refresh=views.FIRE_REFRESH,
         note="two Jeeps with SAFE AI drivers teleported 0.3 m per frame along the street; lane offsets guessed"),
    dict(clip="atmosphere_coastline_sunset", feature="sky table", keys=["atmosphere"], world="Abel", mode="S",
         frames=240, date=[1985, 7, 5, 18, 35], vd=4000, stage=stage_of(A_COAST_SKY),
         path=line(A_COAST_SKY, [1550, 5150, 3, 310, 6])),
    dict(clip="atmosphere_dawn_lapse", feature="sky table", keys=["atmosphere"], world="Abel", mode="S",
         frames=300, date=[1985, 7, 5, 4, 40], vd=4000, stage=stage_of([4400, 3300, 1.75, 60, 4]),
         path=line([4400, 3300, 1.75, 60, 4], [4400, 3300, 1.75, 60, 4]), lapse=12),
    dict(clip="shadows_woods_pine2_evening", feature="shadows", keys=["treeShadows", "softShadows"], world="Abel",
         mode="S", frames=240, date=[1985, 7, 5, 17, 30], vd=1500, stage=stage_of(A_WOODS),
         path=line(A_WOODS, along(A_WOODS, 10, h=225)), props=['[["Jeep"], 15, 2, 70, "veh"] call capSpawn;'],
         note="10 m along instead of 7.3's 20 m: at 20 m the camera is inside a pine crown"),
    dict(clip="shadows_forest_edge_lapse", feature="shadows", keys=["treeShadows", "softShadows"], world="Abel",
         mode="S", frames=240, date=[1985, 7, 5, 17, 30], vd=1500, stage=stage_of(A_FOREST),
         path=line(A_FOREST, A_FOREST), lapse=4500 / 240, props=['[["SoldierWB"], 12, 2, 215, "man"] call capSpawn;']),
    dict(clip="shade_lowsun_fp_orbit", feature="shade", keys=["lowSunRig", "crownCanopy"], world="Abel", mode="S",
         frames=240, date=[1985, 3, 1, 8, 0], vd=1500, stage=LOWSUN_STAGE,
         path=orbit(LOWSUN_STAGE, [16, 5], 12, 1.75, 0, -110, -3), props=views.LOWSUN_FP_PROPS,
         note="orbit at 12 m around the M113 of the lowsun_fp set, 0 to -110 deg (7.3's 0 to 120 runs the "
              "camera into the tent at bearing 99 deg, 10 m from the M113); eye 1.75 and pitch -3 assumed"),
    dict(clip="shade_sinai_orbit", feature="shade", keys=["lowSunRig", "crownCanopy"], world="Sinai", mode="S",
         frames=180, date=[2005, 7, 21, 16, 10], vd=2000, stage=SINAI_STAGE,
         path=orbit(SINAI_STAGE, [16, 5], 12, 1.75, 0, 90, -3), props=SINAI_SET,
         note="orbit around the Zelda of the sinai_lowsun_fp camp; radius 12 assumed"),
    dict(clip="night_road_flat_drive", feature="night", keys=["nightLights", "moonShadows", "stars"], world="Abel",
         mode="S", frames=280, date=[1985, 3, 17, 23, 0], vd=800, stage=stage_of(A_ROAD),
         path=line(A_ROAD, along(A_ROAD, 112)), props=jeep_with_driver("capMover1", 4, 0, 0),
         movers=[("capMover1", [[0, 4, 0, 0], [279, 116, 0, 0]])],
         note="the camera 4 m behind the Jeep, both 0.4 m per frame along the probed flat road"),
    dict(clip="night_moon_crane", feature="night", keys=["nightLights", "moonShadows", "stars"], world="Abel",
         mode="S", frames=240, date=[1985, 3, 1, 20, 0], vd=800, stage=stage_of(A_ROAD_HEAD),
         path=line(A_ROAD_HEAD, [6891.9, 8835.4, 8, 69.6, 25]),
         props=jeep_with_driver("capJeep", 12, 0, 0),
         note="crane and tilt as one eased line (7.3: crane to eye 8, then tilt to +25)"),
    dict(clip="combat_ffh", feature="combat", keys=["combatLights", "combatShadows"], world="Abel", mode="P",
         frames=300, date=[1985, 7, 12, 23, 0], vd=1000, stage=stage_of(A_FFH),
         path=line(A_FFH, [7600, 10860, 2, 0, -3]), props=views.FFH_PROPS, refresh=views.FFH_REFRESH,
         events={0: ['call { private ["_p"]; _p = [30, -35] call capStageRel; '
                     'capFlare = "Flare" createVehicle [_p select 0, _p select 1, 150] }'],
                 60: ['[[27, 1], "Shell120"] call capBoom'],
                 180: ['[[40, -6], "Shell120"] call capBoom']},
         note="static capFFStageN stand-in (posed men, burning Ural, Fire); the fighters' volleys are not staged"),
    dict(clip="water_coastline_sunset", feature="water", keys=["seaWaves"], world="Abel", mode="S", frames=240,
         date=[1985, 7, 5, 18, 35], vd=4000, eyeAbs=True, stage=stage_of(A_COAST_SEA),
         path=line(A_COAST_SEA, [1550, 5170, 3, 290, -4])),
    dict(clip="water_sea_noon", feature="water", keys=["seaWaves"], world="Abel", mode="S", frames=180,
         date=[1985, 7, 5, 12, 30], vd=4000, stage=stage_of(A_SEA_NOON),
         path=line(A_SEA_NOON, strafe(A_SEA_NOON, 15, p=-8))),
    dict(clip="ssr_bay_offshore", feature="ssr", keys=["ssr"], world="Abel", mode="S", frames=240,
         date=[1985, 7, 5, 6, 30], vd=3000, eyeAbs=True, stage=stage_of(A_BAY),
         path=line(A_BAY, [9210, 6951, 3, 262, -3]),
         props=['[["BoatW", "BoatE", "Boat"], 45, 10, -202, "veh"] call capSpawn;']),
    dict(clip="ssr_town_rain", feature="ssr", keys=["ssr"], world="Abel", mode="P", frames=180,
         date=[1985, 7, 5, 19, 30], vd=800, overcast=0.9, rain=1, stage=stage_of(A_TOWN_RAIN_ST),
         path=line(A_TOWN_RAIN_ST, along(A_TOWN_RAIN_ST, 15)), props=['[["Jeep"], 12, 2, 70, "veh"] call capSpawn;'],
         note="pitch 0 assumed (7.3 gives none)"),
    dict(clip="materials_town_night_rain", feature="materials", keys=["wetWeather", "lampStreaks"], world="Abel",
         mode="P", frames=240, date=[1985, 2, 27, 22, 0], vd=800, overcast=0.9, rain=1, stage=stage_of(A_TOWN),
         path=line(A_TOWN, along(A_TOWN, 20)), props=views.TOWN_FIRE, refresh=views.FIRE_REFRESH),
    dict(clip="materials_after_rain", feature="materials", keys=["wetWeather"], world="Abel", mode="S",
         frames=180, date=[1985, 3, 1, 8, 0], vd=1500, stage=LOWSUN_STAGE,
         path=orbit(LOWSUN_STAGE, [16, 5], 12, 1.75, 0, 120, -3), props=views.LOWSUN_FP_PROPS,
         note="7.3 sets wetness=1 in both passes: the knob lands with B7, so this clip waits for it"),
    dict(clip="airbeams_pinewood_morn", feature="air (beams)", keys=["airBeams"], world="Abel", mode="S",
         frames=240, date=[1985, 7, 5, 6, 0], vd=1500, fog=0.1, stage=stage_of(A_PINE_MORN),
         path=line(A_PINE_MORN, along(A_PINE_MORN, 15, h=80))),
    dict(clip="airbeams_pine_eve", feature="air (beams)", keys=["airBeams"], world="Abel", mode="S", frames=180,
         date=[1985, 7, 5, 18, 15], vd=1500, stage=stage_of(A_PINE_EVE), path=line(A_PINE_EVE, strafe(A_PINE_EVE, -10))),
    dict(clip="airbeams_headlights", feature="air (beams)", keys=["airBeams"], world="Abel", mode="S", frames=180,
         date=[1985, 2, 27, 22, 0], vd=800, fog=0.2, stage=stage_of(A_HL_FRONT), path=line(A_HL_FRONT, A_HL_FRONT),
         props=jeep_with_driver("capMover1", 50, 0, 180), movers=[("capMover1", [[0, 50, 0, 180], [179, 5, 0, 180]])],
         note="the Jeep comes toward the camera at 0.25 m per frame; along the road assumed straight"),
    dict(clip="airhaze_valley_lapse", feature="air (haze, valley mist)", keys=["airHaze", "valleyMist"],
         world="Abel", mode="S", frames=300, date=[1985, 7, 5, 5, 15], vd=4000, stage=stage_of(A_VALLEY),
         path=line(A_VALLEY, [6100, 5000, 1.75, 60, -6]), lapse=27),
    dict(clip="hfrt_hill_1730", feature="hfrt", keys=["canopyShadows"], world="Abel", mode="S", frames=300,
         date=[1985, 9, 20, 17, 30], vd=2000, stage=stage_of(A_HILL), path=line(A_HILL, [5440, 6200, 2.5, 180, -8])),
    dict(clip="hfrt_slope_lapse", feature="hfrt", keys=["canopyShadows"], world="Abel", mode="S", frames=240,
         date=[1985, 9, 20, 16, 30], vd=2000, stage=stage_of(A_SLOPE), path=line(A_SLOPE, A_SLOPE), lapse=5400 / 240),
    dict(clip="clouds_coast_oc_up", feature="clouds", keys=["litClouds", "skyCloudShadows"], world="Abel",
         mode="S", frames=240, date=[1985, 7, 5, 18, 35], vd=4000, overcast=0.5, stage=stage_of(A_COAST_UP),
         path=line(A_COAST_UP, [1550, 5150, 3, 300, -4])),
    dict(clip="clouds_broken_high", feature="clouds", keys=["litClouds", "skyCloudShadows"], world="Eden",
         mode="S", frames=240, date=[1985, 7, 5, 14, 30], vd=5000, overcast=0.5, stage=stage_of(E_HIGH),
         path=line(E_HIGH, [9200, 3700, 250, 300, -12]),
         note="7.3 steps the sky drift 0 to 15 deg over the clip; the clouds are held still here"),
    dict(clip="texsharp_forest_edge_walk", feature="texsharp", keys=["terrainDetail", "sharpen"], world="Abel",
         mode="S", frames=240, date=[1985, 7, 5, 18, 15], vd=1500, stage=stage_of([4674, 10156, 1.75, 75, 0]),
         path=line([4674, 10156, 1.75, 75, 0], along([4674, 10156, 1.75, 75, 0], 60))),
    dict(clip="texsharp_meadow", feature="texsharp", keys=["terrainDetail", "sharpen"], world="Eden", mode="S",
         frames=180, date=[1985, 7, 5, 14, 30], vd=3500, overcast=0.5, stage=stage_of(E_MEADOW),
         path=line(E_MEADOW, along(E_MEADOW, 30)), props=['[["Jeep"], 12, -5, 70, "veh"] call capSpawn;']),
]


def path_def(c, n):
    kind, a, b = c["path"]
    return (f'call {{ capPathKind = "{kind}"; capPathA = [{", ".join(capgen.num(x) for x in a)}]; '
            f'capPathB = [{", ".join(capgen.num(x) for x in b)}]; capN = {n}; capLapse = {capgen.num(c.get("lapse", 0))} }}')


def frame_label(c, pas, k):
    return f"{c['clip']}__{pas}__f{k + 1:04d}"


def stage_block(c, n):
    v = dict(c["stage"], date=c["date"], overcast=c.get("overcast", 0), fog=c.get("fog", 0),
             rain=c.get("rain", 0), vd=c["vd"], eyeAbs=c.get("eyeAbs", False), props=c.get("props", []))
    out = capgen.begin(v, c["clip"])
    out.append("[] call capStageSave")
    out.append("call { capMovers = [] }")
    for var, keys in c.get("movers", []):
        ks = ", ".join("[" + ", ".join(capgen.num(x) for x in k) + "]" for k in keys)
        out.append(f"[{var}, [{ks}]] call capMoverAdd")
    out.append(path_def(c, n))
    return out


def off_setting(c):
    """The "off" pass's triSetLighting word: the clip's own (the opening's
    "baseline") or the shipped defaults with the clip's switch keys at 0."""
    return c.get("off") or "remastered," + ",".join(f"{k}=0" for k in c["keys"])


def s_mode(c, n):
    off = off_setting(c)
    out = ["triSimFrames 150"]
    for pas, setting in (("off", off), ("on", "remastered")):
        out += [f'"{setting}" call capMode', "[] call capPin"]
        for k in range(n):
            for sqf in c.get("events", {}).get(k, []):
                out.append(sqf)
            out += [f"[{k}] call capClockAt", f"[{k}, {n}] call capPathAt", f"[{k}] call capPropsAt"]
            if c.get("refresh"):
                out.append(c["refresh"])
            out += ["triSimFrames 2", f'triScreenshot "{frame_label(c, pas, k)}"',
                    f'["{frame_label(c, pas, k)}", {k}] call capFrameLog']
    return out


def p_mode(c, n):
    off = off_setting(c)
    out = ["triSimFrames 150", '"remastered" call capMode', "[] call capPin",
           f"call {{ capFixRes = triSetFixedStep {capgen.num(1.0 / FPS)} }}"]
    for k in range(n):
        for sqf in c.get("events", {}).get(k, []):
            out.append(sqf)
        out += ["call { capPauseRes = triPauseGame }", f"[{k}, {n}] call capPathAt", f"[{k}] call capPropsAt"]
        for pas, setting in (("off", off), ("on", "remastered")):
            out += [f'"{setting}" call capModeQuick', "triSimFrames 6", f'triScreenshot "{frame_label(c, pas, k)}"',
                    f'["{frame_label(c, pas, k)}", {k}] call capFrameLog']
        out += ["call { capPauseRes = triUnpauseGame }", "triSimFrames 1"]
    out.append("call { capFixRes = triSetFixedStep 0 }")
    return out


def write_clip(c, frames, out_dir):
    n = frames or c["frames"]
    title = f"Video clip {c['clip']} ({c['feature']}, {c['mode']} mode, {n} frames): gen_video_tests.py"
    first = c["stage"]
    lines = [capgen.header(c["world"], first, title, include="../capture_lib.sqf",
                           extra_includes=("capture_motion.sqf",))]
    lines += [f"\n// ---- {c['clip']}" + (f" ({c['note']})" if c.get("note") else "")]
    lines += stage_block(c, n)
    lines += s_mode(c, n) if c["mode"] == "S" else p_mode(c, n)
    lines += capgen.end(c)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    capgen.write(out_dir / f"{c['clip']}.test.sqf", "\n".join(lines) + "\n")
    # 7.1: toml timeout 3600, assert_timeout 180
    capgen.write(out_dir / f"{c['clip']}.test.toml",
                 capgen.toml(c["world"], title, views.WORLDS, timeout=3600, assert_timeout=180))
    return n


def write_paused_check(out_dir):
    """Plan 7.2: two paused renders of a burning fire must be identical for P
    mode to pair its shots. Shoots a fire and a burning Ural at night: two
    frames 6 frames apart while paused (with a lighting switch and back
    between them, as P mode does), then two while running."""
    c = next(c for c in CLIPS if c["clip"] == "combat_ffh")
    name = "paused_fire_check"
    title = f"{name}: do paused frames hold fire and smoke still? gen_video_tests.py --paused-check"
    lines = [capgen.header(c["world"], c["stage"], title, include="../capture_lib.sqf",
                           extra_includes=("capture_motion.sqf",))]
    lines += stage_block(dict(c, clip=name), 1)
    lines += ["triSimFrames 150", '"remastered" call capMode', "[] call capPin",
              f"call {{ capFixRes = triSetFixedStep {capgen.num(1.0 / FPS)} }}",
              "call { capPauseRes = triPauseGame }",
              "triSimFrames 6", f'triScreenshot "{name}__paused_a"', f'["{name}__paused_a", 0] call capFrameLog',
              '"baseline" call capModeQuick', "triSimFrames 6", f'triScreenshot "{name}__paused_baseline"',
              f'["{name}__paused_baseline", 1] call capFrameLog',
              '"remastered" call capModeQuick', "triSimFrames 6", f'triScreenshot "{name}__paused_b"',
              f'["{name}__paused_b", 2] call capFrameLog',
              "call { capPauseRes = triUnpauseGame }",
              "triSimFrames 6", f'triScreenshot "{name}__running_c"', f'["{name}__running_c", 3] call capFrameLog',
              "triSimFrames 6", f'triScreenshot "{name}__running_d"', f'["{name}__running_d", 4] call capFrameLog',
              "call { capFixRes = triSetFixedStep 0 }"]
    lines += capgen.end(c)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    capgen.write(out_dir / f"{name}.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out_dir / f"{name}.test.toml", capgen.toml(c["world"], title, views.WORLDS, timeout=1800,
                                                         assert_timeout=180))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--clip", action="append", default=[])
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--frames", type=int, default=0, help="override the clip length (short test runs)")
    ap.add_argument("--list", action="store_true")
    ap.add_argument("--paused-check", action="store_true")
    ap.add_argument("--out", default=str(HERE))
    a = ap.parse_args()
    out = Path(a.out)
    out.mkdir(parents=True, exist_ok=True)
    if a.list:
        for c in CLIPS:
            print(f"{c['clip']:32s} {c['feature']:26s} {c['world']:6s} {c['mode']} {c['frames']:4d}  "
                  f"off: {','.join(c['keys'])}")
        return 0
    if a.paused_check:
        write_paused_check(out)
        print("paused_fire_check written")
    chosen = CLIPS if a.all else [c for c in CLIPS if c["clip"] in a.clip]
    missing = set(a.clip) - {c["clip"] for c in CLIPS}
    if missing:
        raise SystemExit(f"unknown clip(s): {sorted(missing)}; see --list")
    for c in chosen:
        n = write_clip(c, a.frames, out)
        print(f"{c['clip']}: {c['mode']} mode, {n} frames")
    return 0


if __name__ == "__main__":
    sys.exit(main())
