"""Writes the anim feature's tri tests from the generated timing table.

  python gen_ded_anim_tests.py <ded_anim_timing.json> [--out <dir>]

Writes ded_anim_clips.test.sqf / .toml and ded_anim_walk.test.sqf / .toml
beside this script (edit this file, not those). The screenshot moments come
from the timing table, so the tests follow the clips when they are retimed or
rebuilt for another model. triScreenshot needs a literal label, which is why
the bursts are written out line by line.

Both tests are quiet: nobody is armed, nobody fires, --nosound. Run them
through run-anim-tests.ps1 (it uses the pipeline's run-tri.ps1: one game,
hidden desktop) and read them with analyze_anim.py.
"""
import argparse
import json
import os

HERE = os.path.dirname(os.path.abspath(__file__))

# ---------------------------------------------------------------------------
# Sites, Everon airfield. Measured by the ground-height grid the clips test
# logs (DEDG lines).
# ---------------------------------------------------------------------------
CLIP_AT = (4800, 11000)          # where he stands for the clips test, facing south
WALK_MID = (4800, 11200)         # middle of the walk track (he walks east); flat from x 4700 to 4900
WALK_FROM = -34.0                # metres east of the middle: where he starts
WALK_CAM = 300.0                 # the camera stands this far south of the track, still on the flat
WALK_EYE = 4.0                   # camera height over its own ground: clear of the bushes
WALK_FOV = 0.105                 # long lens: about 83 m of track across the frame
BURST_DT = 0.25                  # seconds between burst frames
WALK_SHOTS = 124                 # 31 s: start, the walk at full rate, the stop
STOP_AT = 96                     # burst frame at which he is told to stop (24 s)
TURN_SHOTS = 80                  # 20 s: the turn round on the spot
BACK_SHOTS = 64                  # 16 s: a short walk back west, in the slow band

HEADER = """// ============================================================================
//  %s
//  Written by gen_ded_anim_tests.py (edit that, not this file).
//  Needs @kj_anim (features/anim/build-kj-anim.ps1). Quiet: nobody is armed,
//  nobody fires, --nosound. DEDA / DEDS / DEDW / DEDG / DEDF lines in
//  game_stdout.log are the measurements analyze_anim.py reads.
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
removeAllWeapons player
setDate [1985, 6, 10, 12, 0]
0 setOvercast 0.35
0 setFog 0
setViewDistance 2500
showCinemaBorder false
daC = createCenter civilian
daG = createGroup civilian
ded = objNull
"""

VIEW = """// daView: [bearing from him (0 = north of him), distance, eye height above the
// local ground, aim height above his feet] -> a triSetView array.
daProbe = "HeliHEmpty" createVehicle [%d, %d, 0]
daView = { private ["_b", "_d", "_e", "_a", "_p", "_cx", "_cz", "_g"]; _b = _this select 0; _d = _this select 1; _e = _this select 2; _a = _this select 3; _p = getPosASL ded; _cx = (_p select 0) + _d * sin _b; _cz = (_p select 1) + _d * cos _b; daProbe setPos [_cx, _cz, 0]; _g = (getPosASL daProbe) select 2; [_cx, _g + _e, _cz, (_p select 0) - _cx, ((_p select 2) + _a) - (_g + _e), (_p select 1) - _cz] }
daCam = "camera" camCreate [%d, %d, 30]
daCam cameraEffect ["internal", "back"]
daFov = { daCam camSetTarget [%d, %d, 30]; daCam camSetFov _this; daCam camCommit 0 }
"""

TOML = """# %s (experiment, kept out of tests/integration so a
# sweep never runs it). Needs the @kj_anim mod built beside the data dir
# (experiments/kaiju/features/anim/build-kj-anim.ps1). It is mounted IN PLACE OF
# @uddedushka: both carry the addon uddedushka. Run it through
# features/anim/run-anim-tests.ps1: one game, hidden desktop, --render gl33.
# Written by gen_ded_anim_tests.py. Quiet, no gunfire.
tags = ["kaiju", "experiment", "visual"]
binary = "PoseidonGame"
mission = "experiments/kaiju/features/anim/DedAnim.Eden"
timeout = %d
assert_timeout = 120
extra_args = ["--nosound", "--no-strict", "--width", "%d", "--height", "%d", "--mods-dir", "..", "--mod", "@kj_anim"]
"""


