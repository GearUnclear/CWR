"""Generate the v2 comparison-video clip tests: the vanilla upstream game on the
left, Uslu dur! with every feature on (triSetLighting "remastered") on the
right, one clip per showcased feature (the owner, 2026-10-19: "vanilla game on
left and all features on on the right with clips that specifically showcase
each"; "for the video it is expected to just load the vanilla upstream repo").

    python gen_video_v2.py --list
    python gen_video_v2.py [--clip <clip> ...] [--frames N] [--side ud|vanilla|both]

Two sets, the same camera paths, clocks, props, events and frame counts, so the
halves line up frame for frame:

  ud       video/ud_<clip>.test.sqf next to this file (graphics-remaster
           tooling; runs on any fork build from A1 on, the final overhaul
           build included): "remastered" only, no off pass, the presented
           frame (triCaptureAtPresent), the clouds held (triPinCloudDrift)
  vanilla  <vanilla-upstream>/tests/rendering/captures/video/van_<clip>.test.sqf
           with the vanilla capture_lib.sqf and capture_motion.sqf there: only
           verbs the vanilla 3.05 engine has (checked), the frame read before
           the gamma pass (collect_vanilla.py applies gamma 1.2 offline), the
           LoBo test bed mounted without its Bin (see tmp/look/ref/NOTES.md)

Both sides use S mode (plan section 7.2): per frame the sim clock is pinned to
SIM_T + k/30 (triSetSimTime), the camera is put on the path, movers are
teleported, events fire on their frame, then triSimFrames 2 and the shot.
P mode cannot pair two builds, so fire, smoke, rain and flare particles and
anything AI does are free to differ between the halves; the props are captive
and only move by teleport. Frames are labelled <clip>__ud__f0001 and
<clip>__vanilla__f0001.
"""
import argparse
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))
sys.path.insert(0, str(HERE))

import capgen  # noqa: E402
import views  # noqa: E402
from gen_video_tests import (A_BAY, A_FFH, A_FOREST, A_PINE_EVE, A_PINE_MORN, A_ROAD, A_TOWN, E_HIGH,  # noqa: E402
                             LOWSUN_STAGE, SINAI_SET, SINAI_STAGE, along, line, orbit, rel, stage_of, strafe)

VANILLA_ROOT = Path("D:/Arma_CWA_worktrees/vanilla-upstream")
VANILLA_DIR = VANILLA_ROOT / "tests/rendering/captures/video"
VANILLA_LOBO = "D:/Arma_CWA_worktrees/vanilla-upstream/tmp/lobo-mods"  # @LoBo addons + dta, no Bin
FORK_ONLY = ["triPinCloudDrift", "triGpuTimes", "triSettleScene", "triSetLighting", "triSceneStats",
             "triPinObjectDetail", "triFeatureState", "triCaptureAtPresent", "triSetFixedStep", "triCamView"]
# The owner, 2026-10-19: "record 60fps this is a youtube video". The clip table
# keeps its frame counts, movers and events at 30 a second (BASE_FPS); every
# clip is shot at FPS frames a second for the same length, the keys scaled.
FPS = 60
BASE_FPS = 30
# Every clip pins the sim clock here (sea, water and anything keyed on
# Glob.time start from the same phase in both builds).
SIM_T = 2000

HELPERS = r'''
// 60 frames a second: the sim clock steps 1/60 s per frame (capClockAt).
capFps = 60
// [forward, right, height, [vx, vy, vz], ammo] call capShellAt: a shell (or a
// flare) of that ammo fired from a point relative to the staging view
// (createShell, an upstream verb).
capShellAt = {
    private ["_p"];
    _p = [_this select 0, _this select 1] call capStageRel;
    capShellObj = createShell [_this select 4, [_p select 0, _p select 1, _this select 2], _this select 3, player, objNull];
    format ["view=%1;event=shell;ammo=%2;pos=%3;object=%4", capView, _this select 4, _p, capShellObj] call capLog;
}
// [] call capPinProps: every prop back on its spawn point, at rest. A shell
// burst throws captive men tens of metres (unhurt, allowDamage false); the
// combat clip re-pins them every frame.
capPinProps = {
    { (_x select 0) setVelocity [0, 0, 0]; (_x select 0) setDir (_x select 3); (_x select 0) setPos [_x select 1, _x select 2, 0] } forEach capProps;
}
// The resistance side (Guerrilla Mode's player side) for the guerrilla props.
createCenter resistance
capGrpG = createGroup resistance
capKneel = []
// [unit, unitPos, behaviour] call capPose: a posed man kneels ("MIDDLE"), lies
// ("DOWN") or stands, weapon lowered ("SAFE") or raised ("AWARE", "COMBAT").
capPose = {
    if (not (isNull (_this select 0))) then { (_this select 0) setUnitPos (_this select 1); (_this select 0) setBehaviour (_this select 2) };
}
'''

# The head fix (tmp/video/heads/RECIPE.txt, from the head board, 2026-10-19): (A) every video
# frame advances the world by exactly 1/60 s (the world frozen with setAccTime 0 between shots,
# one tick per shot inside ONE statement; triSetFixedStep 1/60 on the fork, a 60-frame render
# pump plus setAccTime 1/18 against the engine's 0.3 s tick cap on vanilla), and (B) every posed
# man doWatches a point 300 m straight ahead at eye level, ordered in the staging block before
# the settle frames. Capture staging only: no engine, config or game change.
GAZE_HOLD = ('call { { if (("Man" countType [_x select 0]) == 1) then { '
             'capWg0 = [_x select 1, _x select 2] call capGround; '
             'capWx1 = (_x select 1) + 300 * (sin (_x select 3)); '
             'capWz1 = (_x select 2) + 300 * (cos (_x select 3)); '
             'capWg1 = [capWx1, capWz1] call capGround; '
             '(_x select 0) doWatch [capWx1, capWz1, 1.6 + capWg0 - capWg1] } } forEach capProps }')
STEP = {"fork": "call { setAccTime 1; triSimFrames 1; setAccTime 0 }",
        "vanilla": "call { triWaitFrames 60; setAccTime (1/18); triSimFrames 1; setAccTime 0 }"}
FREEZE = {"fork": "call { capStepRes = triSetFixedStep (1/60); setAccTime 0 }", "vanilla": "call { setAccTime 0 }"}
# Kneeling: setUnitPos "MIDDLE" does nothing on either build (RECIPE.txt section 7); a pose
# ("MIDDLE", ...) is kept "UP" and gets switchMove "Crouch" after capPin instead.
KNEEL = 'call { { if (not (isNull _x)) then { _x switchMove "Crouch" } } forEach capKneel }'

A_DAWN = [6100, 5000, 1.75, 28, -6]
A_CAMP = [8350, 2950, 1.75, 332, -3]
A_TOWN_RAIN = [7152, 8915, 1.4, 345, 1]
A_DRIVE = [A_ROAD[0], A_ROAD[1], 2.5, A_ROAD[3], -4]
A_CHECK = [7266.4, 8355.9, 1.7, 200, 0]
OUTPOST_STAGE = LOWSUN_STAGE


def man(cls, forward, right, rdir, grp="capGrpW", pose=None, var="capU"):
    """A posed captive man (capSpawn kind "man"); pose = (unitPos, behaviour). "MIDDLE" kneels
    by switchMove "Crouch" after capPin (KNEEL); setUnitPos stays "UP"."""
    out = [f'{var} = [["{cls}"], {forward}, {right}, {rdir}, "man", {grp}] call capSpawn;']
    if pose:
        unit_pos = "UP" if pose[0] == "MIDDLE" else pose[0]
        out.append(f'[{var}, "{unit_pos}", "{pose[1]}"] call capPose;')
        if pose[0] == "MIDDLE":
            out.append(f'capKneel = capKneel + [{var}];')
    return out