def spawn(at, facing):
    return [
        "daAt = [%d, %d, 0]" % at,
        '"UDDedushka" createUnit [daAt, daG, "ded = this", 0.5, "PRIVATE"]',
        "triSimFrames 5",
        "triAssert [!(isNull ded)]",
        'triAssertEq [typeOf ded, "UDDedushka"]',
        "triAssert [alive ded]",
        "daAnim = 0",
        'ded addEventHandler ["AnimChanged", { daAnim = daAnim + 1; logInfo format ["DEDA anim %1 %2", time, _this select 1] }]',
        # Held still. setUnitPos UP keeps the AI in the upright family, so the move names are the plain ones.
        'daHold = { ded setCaptive true; ded disableAI "MOVE"; ded disableAI "TARGET"; ded disableAI "AUTOTARGET"; daG setBehaviour "CARELESS"; ded setUnitPos "UP"; ded stop true; ded setDir %d; ded setPos daAt }' % facing,
        "call daHold",
        "ded allowDammage false",
    ]


def shot(lines, label, view=None, settle=2):
    if view is not None:
        lines.append("triSetView (%s call daView)" % json.dumps(list(view)))
        lines.append("triSimFrames %d" % settle)
    lines.append('triScreenshot "%s"' % label)
    lines.append('logInfo format ["DEDS %s %%1 %%2 %%3 %%4", time, getMove ded, getPosASL ded, getDir ded]' % label)