def veh(classes, forward, right, rdir, var="capV"):
    cl = ", ".join(f'"{c}"' for c in classes)
    return [f'{var} = [[{cl}], {forward}, {right}, {rdir}, "veh"] call capSpawn;']


def driven(var, classes, driver, grp, forward, right, rdir):
    """A vehicle with a SAFE captive driver (headlights at night), MOVE off: it moves only by teleport."""
    cl = ", ".join(f'"{c}"' for c in classes)
    return [f'{var} = [[{cl}], {forward}, {right}, {rdir}, "veh"] call capSpawn;',
            f'if (not (isNull {var})) then {{ {var} lock false; "{driver}" createUnit [[capStageX, capStageZ, 0], {grp}, '
            f'"capNewUnit = this", 0.6, "PRIVATE"]; capNewUnit setCaptive true; capNewUnit allowDamage false; '
            f'capNewUnit moveInDriver {var}; capNewUnit disableAI "MOVE"; capNewUnit setBehaviour "SAFE" }};']


def at(stage, forward, right, eye, h, p):
    """A camera key [east, north, eye, heading, pitch] at [forward, right] from a stage view."""
    x, z = rel(stage[0], stage[1], stage[3], forward, right)
    return [round(x, 2), round(z, 2), eye, h, p]


def paced(k1, f0, f1, right, rdir, steps=20):
    """Mover keys from forward f0 (frame 0) to f1 (table frame k1) on the camera's own pace: the
    camera path eases in and out (capEase, smoothstep), a two-key mover runs at an even speed,
    so a mover meant to hold a distance to the camera needs the eased curve, here in `steps`
    linear pieces (off by under 0.2 m on a 96 m run)."""
    def ease(t):
        return t * t * (3 - 2 * t)
    return [[k1 * i / steps, round(f0 + (f1 - f0) * ease(i / steps), 3), right, rdir] for i in range(steps + 1)]


# One dict per clip, in video order (first light to the last round). Every clip
# is a scene of the Uslu dur! world (Guerrilla Mode: a resistance against an
# occupying army) staged so its lighting feature shows. Base-game classes only
# (resistance SoldierG*, occupier SoldierE*/BMP/UAZ/Ural), so the vanilla build
# shows the same scene. title is the title card (2 to 4 plain words); shows is
# what the clip is staged to show (the narration and CLIPS.txt read it).
# movers: (SQF var, [[frame, forward, right, relative dir], ...]) from the
# stage; events: frame -> SQF; drift: the ud side steps the cloud-shadow field
# this many metres [east, north] a frame.
CLIPS = [
    dict(clip="opening", title="Malden, under occupation", world="Abel", frames=495, key_base=180,
         date=[1985, 7, 5, 18, 15], vd=1500, stage=stage_of(A_PINE_EVE),
         path=line(A_PINE_EVE, strafe(A_PINE_EVE, -10, h=300, p=3)),
         props=veh(["BMP"], 23, 4, 250) + man("SoldierEB", 14, -3, 160, "capGrpE")
         + man("SoldierEB", 16, -1.5, 200, "capGrpE"),
         shows="establishing shot: evening sun behind a cypress row on occupied Malden, an occupier BMP and two "
               "sentries; sun beams, long soft shadows, camera-like exposure"),
    dict(clip="valley_mist", title="Moving out at dawn", world="Abel", frames=285, key_base=210,
         date=[1985, 7, 1, 5, 45], vd=4000, stage=stage_of(A_DAWN),
         path=line(along(A_DAWN, -9), along(A_DAWN, -9, h=48, p=-5)),
         props=man("SoldierGB", -4.5, -3, 10, "capGrpG", ("UP", "AWARE"))
         + man("SoldierGMG", -3.5, 1.5, 20, "capGrpG")
         + man("SoldierGB", -1.5, 4.5, 30, "capGrpG", ("MIDDLE", "AWARE"))
         + man("SoldierGLAW", -2.5, -6.5, 5, "capGrpG"),
         shows="a resistance squad on the ridge above the dawn valley: mist in the valley and the bay lit by the "
               "rising sun, haze fading the far hills"),
    dict(clip="pine_shafts", title="Through the pines", world="Abel", frames=285, key_base=210,
         date=[1985, 7, 5, 6, 0], vd=1500, fog=0.1, stage=stage_of(A_PINE_MORN),
         path=line(A_PINE_MORN, along(A_PINE_MORN, 10, h=78)),
         props=man("SoldierGB", 15, -3, 30, "capGrpG", ("MIDDLE", "AWARE")) + man("SoldierGB", 19, 2, 40, "capGrpG")
         + man("SoldierGMG", 23, -1, 35, "capGrpG", ("MIDDLE", "AWARE")),
         shows="guerrillas among the pines in morning mist: volumetric fog and sun shafts through the trees"),
    dict(clip="bay_reflections", title="Coastal patrol, dawn", world="Abel", frames=285, key_base=210,
         date=[1985, 7, 5, 6, 30], vd=3000, eyeAbs=True, stage=stage_of(A_BAY),
         path=line(A_BAY, along(A_BAY, 12, h=279)),
         props=veh(["BoatE", "BoatW", "Boat"], 45, 10, -202),
         shows="an occupier patrol boat on the calm bay at dawn: the hill and the boat mirrored in the water "
               "(screen-space reflections), sea waves"),
    dict(clip="outpost", title="Occupier outpost", world="Abel", frames=300, key_base=210,
         date=[1985, 3, 1, 8, 0], vd=1500, stage=OUTPOST_STAGE,
         path=orbit(OUTPOST_STAGE, [16, 5], 13, 1.75, 0, -90, -3),
         props=veh(["UAZ", "Jeep"], 10, -5, -107.5) + veh(["BMP", "M113"], 16, 5, 72.5)
         + man("SoldierEB", 7, 1, -112.5, "capGrpE") + veh(["CampEast", "Camp"], 22, -3, -152.5)
         + man("SoldierEMG", 20, 9, 20, "capGrpE"),
         shows="an occupier outpost in a low winter-morning sun, orbiting a BMP with a UAZ, a tent and two "
               "sentries: rim light, deep soft shade, ambient occlusion and specular on the hulls"),
    dict(clip="cloud_shadows", title="Everon from the air", world="Eden", frames=255, key_base=210,
         date=[1985, 7, 5, 14, 30], vd=5000, overcast=0.5, stage=stage_of(E_HIGH),
         path=line(E_HIGH, [9200, 3700, 250, 300, -12]), drift=(2.5, 1.0),
         shows="a recon view of Everon from 250 m: lit clouds and their shadows sweeping over the hills (the ud "
               "side steps the cloud-shadow field 2.5 m east, 1 m north a frame, a gentle time-lapse)"),
    dict(clip="sinai_camp", title="Hidden camp, Sinai", world="Sinai", frames=270, key_base=210,
         date=[2005, 7, 21, 16, 10], vd=2000, stage=SINAI_STAGE,
         path=line(A_CAMP, along(A_CAMP, 3, h=352)), props=SINAI_SET,
         shows="a camp under camouflage netting on Sinai, LoBo's IDF HMMWV, Zelda and rifleman, in hard late "
               "sun: shade under the net, warm sand; a modded island with its own factions"),
    dict(clip="ambush", title="Ambush at golden hour", world="Abel", frames=285, key_base=210,
         date=[1985, 7, 5, 18, 15], vd=1500, stage=stage_of(A_FOREST),
         path=line(A_FOREST, strafe(A_FOREST, 8, h=82)),
         props=man("SoldierGB", 8, -1.5, 40, "capGrpG", ("MIDDLE", "COMBAT"))
         + man("SoldierGLAW", 10, 2.5, 50, "capGrpG", ("MIDDLE", "COMBAT"))
         + man("SoldierGMG", 12, -4, 35, "capGrpG", ("DOWN", "COMBAT"))
         + driven("capMover1", ["UAZ", "Jeep"], "SoldierEB", "capGrpE", 25, -15, 90),
         movers=[("capMover1", [[0, 25, -15, 90], [209, 25, 12, 90]])],
         shows="guerrillas waiting at a forest edge at golden hour as an occupier UAZ crawls across the slope: soft "
               "tree shadows and long canopy shadows, terrain detail"),
    dict(clip="convoy", title="Convoy at sunset", world="Abel", frames=270, key_base=240,
         date=[1985, 7, 5, 18, 25], vd=2500, stage=stage_of(A_ROAD),
         path=line(at(A_ROAD, 104, -2.5, 1.8, 258, 1), at(A_ROAD, 103, -2.5, 1.8, 252, 0)),
         props=driven("capMover1", ["UAZ", "Jeep"], "SoldierEB", "capGrpE", 30, 1.5, 0)
         + driven("capMover2", ["Ural", "Truck5t"], "SoldierEB", "capGrpE", 16, 1.5, 0)
         + driven("capMover3", ["Ural", "Truck5t"], "SoldierEB", "capGrpE", 2, 1.5, 0),
         movers=[("capMover1", [[0, 30, 1.5, 0], [239, 90, 1.5, 0]]),
                 ("capMover2", [[0, 16, 1.5, 0], [239, 76, 1.5, 0]]),
                 ("capMover3", [[0, 2, 1.5, 0], [239, 62, 1.5, 0]])],
         shows="an occupier convoy (UAZ and two Urals) coming down a country road into a low sunset: backlight, "
               "long shadows, warm sky"),
    dict(clip="checkpoint", title="Checkpoint at dusk", world="Abel", frames=270, key_base=210,
         date=[1985, 7, 5, 19, 25], vd=1500, stage=stage_of(A_CHECK),
         path=line(along(A_CHECK, 6), along(A_CHECK, 16, p=-1)),
         props=veh(["Fortress1"], 34, -6, 0) + veh(["Fortress1"], 34, 6, 0) + veh(["Ural", "Truck5t"], 41, -9, 20)
         + veh(["UAZ", "Jeep"], 29, 7.5, 150) + man("SoldierEB", 26, -3, 150, "capGrpE")
         + man("SoldierEB", 27.5, 1.5, 210, "capGrpE") + man("OfficerE", 25, 4, 190, "capGrpE")
         + ['capFire = [["Fire"], 27, -1.5, 0, "veh"] call capSpawn;',
            'if (not (isNull capFire)) then { capFire inflame true };'],
         refresh=views.FIRE_REFRESH,
         shows="an occupier checkpoint on a road at dusk: sandbag bunkers, a parked Ural and UAZ, sentries by a "
               "fire; twilight sky, fire light on the men"),
    dict(clip="town_patrol", title="Night patrol in town", world="Abel", frames=285, key_base=210,
         # a short push across the square to the right of the fire, 7 m or more from the
         # sentries and 10 m from the birch (the old path ran up the stage line through the birch
         # and past the house corner; 2026-10-19 near-object check)
         date=[1985, 2, 27, 22, 0], vd=800, stage=stage_of(A_TOWN),
         path=line(at(A_TOWN, -4, 6, 1.75, 340, 3), at(A_TOWN, 2, 8.5, 1.75, 335, 2)),
         props=views.TOWN_FIRE + man("SoldierEB", 10, 5.5, 230, "capGrpE") + man("SoldierEB", 7.5, 1.5, 60, "capGrpE")
         + driven("capMover1", ["UAZ", "Jeep"], "SoldierEB", "capGrpE", 75, -2.5, 180)
         + veh(["Ural", "Truck5t"], 30, 6, 160),
         movers=[("capMover1", [[0, 75, -2.5, 180], [209, 38, -2.5, 180]])],
         refresh=views.FIRE_REFRESH,
         shows="the occupied town at night: street lamps lighting the street, two occupiers at a fire, a parked "
               "Ural, a patrol UAZ coming down the street with its headlights"),
    dict(clip="rain_town", title="Town in the rain", world="Abel", frames=285, key_base=210,
         date=[1985, 2, 27, 22, 0], vd=800, overcast=0.9, rain=1, stage=stage_of(A_TOWN),
         # 3.5 m right of the stage line and 2 m shorter: the cypress by the house corner stays
         # 7 m off (2026-10-19 near-object check)
         path=line(at(A_TOWN_RAIN, 14, 3.5, 1.4, 345, 1), at(A_TOWN_RAIN, 24, 3.5, 1.4, 341, 0)),
         props=driven("capMover1", ["UAZ", "Jeep"], "SoldierEB", "capGrpE", 55, -2.5, 180)
         + man("SoldierGB", 29, 6, 200, "capGrpG", ("MIDDLE", "COMBAT"))
         + man("SoldierGB", 31, 6.8, 190, "capGrpG", ("MIDDLE", "COMBAT")),
         movers=[("capMover1", [[0, 55, -2.5, 180], [209, 34, -2.5, 180]])],
         shows="guerrillas crouched at the roadside in the rainy night town as an occupier patrol UAZ passes: wet "
               "asphalt, puddles, lamp and headlight reflections"),
    dict(clip="supply_run", title="Supply run at night", world="Abel", frames=300, key_base=240,
         date=[1985, 3, 17, 23, 0], vd=800, stage=stage_of(A_ROAD), path=line(A_DRIVE, along(A_DRIVE, 96)),
         props=driven("capMover1", ["Skoda", "SkodaBlue", "Jeep"], "SoldierGB", "capGrpG", 7, 0, 0),
         # the Skoda keeps its 7 m lead on the eased camera (two linear keys let the camera run
         # through the car mid-clip, 2026-10-19 near-object check)
         movers=[("capMover1", paced(239, 7, 103, 0, 0))],
         shows="a resistance supply run on a moonless country road, 7 m behind a civilian Skoda: low beams lighting "
               "the road, a sky full of stars"),
    dict(clip="firefight", title="Firefight after dark", world="Abel", frames=315, key_base=240,
         date=[1985, 7, 12, 23, 0], vd=1000, stage=stage_of(A_FFH),
         path=line(A_FFH, [7600, 10860, 2, 0, -3]),
         props=['{ [["SoldierGB"], _x select 0, _x select 1, 70, "man", capGrpG] call capSpawn } forEach [[13, -7], '
                '[15.5, -4.5], [12.5, -2.5]];',
                '{ [["SoldierEB"], _x select 0, _x select 1, -110, "man", capGrpE] call capSpawn } forEach [[16, 3.5], '
                '[18, 6], [15, 8.5]];',
                'capBurnV = [["Ural", "Truck5t"], 36, 13, 30, "veh"] call capSpawn;',
                'if (not (isNull capBurnV)) then { capBurnV allowDamage true; capBurnV setDammage 1; capBurnV inflame '
                'true };',
                'capBurnB = [["BMP"], 30, -24, 120, "veh"] call capSpawn;',
                'if (not (isNull capBurnB)) then { capBurnB allowDamage true; capBurnB setDammage 1; capBurnB inflame '
                'true };',
                'capFire = [["Fire"], 21, -12, 0, "veh"] call capSpawn;',
                'if (not (isNull capFire)) then { capFire inflame true };'],
         refresh=views.FFH_REFRESH,
         events={0: ['[30, -35, 45, [0, 0, 12], "Flare"] call capShellAt'],
                 36: ['[42, -18, 40, [0, 0, -250], "Shell120"] call capShellAt'],
                 116: ['[48, 12, 40, [0, 0, -250], "Shell120"] call capShellAt'],
                 181: ['[38, 26, 40, [0, 0, -250], "Shell120"] call capShellAt']},
         pin_props=True,
         shows="resistance fighters and occupiers at night between a burning Ural and a burning BMP: a fire, a "
               "flare and three shell bursts lighting the men, with shadows; the bursts land 25 to 35 m from the "
               "men, who are re-pinned every frame (a burst throws them)"),
]