def clips_test(timing):
    by = {c["move"]: c for c in timing["clips"]}
    h = timing["height_m"]
    x, z = CLIP_AT
    # Views: he faces south (180), so bearing 180 is dead ahead of him, 270 his right side (west).
    front34 = (150, 2.1 * h, 0.2 * h, 0.47 * h)      # three quarter front, from his left
    right34 = (215, 2.1 * h, 0.2 * h, 0.47 * h)      # three quarter front, from his right
    # Side views are from his LEFT (east): the west side of the site has a tree and a house in the way.
    side_r = (95, 2.2 * h, 0.06 * h, 0.46 * h)
    side_l = side_r
    L = (HEADER % "Dedushka clips: every move played on the creature and photographed at its key moments").splitlines()
    L.append("player setPos [%d, %d, 0]" % (x - 60, z + 260))
    L += spawn(CLIP_AT, 180)
    L += (VIEW % (x, z + 300, x, z - 200, x, z, )).splitlines()
    L.append("0.7 call daFov")
    # Ground heights round the site, for choosing the walk track: one line per row.
    L.append("// ground heights (ASL) on a 25 m grid, 350 m by 500 m")
    L.append("daGz = %d" % (z - 250))
    L.append('while { daGz <= %d } do { daRow = []; daGx = %d; while { daGx <= %d } do { daProbe setPos [daGx, daGz, 0]; daRow = daRow + [(getPosASL daProbe) select 2]; daGx = daGx + 25 }; logInfo format ["DEDG %%1 %%2 %%3", daGz, %d, daRow]; daGz = daGz + 25 }'
             % (z + 250, x - 175, x + 175, x - 175))
    L.append("triSimFrames 200")
    L.append('logInfo format ["DEDA settled %1 %2 %3 %4 alive %5", time, getMove ded, getPosASL ded, getDir ded, alive ded]')
    L.append('triAssertEq [getMove ded, "dedidle"]')
    L.append("triSetView (%s call daView)" % json.dumps(list(front34)))
    L.append("triSimFrames 6")
    L.append('triScreenshot "00_warmup_discard"')

    def play(move, shots, view, back_to="dedidle", extra_wait=1.8, reset=True, holds=None):
        c = by[move]
        tag = move[3:].lower()
        L.append("// ---- %s (%.1f s%s)" % (move, c["secs"], ", loop" if c["looped"] else ""))
        if reset:
            L.append("call daHold")
            L.append("triSimFrames 40")
        L.append('if (true) then { daT0 = time; daP0 = getPosASL ded; ded playMove "%s" }' % move)
        L.append('logInfo format ["DEDA play %s %%1 from %%2"' % move + ", time, getMove ded]")
        L.append("triSimUntil { time > daT0 + 0.5 }")
        L.append('triAssertEq [getMove ded, "%s"]' % move.lower())
        for i, (t, label, v) in enumerate(shots):
            L.append("triSimUntil { time > daT0 + %.2f }" % t)
            shot(L, "%s_%d_%s" % (tag, i + 1, label), v if v is not None else view)
        return c

    # Idle: two moments of the standing loop (he is already in it).
    L.append("// ---- DedIdle (he spawns into it)")
    L.append("if (true) then { daT0 = time }")
    shot(L, "idle_1_front", front34)
    L.append("triSimUntil { time > daT0 + 3.0 }")
    shot(L, "idle_2_side", side_r)
    L.append("triSimUntil { time > daT0 + 6.0 }")
    shot(L, "idle_3_front", front34)

    # Walk, played once (a playMove runs a loop at its slowest rate for one cycle).
    w = by["DedWalk"]
    slow = w["secs"] / w["slowest"]
    ev = {e["name"]: e["t"] for e in w["events"]}
    c = play("DedWalk", [(ev["foot_r_down"] / w["slowest"] - 0.7, "right_foot_coming_down", None),
                         (ev["foot_r_down"] / w["slowest"] + 0.3, "right_foot_down", None),
                         (0.55 * slow, "mid_stride", None),
                         (ev["foot_l_down"] / w["slowest"] + 0.3, "left_foot_down", None)], side_r)
    L.append("triSimUntil { time > daT0 + %.2f }" % (slow + 2.2))
    L.append('logInfo format ["DEDA walkcycle moved %1 m in one played cycle, step ' + "%.3f" % w["step_m"] + ', move %2", (getPosASL ded) distance daP0, getMove ded]')
    L.append('triAssertEq [getMove ded, "dedidle"]')
    L.append("triAssert [abs (((getPosASL ded) distance daP0) - %.3f) < 2.5]" % w["step_m"])

    for move, view in (("DedTurnL", front34), ("DedTurnR", right34)):
        t = by[move]
        ev = sorted(e["t"] for e in t["events"])
        play(move, [(ev[0] - 0.6, "foot_up", None), (ev[0] + 0.3, "foot_down", None), (ev[1] + 0.3, "other_foot_down", None)], view)
        L.append("triSimUntil { time > daT0 + %.2f }" % (t["secs"] + 2.0))
        L.append('triAssertEq [getMove ded, "dedidle"]')

    def act(move, labels, view, alt=None):
        c = by[move]
        ev = {e["name"]: e["t"] for e in c["events"]}
        shots = []
        for name, label, off in labels:
            t = ev[name] + off if name in ev else float(name)
            shots.append((t, label, alt if label.endswith("_side") else None))
        play(move, shots, view)
        L.append("triSimUntil { time > daT0 + %.2f }" % (c["secs"] + 1.8))
        L.append('logInfo format ["DEDA after %s %%1 %%2"' % move + ", time, getMove ded]")
        L.append('triAssertEq [getMove ded, "dedidle"]')

    act("DedRoar", [("head_down", "head_down", 0.0), ("roar_start", "thrown_up", 0.3), ("roar_full", "full_cry", 0.5),
                    ("roar_full", "full_cry_side", 1.4), ("roar_end", "spent", 0.9)], front34, side_r)
    # The beam: rise, hold (he must stay in it on his own), lower.
    r, hld, low = by["DedBeamRise"], by["DedBeamHold"], by["DedBeamLower"]
    play("DedBeamRise", [(0.3, "start_side", None), (0.45 * r["secs"], "rising_side", None), (r["secs"] - 0.2, "tall_side", None)], side_r)
    L.append("triSimUntil { time > daT0 + %.2f }" % (r["secs"] + 1.5))
    shot(L, "beamhold_1_front", front34)
    L.append('triAssertEq [getMove ded, "dedbeamhold"]')
    L.append("triSimUntil { time > daT0 + %.2f }" % (r["secs"] + 1.5 + 3.2 * hld["secs"]))
    shot(L, "beamhold_2_side", side_r)
    L.append('logInfo format ["DEDA hold kept %1 %2", time, getMove ded]')
    L.append('triAssertEq [getMove ded, "dedbeamhold"]')
    ev = {e["name"]: e["t"] for e in low["events"]}
    play("DedBeamLower", [(ev["ribs_closed"], "sagging_side", None), (ev["lowest"], "lowest_side", None)], side_r, reset=False)
    L.append("triSimUntil { time > daT0 + %.2f }" % (low["secs"] + 1.8))
    L.append('triAssertEq [getMove ded, "dedidle"]')

    act("DedStomp", [("foot_top", "foot_up", -0.3), ("impact", "falling", -0.3), ("impact", "impact", 0.12),
                     ("impact", "after_side", 0.6)], right34, side_r)
    act("DedSwat", [("arm_raised", "arm_raised", 0.0), ("sweep_03", "sweep_early", 0.0), ("sweep_05", "sweep_mid", 0.0),
                    ("sweep_07", "sweep_late", 0.0), ("sweep_05", "x", 99.0)][:4], right34)
    act("DedFlinch", [("deepest", "deepest", 0.0), ("deepest", "ringing", 0.5)], front34)
    # Dormant: he must stay in it on his own, and come out when DedIdle is played.
    d = by["DedDormant"]
    play("DedDormant", [(5.0, "sunk_front", None), (8.0, "sunk_side", side_r)], front34)
    L.append("triSimUntil { time > daT0 + %.2f }" % (d["secs"] + 4.0))
    L.append('logInfo format ["DEDA dormant kept %1 %2", time, getMove ded]')
    L.append('triAssertEq [getMove ded, "deddormant"]')
    L.append('if (true) then { daT0 = time; ded playMove "DedIdle" }')
    L.append("triSimUntil { time > daT0 + 5.0 }")
    shot(L, "dormant_3_woken", front34)
    L.append('triAssertEq [getMove ded, "dedidle"]')
    # Death: he falls on his face and is dead.
    L.append("// ---- death")
    L.append("triAssert [alive ded]")
    L.append("if (true) then { daT0 = time; ded allowDammage true; ded setDammage 1 }")
    L.append("triSimUntil { time > daT0 + %.2f }" % (0.55 * by["DedDying"]["secs"]))
    shot(L, "die_1_falling", side_r)
    L.append("triSimUntil { time > daT0 + %.2f }" % (by["DedDying"]["secs"] + 2.0))
    shot(L, "die_2_down", side_r)
    L.append("triAssert [!(alive ded)]")
    L.append('logInfo format ["DEDA done %1 animChanges %2", time, daAnim]')
    L.append("triEndTest")
    return L


def walk_test(timing):
    by = {c["move"]: c for c in timing["clips"]}
    w = by["DedWalk"]
    mx, mz = WALK_MID
    L = (HEADER % "Dedushka walk: speed, foot slide, footfalls and the turn on the spot, from a fixed side camera").splitlines()
    L.append("player setPos [%d, %d, 0]" % (mx - 120, mz + 200))
    L += spawn((mx + int(WALK_FROM), mz), 90)
    L += (VIEW % (mx, mz + 300, mx, mz - 350, mx, mz)).splitlines()
    L.append("%s call daFov" % WALK_FOV)
    L.append('call preprocessFile "kj\\anim\\ded_anim_timing.sqf"')
    L.append("triAssert [KJ_animLoaded]")
    L.append("// ground heights along the track and under the camera")
    L.append('daRow = []; daGx = %d; while { daGx <= %d } do { daProbe setPos [daGx, %d, 0]; daRow = daRow + [(getPosASL daProbe) select 2]; daGx = daGx + 5 }; logInfo format ["DEDG track %%1 %%2 %%3", %d, %d, daRow]'
             % (mx - 60, mx + 60, mz, mz, mx - 60))
    L.append("triSimFrames 200")
    L.append('triAssertEq [getMove ded, "dedidle"]')
    # The fixed camera: south of the track middle, a few metres up, looking north.
    L.append("daProbe setPos [%d, %d, 0]" % (mx, mz - int(WALK_CAM)))
    L.append("daCamY = ((getPosASL daProbe) select 2) + %.1f" % WALK_EYE)
    L.append("daFix = [%d, daCamY, %d, 0, ((getPosASL ded) select 2) + 14 - daCamY, %d]" % (mx, mz - int(WALK_CAM), int(WALK_CAM)))
    L.append('logInfo format ["DEDC cam %%1 fov %s dist %d mid [%d,%d] ded asl %%2", daFix, getPosASL ded]' % (WALK_FOV, int(WALK_CAM), mx, mz))
    L.append("triSetView daFix")
    L.append("triSimFrames 6")
    L.append('triScreenshot "00_warmup_discard"')
    # Background: him out of the frame.
    L.append("ded setPos [%d, %d, 0]" % (mx + 400, mz + 300))
    L.append("triSimFrames 8")
    shot(L, "bg")
    # Calibration: standing 40 m apart on the track.
    for tag, off in (("cal_a", -20), ("cal_b", 20)):
        L.append("if (true) then { ded setPos [%d, %d, 0]; ded setDir 90 }" % (mx + off, mz))
        L.append("triSimFrames 30")
        shot(L, tag)
    L.append("call daHold")
    L.append("triSimFrames 60")
    L.append('[ded] exec "kj\\anim\\footfalls.sqs"')
    L.append("DEDW_ON = true")
    L.append("DEDW_QUIT = false")
    L.append('[ded, 0.05] exec "ded_watch.sqs"')
    L.append("daFalls = 0")
    L.append("triSimFrames 20")
    L.append("// ---- the walk: the AI takes him east at his full rate (the goal is far), then he is told to stop")
    L.append('if (true) then { daT0 = time; daP0 = getPosASL ded; ded stop false; ded enableAI "MOVE"; ded doMove [%d, %d, 0] }' % (mx + 260, mz))
    L.append('logInfo format ["DEDA walk start %1 %2", time, getPosASL ded]')
    foot_log = 'if (KJ_footfallCount > daFalls) then { daFalls = KJ_footfallCount; logInfo format ["DEDF %1", KJ_footfall] }'
    for i in range(WALK_SHOTS):
        L.append("triSimUntil { time >= daT0 + %.2f }" % (i * BURST_DT))
        if i == STOP_AT:
            L.append("if (true) then { doStop ded; ded doMove (getPos ded) }")
            L.append('logInfo format ["DEDA walk stop %1 %2 moved %3", time, getPosASL ded, (getPosASL ded) distance daP0]')
        shot(L, "w_%03d" % i)
        L.append(foot_log)
    L.append('logInfo format ["DEDA walk end %1 %2 moved %3 move %4", time, getPosASL ded, (getPosASL ded) distance daP0, getMove ded]')
    L.append("triAssert [((getPosASL ded) distance daP0) > %.1f]" % (0.6 * w["speed_m_s"] * STOP_AT * BURST_DT))
    L.append('triAssertEq [getMove ded, "dedidle"]')
    L.append("// ---- the turn: told to watch a point behind him, he must come round on the spot")
    L.append('if (true) then { daT0 = time; daD0 = getDir ded; daTurnBy = "doWatch"; ded doWatch [%d, %d, 0] }' % (mx - 400, mz))
    L.append('logInfo format ["DEDA turn start %1 %2 dir %3", time, getPosASL ded, getDir ded]')
    for i in range(TURN_SHOTS):
        L.append("triSimUntil { time >= daT0 + %.2f }" % (i * BURST_DT))
        if i == 12:
            # No turn after 3 s: doWatch does not move him, so order him to a point behind instead.
            L.append('if ((cos ((getDir ded) - daD0)) > 0.97) then { daTurnBy = "doMove"; ded doMove [%d, %d, 0] }' % (mx - 300, mz))
            L.append('logInfo format ["DEDA turn by %1 %2 dir %3", daTurnBy, time, getDir ded]')
        shot(L, "t_%03d" % i)
        L.append(foot_log)
    L.append('logInfo format ["DEDA turn end %1 %2 dir %3 move %4 footfalls %5", time, getPosASL ded, getDir ded, getMove ded, KJ_footfallCount]')
    L.append("triAssert [(cos ((getDir ded) - daD0)) < -0.9]")
    L.append("// ---- a short walk back west: the goal is near, so the AI holds him in the slow band")
    L.append('if (true) then { daT0 = time; daP0 = getPosASL ded; ded doWatch objNull; ded doMove [((getPosASL ded) select 0) - 45, %d, 0] }' % mz)
    L.append('logInfo format ["DEDA back start %1 %2", time, getPosASL ded]')
    for i in range(BACK_SHOTS):
        L.append("triSimUntil { time >= daT0 + %.2f }" % (i * BURST_DT))
        shot(L, "b_%03d" % i)
        L.append(foot_log)
    L.append('logInfo format ["DEDA back end %1 %2 moved %3 move %4 footfalls %5", time, getPosASL ded, (getPosASL ded) distance daP0, getMove ded, KJ_footfallCount]')
    L.append("if (true) then { DEDW_QUIT = true }")
    L.append("triAssert [alive ded]")
    L.append("triAssert [((getPosASL ded) distance daP0) > 8]")
    L.append("triAssert [KJ_footfallCount >= 8]")
    L.append("triEndTest")
    return L