def clip_by_name(name):
    for c in CLIPS:
        if c["clip"] == name:
            return c
    raise KeyError(name)


def num(v):
    return capgen.num(v)


def frame_label(c, side, k):
    return f"{c['clip']}__{side}__f{k + 1:04d}"


def clip_frames(c):
    """Frames the clip is shot with at FPS (its table count is at BASE_FPS)."""
    return c["frames"] * FPS // BASE_FPS


def clip_seconds(c):
    return c["frames"] / BASE_FPS


def scale_key(c, n, k):
    """A table frame number on a run of n frames: the same place in the clip. Keys are
    written for key_base frames (the clip's length when they were staged)."""
    full = c.get("key_base", c["frames"])
    if full <= 1:
        return k
    return int(round(k * (n - 1) / (full - 1)))


def stage_block(c, n, side):
    v = dict(c["stage"], date=c["date"], overcast=c.get("overcast", 0), fog=c.get("fog", 0),
             rain=c.get("rain", 0), vd=c["vd"], eyeAbs=c.get("eyeAbs", False), props=c.get("props", []))
    out = capgen.begin(v, c["clip"], legacy=(side == "vanilla"), sim_time=SIM_T)
    out.append("[] call capStageSave")
    out.append(GAZE_HOLD)
    out.append("call { capMovers = [] }")
    for var, keys in c.get("movers", []):
        keys = [[scale_key(c, n, k[0])] + list(k[1:]) for k in keys]
        assert keys[-1][0] <= n - 1, f"{c['clip']}: mover key past the last frame"
        ks = ", ".join("[" + ", ".join(num(x) for x in k) + "]" for k in keys)
        out.append(f"[{var}, [{ks}]] call capMoverAdd")
    kind, a, b = c["path"]
    out.append(f'call {{ capPathKind = "{kind}"; capPathA = [{", ".join(num(x) for x in a)}]; '
               f'capPathB = [{", ".join(num(x) for x in b)}]; capN = {n}; capLapse = 0 }}')
    return out