def families_test(timing):
    """The same clips under the stances the AI asks an unarmed man for (under fire, in COMBAT), without any fire."""
    h = timing["height_m"]
    x, z = CLIP_AT
    front34 = (150, 2.1 * h, 0.2 * h, 0.47 * h)
    L = (HEADER % "Dedushka stance families: the low and crouch copies of the loops, entered without any gunfire").splitlines()
    L.append("player setPos [%d, %d, 0]" % (x - 60, z + 260))
    L += spawn(CLIP_AT, 180)
    L += (VIEW % (x, z + 300, x, z - 200, x, z)).splitlines()
    L.append("0.7 call daFov")
    L.append("triSimFrames 200")
    L.append('triAssertEq [getMove ded, "dedidle"]')
    L.append("triSetView (%s call daView)" % json.dumps(list(front34)))
    L.append("triSimFrames 6")
    L.append('triScreenshot "00_warmup_discard"')
    L.append('// ---- "down": the stance the AI wants under fire. He must stay upright, in the low copy of the idle.')
    L.append('if (true) then { daT0 = time; daN0 = daAnim; ded setUnitPos "DOWN" }')
    L.append("triSimUntil { time > daT0 + 3.0 }")
    L.append('logInfo format ["DEDA low idle %1 %2", time, getMove ded]')
    L.append('triAssertEq [getMove ded, "dedidlelow"]')
    shot(L, "low_1_idle", front34)
    L.append("// ---- walking in the low family")
    L.append('if (true) then { daT0 = time; daP0 = getPosASL ded; ded stop false; ded enableAI "MOVE"; ded doMove [%d, %d, 0] }' % (x, z - 300))
    L.append("triSimUntil { time > daT0 + 5.0 }")
    L.append('logInfo format ["DEDA low walk %1 %2 moved %3", time, getMove ded, (getPosASL ded) distance daP0]')
    L.append('triAssertEq [getMove ded, "dedwalklow"]')
    shot(L, "low_2_walk", front34)
    L.append("// ---- stance change in mid stride: back to the upright family")
    L.append('if (true) then { daT0 = time; ded setUnitPos "UP" }')
    L.append("triSimUntil { time > daT0 + 3.0 }")
    L.append('logInfo format ["DEDA up walk %1 %2 moved %3", time, getMove ded, (getPosASL ded) distance daP0]')
    L.append('triAssertEq [getMove ded, "dedwalk"]')
    L.append("triAssert [((getPosASL ded) distance daP0) > 8]")
    L.append("// ---- stop, then COMBAT with free stance: the AI asks for crouch or low; he must settle in one of the idle copies")
    L.append("if (true) then { daT0 = time; doStop ded; ded doMove (getPos ded) }")
    L.append("triSimUntil { time > daT0 + 4.0 }")
    L.append('triAssertEq [getMove ded, "dedidle"]')
    L.append('if (true) then { daT0 = time; ded setUnitPos "AUTO"; daG setBehaviour "COMBAT"; daG setCombatMode "GREEN" }')
    L.append("triSimUntil { time > daT0 + 8.0 }")
    L.append('logInfo format ["DEDA combat auto %1 %2", time, getMove ded]')
    L.append('triAssert [(getMove ded) in ["dedidle", "dedidlelow", "dedidlecrouch"]]')
    shot(L, "combat_1_idle", front34)
    L.append("// ---- an act from whichever family he is in, and back")
    L.append('if (true) then { daT0 = time; ded playMove "DedFlinch" }')
    L.append("triSimUntil { time > daT0 + 0.5 }")
    L.append('triAssertEq [getMove ded, "dedflinch"]')
    L.append("triSimUntil { time > daT0 + %.2f }" % (next(c for c in timing["clips"] if c["move"] == "DedFlinch")["secs"] + 4.0))
    L.append('logInfo format ["DEDA after flinch %1 %2", time, getMove ded]')
    L.append('triAssert [(getMove ded) in ["dedidle", "dedidlelow", "dedidlecrouch"]]')
    L.append("// ---- a turn in the low family")
    L.append('if (true) then { daT0 = time; daD0 = getDir ded; daG setBehaviour "CARELESS"; ded setUnitPos "DOWN"; ded doWatch [%d, %d, 0] }' % (x, z + 400))
    L.append("triSimUntil { time > daT0 + 6.0 }")
    L.append('logInfo format ["DEDA low turn %1 %2 dir %3 from %4", time, getMove ded, getDir ded, daD0]')
    L.append('triAssert [(getMove ded) in ["dedturnllow", "dedturnrlow"]]')
    shot(L, "low_3_turn", front34)
    L.append("triSimUntil { time > daT0 + 20.0 }")
    L.append('logInfo format ["DEDA low turn done %1 %2 dir %3 animChanges %4", time, getMove ded, getDir ded, daAnim - daN0]')
    L.append("triAssert [(cos ((getDir ded) - daD0)) < -0.9]")
    L.append("// A stance loop shows as hundreds of move changes; the whole test needs about twenty.")
    L.append("triAssert [(daAnim - daN0) < 60]")
    L.append("triAssert [alive ded]")
    L.append("triEndTest")
    return L


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("timing")
    ap.add_argument("--out", default=HERE)
    args = ap.parse_args()
    timing = json.load(open(args.timing, encoding="utf-8"))
    for name, lines, title, timeout, size in (
            ("ded_anim_clips", clips_test(timing), "Dedushka clips test", 600, (1280, 960)),
            ("ded_anim_walk", walk_test(timing), "Dedushka walk test", 900, (1280, 720)),
            ("ded_anim_families", families_test(timing), "Dedushka stance families test", 400, (1280, 960))):
        with open(os.path.join(args.out, name + ".test.sqf"), "w", encoding="ascii", newline="\n") as fh:
            fh.write("\n".join(lines) + "\n")
        with open(os.path.join(args.out, name + ".test.toml"), "w", encoding="ascii", newline="\n") as fh:
            fh.write(TOML % (title, timeout, size[0], size[1]))
        print("%s: %d lines, %d screenshots" % (name, len(lines), sum(1 for l in lines if l.startswith("triScreenshot"))))


if __name__ == "__main__":
    main()