# The vanilla side waits in short statements: at its finest object detail
# (see run_v2_lane.ps1, frameRate=3) vanilla draws these views at about 10 fps,
# and a single long triSimFrames (the lib's 450-frame settle took 40 s) left
# tri waiting forever for the next statement. 90-frame steps stay near 10 s.
VANILLA_STEP = 90


def frames_block(c, n, side):
    fork = side in ("ud", "pv")
    mode = "remastered" if fork else "vanilla"
    if fork:
        # The same short statements on the ud side: the settle (triSettleScene)
        # is capped at 150 frames and the rest of the wait comes first in
        # 90-frame steps, in case the final build draws as slowly as vanilla.
        out = ["call { capSettleArgs = [150, 30, 2] }"] + [f"triSimFrames {VANILLA_STEP}"] * 4
        out += [f'"{mode}" call capMode', "[] call capPin", KNEEL, FREEZE["fork"]]
    else:
        out = [f"call {{ capSettleFrames = {VANILLA_STEP} }}"] + [f"triSimFrames {VANILLA_STEP}"] * 6
        out += [f'"{mode}" call capMode', "[] call capPin", KNEEL, FREEZE["vanilla"]]
    drift = c.get("drift") if fork else None
    events = {}
    for k, sqfs in c.get("events", {}).items():
        events.setdefault(scale_key(c, n, k), []).extend(sqfs)
    for k in range(n):
        for sqf in events.get(k, []):
            out.append(sqf)
        out += [f"[{k}] call capClockAt", f"[{k}, {n}] call capPathAt", f"[{k}] call capPropsAt"]
        if c.get("refresh"):
            out.append(c["refresh"])
        if c.get("pin_props"):
            out.append("[] call capPinProps")
        if drift:
            kk = k * (c.get("key_base", c["frames"]) - 1) / max(1, n - 1)
            out.append(f"call {{ capDriftRes = triPinCloudDrift [{capgen.CLOUD_DRIFT[0] + drift[0] * kk:g}, "
                       f"{capgen.CLOUD_DRIFT[1] + drift[1] * kk:g}, {capgen.CLOUD_DRIFT[2]}] }}")
        lab = frame_label(c, side, k)
        out += [STEP["fork" if fork else "vanilla"], f'triScreenshot "{lab}"', f'["{lab}", {k}] call capFrameLog']
    return out


def vanillaize(text):
    out = []
    for l in text.split("\n"):
        if l.strip() == "triSimFrames 240":
            out += ["triSimFrames 120", "triSimFrames 120"]
            continue
        if l.strip() == "triCaptureAtPresent true":
            out.append("// (vanilla: no triCaptureAtPresent; collect_vanilla.py applies the gamma pass, 1.2, offline)")
            continue
        out.append(l)
    text = "\n".join(out)
    code = "\n".join(x.split("//")[0] for x in text.split("\n"))
    for v in FORK_ONLY:
        assert v not in code, f"fork-only verb left: {v}"
    return text


def split_long_waits(text):
    """The header's 240-frame boot wait as two statements (see VANILLA_STEP)."""
    nl = chr(10)
    return text.replace(nl + "triSimFrames 240" + nl, nl + "triSimFrames 120" + nl + "triSimFrames 120" + nl)


def write_clip(c, frames, side):
    n = frames or clip_frames(c)
    tag = {"ud": "ud", "pv": "pv", "vanilla": "van"}[side]
    name = f"{tag}_{c['clip']}"
    who = {"ud": "Uslu dur!, everything on (remastered)",
           "pv": "framing preview on the fork (remastered), labels __pv__",
           "vanilla": "vanilla upstream 3.05 (vanilla-safe verbs only)"}[side]
    title = f"Video v2 clip {c['clip']}, {who}: {n} frames at {FPS} fps, S mode. gen_video_v2.py"
    lines = [capgen.header(c["world"], c["stage"], title, include="../capture_lib.sqf",
                           extra_includes=("capture_motion.sqf",)), HELPERS]
    lines.append(f"// ---- {c['clip']}: {c['title']} ({c['shows']})")
    lines += stage_block(c, n, side)
    lines += frames_block(c, n, side)
    lines += capgen.end(c, legacy=(side == "vanilla"))
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    text = "\n".join(lines) + "\n"
    toml_text = capgen.toml(c["world"], title, views.WORLDS, timeout=3600, assert_timeout=180)
    text = split_long_waits(text)
    if side == "vanilla":
        text = vanillaize(text)
        toml_text = toml_text.replace('"--mods-dir", "."', f'"--mods-dir", "{VANILLA_LOBO}"')
        out_dir = VANILLA_DIR
    else:
        out_dir = HERE
    out_dir.mkdir(parents=True, exist_ok=True)
    capgen.write(out_dir / f"{name}.test.sqf", text)
    capgen.write(out_dir / f"{name}.test.toml", toml_text)
    return out_dir / f"{name}.test.sqf", n


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--clip", action="append", default=[])
    ap.add_argument("--frames", type=int, default=0, help="override the clip length (short test runs)")
    ap.add_argument("--side", choices=("ud", "vanilla", "both", "pv"), default="both",
                    help="pv: framing previews pv_<clip> on the fork (use with --frames 3)")
    ap.add_argument("--list", action="store_true")
    a = ap.parse_args()
    if a.list:
        t = 0
        for i, c in enumerate(CLIPS, 1):
            t += c["frames"]
            print(f"{i:2d} {c['clip']:16s} {c['world']:6s} {c['frames']:4d} f  {c['title']}")
        print(f"total {t} frames, {t / FPS:.1f} s")
        return 0
    missing = set(a.clip) - {c["clip"] for c in CLIPS}
    if missing:
        raise SystemExit(f"unknown clip(s): {sorted(missing)}; see --list")
    chosen = [c for c in CLIPS if not a.clip or c["clip"] in a.clip]
    sides = ("ud", "vanilla") if a.side == "both" else (a.side,)
    for c in chosen:
        for side in sides:
            p, n = write_clip(c, a.frames, side)
            print(f"{side:7s} {c['clip']}: {n} frames -> {p}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
