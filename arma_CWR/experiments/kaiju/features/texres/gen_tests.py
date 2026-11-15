r"""Writes the two tri tests of the texture-resolution study from their shot tables.

  python gen_tests.py

  texres_charts.test.sqf   the chart creature (density patches and the mip tell) photographed
                           from a man's eye height at its feet, from 60 m and from 300 m; then
                           the BEFORE and AFTER bodies from the same cameras; then draw calls
                           and frame time with 5 and with 101 sections.
  texres_limits.test.sqf   one 8 m panel per texture format and size, an overview and two
                           close-ups each, with the frame time of the first look (the load).

Positions are given in the creature's own frame (x = metres to its left, front = metres in
front of it, height above its feet) and turned into world numbers here, so the SQF holds
only literals. The creature stands at [5055, 3870] on the Morton shelf facing south, as in
the pipeline's photo test: its left is east, its front is south.

Statements stay at the top level, one per line: tri sends a test line by line, and a
triSimFrames inside a loop or a called block is slow (pipeline NOTES.md, surprise 9).
"""
import os

HERE = os.path.dirname(os.path.abspath(__file__))
AT = (5055.0, 3870.0)


def world(xm, front):
    return AT[0] + xm, AT[1] - front


def view(cam, target):
    (cx, cf, eye), (tx, tf, th) = cam, target
    wx, wz = world(cx, cf)
    twx, twz = world(tx, tf)
    return "[%.2f, %.2f, %.2f, %.2f, %.2f, %.2f] call tvView" % (wx, wz, eye, twx, twz, th)


HEADER = r'''// ============================================================================
//  %(title)s
//  Written by gen_tests.py: edit the shot tables there, not this file.
//
//  Needs @kj_texres (features/texres/build_texres.py), mounted in place of the
//  shared @uddedushka. Run it through run-texres.ps1 (the pipeline's run-tri.ps1
//  with this feature's folders): one game, hidden desktop, --render gl33, quiet.
//  Nobody is armed against anybody and nobody fires.
//
//  Camera rules from tools/screenshots/README.md and the pipeline's photo test:
//  a cutscene camera only to hide the HUD and set the lens, triSetView for
//  framing, a throwaway first frame, a triSimFrames between change and shot.
//  Every TEXSHOT / TEXRES line in game_stdout.log is a measurement.
// ============================================================================
triSimUntil { alive player }
player allowDammage false
player setCaptive true
removeAllWeapons player
player setPos [5060, 4120, 0]
setDate [1985, 6, 10, 12, 0]
0 setOvercast 0.2
0 setFog 0
setViewDistance 5000
showCinemaBorder false
// A tri run starts with object shadows off; the props and the town should cast theirs.
// (The test bodies have one visual LOD marked lodnoshadow, so they cast none.)
triEnableShadows
tvC = createCenter civilian
tvG = createGroup civilian
tvE = createCenter east
tvEG = createGroup east
tvAt = [5055, 3870, 0]
tvProbe = "HeliHEmpty" createVehicle [5055, 4200, 0]
tvCam = "camera" camCreate [5055, 3700, 30]
tvCam cameraEffect ["internal", "back"]
tvFov = { tvCam camSetTarget [5055, 3900, 30]; tvCam camSetFov _this; tvCam camCommit 0; tvLens = _this }
0.7 call tvFov
// tvView: [camera x, camera z, eye height above the ground or sea there, target x, target z,
// target height above the creature's feet]. The feet are at tvFeet metres above sea level.
tvFeet = 16.335
tvView = { private ["_cx", "_cz", "_g", "_e"]; _cx = _this select 0; _cz = _this select 1; tvProbe setPos [_cx, _cz, 0]; _g = (getPosASL tvProbe) select 2; _e = _g + (_this select 2); tvLast = [_cx, _e, _cz, _this select 3, tvFeet + (_this select 5), _this select 4]; [_cx, _e, _cz, (_this select 3) - _cx, tvFeet + (_this select 5) - _e, (_this select 4) - _cz] }
tvNote = { logInfo format ["TEXSHOT %%1 cam [x,asl,z] to [x,asl,z] %%2 lens %%3 window %%4", _this, tvLast, tvLens, triGetWindowSize] }
ded = objNull
tvHold = { ded setCaptive true; ded disableAI "MOVE"; ded disableAI "TARGET"; ded disableAI "AUTOTARGET"; ded setBehaviour "CARELESS"; ded stop true; ded setDir 180; ded setPos tvAt }
'''

SHOT = '''triSetView (%(view)s)
triSimFrames %(frames)d
triScreenshot "%(name)s"
"%(name)s" call tvNote
'''


def shot(name, cam, target, frames=8):
    return SHOT % {"view": view(cam, target), "frames": frames, "name": name}


def lens(f):
    return "%s call tvFov\n" % f


def spawn(cls):
    return ('"%s" createUnit [tvAt, tvG, "ded = this", 0.5, "PRIVATE"]\n'
            'triSimFrames 5\n'
            'triAssert [!(isNull ded)]\n'
            'triAssertEq [typeOf ded, "%s"]\n'
            'call tvHold\n' % (cls, cls))


def charts():
    out = [HEADER % {"title": "Texture resolution charts on the Dedushka proxy (experiment, not a regression test)."}]
    out.append("// Scale props beside the left foot: an empty T-80 and an unarmed soldier.\n")
    tx, tz = world(12.5, 2.0)
    sx, sz = world(8.8, 7.5)
    out.append('tvTank = "T80" createVehicle [%.1f, %.1f, 0]\n' % (tx, tz))
    out.append("tvTank setDir 180\n")
    out.append("tvMan = objNull\n")
    out.append('"SoldierEB" createUnit [[%.1f, %.1f, 0], tvEG, "tvMan = this", 0.5, "PRIVATE"]\n' % (sx, sz))
    out.append("triSimFrames 5\n")
    out.append('if (true) then { removeAllWeapons tvMan; tvMan setCaptive true; tvMan disableAI "MOVE"; tvMan setCombatMode "BLUE"; tvMan setBehaviour "CARELESS"; tvMan stop true; tvMan setDir 180 }\n')
    out.append("// ---- the chart creature ---------------------------------------------------\n")
    out.append(spawn("UDDedushka"))
    out.append("triSimFrames 120\n")
    out.append('logInfo format ["TEXRES placed %1 asl %2 dir %3 move %4", typeOf ded, getPosASL ded, getDir ded, getMove ded]\n')
    # The helicopter view comes first, before any close view has loaded the sharp levels:
    # the engine picks the levels to load from the distance to the model ORIGIN (the feet).
    # Run charts1 had no mip anchors in the chart model and showed 16 texels per metre here;
    # from run charts2 on the chart model carries them (gen_texres.py, add_mip_anchors).
    out.append("// First look at all: level with the chest from 10 m, 35 m from the origin at the feet.\n")
    out.append(shot("c00_first_look_chest_10m", (0, 15, 32.0), (0, 5, 32.0), frames=20))
    out.append(shot("00_warmup_discard", (0, 60, 1.7), (0, 0, 20)))
    out.append("// Walking up to it: 60 m, then 26 m.\n")
    out.append(shot("c12_60m", (0, 60, 1.7), (0, 0, 20)))
    out.append(shot("c11_20m", (0, 26, 1.7), (0, 0, 14)))
    out.append("// From a man's eye height at its feet: 4 m in front of the toes.\n")
    out.append(shot("c01a_feet_right_toes", (-5, 10, 1.7), (-5, 6, 1.0), frames=20))
    out.append(shot("c01_feet_right_shin", (-5, 10, 1.7), (-5, 2, 6.5)))
    out.append(shot("c02a_feet_left_toes", (5, 10, 1.7), (5, 6, 1.0)))
    out.append(shot("c02_feet_left_shin", (5, 10, 1.7), (5, 2, 6.5)))
    out.append(shot("c03_feet_right_thigh", (-5, 10, 1.7), (-7, 2, 14)))
    out.append(shot("c04_feet_left_thigh", (5, 10, 1.7), (7, 2, 14)))
    out.append(shot("c05_feet_up_belly", (0, 10, 1.7), (0, 4.2, 24)))
    out.append(shot("c06_feet_up_chest", (0, 10, 1.7), (0, 4.8, 31)))
    out.append(shot("c07_feet_up_head", (0, 10, 1.7), (0, 4.2, 38)))
    out.append("// Close enough to touch: 1.5 m from the toes. Then straight up from between the feet.\n")
    out.append(shot("c09_touch_right_toes", (-5, 7.5, 1.7), (-5, 6, 1.2)))
    out.append(shot("c10_touch_left_toes", (5, 7.5, 1.7), (5, 6, 1.2)))
    out.append(shot("c08_under_pelvis", (0, 1, 1.7), (0, 0, 18.5)))
    out.append("// Backing away: 150 m, 300 m, 600 m and 1 km.\n")
    out.append(shot("c13_150m", (0, 150, 1.7), (0, 0, 20)))
    out.append(shot("c14_300m", (0, 300, 1.7), (0, 0, 20)))
    out.append(shot("c15_600m", (0, 600, 1.7), (0, 0, 20)))
    out.append(shot("c16_1km", (0, 1000, 1.7), (0, 0, 20)))
    out.append("// The same far cameras through a 4 times lens (the hold-zoom, binoculars or a scope).\n")
    out.append(lens("0.175"))
    out.append(shot("z12_60m_lens4", (0, 60, 1.7), (0, 0, 12)))
    out.append(shot("z12b_60m_lens4_up", (0, 60, 1.7), (0, 0, 30)))
    out.append(shot("z14_300m_lens4", (0, 300, 1.7), (0, 0, 20)))
    out.append(shot("z16_1km_lens4", (0, 1000, 1.7), (0, 0, 20)))
    out.append(lens("0.7"))
    # B, A and M use different textures except that A and M share theirs: A must come before M,
    # because a level, once loaded, stays loaded. Inside each body the cameras go from far to
    # near, the way a player first walks up, and the view from between the feet (1 m from the
    # origin, which loads everything) comes last of the near ones.
    for tag, cls, title in (("b", "UDDedushkaB", "BEFORE: one 1024 sheet"),
                            ("a", "UDDedushkaA", "AFTER: 4096 sheets by height band"),
                            ("m", "UDDedushkaM", "AFTER with a mip anchor face per sheet")):
        out.append("// ---- %s ----------------------------------------------------\n" % title)
        out.append("deleteVehicle ded\n")
        out.append("triSimFrames 5\n")
        out.append(shot("p%s_00_empty_60m" % tag, (0, 60, 1.7), (0, 0, 20)))
        out.append("triPerfReset 0\n")
        out.append(spawn(cls))
        out.append("triSimFrames 40\n")
        out.append('logInfo format ["TEXRES first 45 frames with %s in view at 60 m: %%1", triPerfStats 0]\n' % cls)
        out.append(shot("p%s_01_60m" % tag, (0, 60, 1.7), (0, 0, 20), frames=20))
        out.append(shot("p%s_02_scale_props" % tag, (22.5, 39, 2.5), (0, 0, 6), frames=12))
        out.append(shot("p%s_03_foot_and_tank" % tag, (15, 19, 1.7), (7, 2, 4), frames=12))
        out.append(shot("p%s_04_toes_and_soldier" % tag, (10.5, 11.5, 1.7), (7, 5.5, 1.4), frames=12))
        out.append(shot("p%s_04b_touch_toes" % tag, (5, 7.5, 1.7), (5, 6, 1.2), frames=12))
        out.append(shot("p%s_05_feet_up" % tag, (0, 14, 1.7), (0, 0, 34), frames=12))
        out.append(shot("p%s_06_shin_from_feet" % tag, (5, 10, 1.7), (5, 2, 5), frames=12))
        out.append(shot("p%s_07_under_pelvis" % tag, (0, 1, 1.7), (0, 0, 18.5), frames=12))
        out.append(shot("p%s_08_150m" % tag, (0, 150, 1.7), (0, 0, 20)))
        out.append(shot("p%s_09_300m" % tag, (0, 300, 1.7), (0, 0, 20)))
        out.append(shot("p%s_10_600m" % tag, (0, 600, 1.7), (0, 0, 20)))
        out.append(shot("p%s_10s_600m_shifted" % tag, (0.4, 600, 1.7), (0.4, 0, 20)))
        out.append(shot("p%s_11_1km" % tag, (0, 1000, 1.7), (0, 0, 20)))
        out.append(shot("p%s_11s_1km_shifted" % tag, (0.6, 1000, 1.7), (0.6, 0, 20)))
        out.append(lens("0.175"))
        out.append(shot("p%s_12_300m_lens4" % tag, (0, 300, 1.7), (0, 0, 20)))
        out.append(shot("p%s_12s_300m_lens4_shifted" % tag, (0.05, 300, 1.7), (0.05, 0, 20)))
        out.append(lens("0.7"))
    out.append("// ---- sections: the AFTER body in 5 runs of faces, then in 101 ------------------\n")
    out.append("// Same sheets, same faces, same camera (60 m). Two rounds, to see the noise.\n")
    for rnd in (1, 2):
        for cls in ("UDDedushkaA", "UDDedushkaS"):
            out.append("deleteVehicle ded\n")
            out.append("triSimFrames 5\n")
            out.append(spawn(cls))
            out.append("triSetView (%s)\n" % view((0, 60, 1.7), (0, 0, 20)))
            out.append("triSimFrames 60\n")
            out.append("triPerfReset 0\n")
            out.append("triSimFrames 200\n")
            out.append('logInfo format ["TEXRES sections round %d %s: %%1", triPerfStats 0]\n' % (rnd, cls))
    out.append("deleteVehicle ded\n")
    out.append("triSimFrames 5\n")
    out.append("triSetView (%s)\n" % view((0, 60, 1.7), (0, 0, 20)))
    out.append("triSimFrames 60\n")
    out.append("triPerfReset 0\n")
    out.append("triSimFrames 200\n")
    out.append('logInfo format ["TEXRES sections no creature: %1", triPerfStats 0]\n')
    out.append('logInfo format ["TEXRES end: time %1 gl errors %2", time, triGetGLErrorCount]\n')
    out.append("triEndTest\n")
    return "".join(out)


PANELS = ["d1_1024", "d1_2048", "d1_4096", "d5_2048", "a8_1024", "png_2048", "png_4096", "tga_1024", "jpg_2048",
          "d1m_4096", "a4_1024", "d1_8192w"]


def limits():
    out = [HEADER % {"title": "Texture limits: one 8 m panel per format and size (experiment, not a regression test)."}]
    out.append("pan = objNull\n")
    out.append('logInfo format ["TEXRES aspect [leftFOV,topFOV,ui...] %1 window %2 cutscene lens %3", triGetAspectSettings, triGetWindowSize, triCamFov]\n')
    out.append("triSimFrames 120\n")
    out.append(shot("00_warmup_discard", (0, 60, 1.7), (0, 0, 20)))
    out.append("triSetView (%s)\n" % view((0, 14, 4.5), (0, 0, 4.5)))
    out.append("triSimFrames 60\n")
    out.append("triPerfReset 0\n")
    out.append("triSimFrames 60\n")
    out.append('logInfo format ["TEXRES baseline, no panel: %1 memory %2 MB", triPerfStats 0, triMemoryMB]\n')
    for pid in PANELS:
        out.append("// ---- %s\n" % pid)
        out.append('if (true) then { pan = "KJTxp_%s" createVehicle tvAt; pan setPos tvAt; pan setDir 0 }\n' % pid)
        out.append("triPerfReset 0\n")
        out.append(shot("l_%s_a_overview" % pid, (0, 14, 4.5), (0, 0, 4.5), frames=20))
        out.append('logInfo format ["TEXRES %s overview, first 20 frames: %%1 memory %%2 MB errors %%3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]\n' % pid)
        out.append("triPerfReset 0\n")
        out.append(shot("l_%s_b_close_left" % pid, (-2, 0.45, 1.5), (-2, 0, 1.5), frames=20))
        out.append('logInfo format ["TEXRES %s close-up (full size level wanted), first 20 frames: %%1 memory %%2 MB errors %%3", triPerfStats 0, triMemoryMB, triGetGLErrorCount]\n' % pid)
        out.append(shot("l_%s_c_close_right" % pid, (2, 0.45, 1.5), (2, 0, 1.5)))
        out.append(shot("l_%s_d_ramps" % pid, (0, 3.0, 2.8), (0, 0, 2.8)))
        if pid == "d1m_4096":
            for d in (50, 150, 400, 800):
                out.append(shot("l_%s_far_%dm" % (pid, d), (0, d, 1.7), (0, 0, 4.5)))
            out.append(lens("0.175"))
            for d in (150, 400, 800):
                out.append(shot("l_%s_far_%dm_lens4" % (pid, d), (0, d, 1.7), (0, 0, 4.5)))
            out.append(lens("0.7"))
        out.append("deleteVehicle pan\n")
        out.append("triSimFrames 5\n")
    out.append("// The player's own lens, for the pixels per metre a walking player gets.\n")
    out.append('tvCam cameraEffect ["terminate", "back"]\n')
    out.append("triClearView\n")
    out.append("triSimFrames 30\n")
    out.append('logInfo format ["TEXRES player view: lens %1 camera %2", triCamFov, triCamView]\n')
    out.append('logInfo format ["TEXRES end: time %1 gl errors %2", time, triGetGLErrorCount]\n')
    out.append("triEndTest\n")
    return "".join(out)


def detail():
    """The AFTER body (with mip anchors) up close, for the detail layer demo: run once plain and once with
    run-texres.ps1 -ShaderOverrideDir detail_demo and compare frame by frame."""
    out = [HEADER % {"title": "Detail layer demo frames: the AFTER body up close (experiment, not a regression test)."}]
    tx, tz = world(12.5, 2.0)
    sx, sz = world(8.8, 7.5)
    out.append('tvTank = "T80" createVehicle [%.1f, %.1f, 0]\n' % (tx, tz))
    out.append("tvTank setDir 180\n")
    out.append("tvMan = objNull\n")
    out.append('"SoldierEB" createUnit [[%.1f, %.1f, 0], tvEG, "tvMan = this", 0.5, "PRIVATE"]\n' % (sx, sz))
    out.append("triSimFrames 5\n")
    out.append('if (true) then { removeAllWeapons tvMan; tvMan setCaptive true; tvMan disableAI "MOVE"; tvMan setCombatMode "BLUE"; tvMan setBehaviour "CARELESS"; tvMan stop true; tvMan setDir 180 }\n')
    out.append(spawn("UDDedushkaM"))
    out.append("triSimFrames 120\n")
    out.append(shot("00_warmup_discard", (0, 60, 1.7), (0, 0, 20)))
    out.append(shot("dd_01_touch_toes_1m5", (5, 7.5, 1.7), (5, 6, 1.2), frames=20))
    out.append(shot("dd_02_toes_and_soldier", (10.5, 11.5, 1.7), (7, 5.5, 1.4), frames=12))
    out.append(shot("dd_03_shin_from_feet", (5, 10, 1.7), (5, 2, 5), frames=12))
    out.append(shot("dd_04_foot_and_tank", (15, 19, 1.7), (7, 2, 4), frames=12))
    out.append(shot("dd_05_feet_up", (0, 14, 1.7), (0, 0, 34), frames=12))
    out.append(shot("dd_06_under_pelvis", (0, 1, 1.7), (0, 0, 18.5), frames=12))
    out.append(shot("dd_07_60m", (0, 60, 1.7), (0, 0, 20)))
    out.append("triPerfReset 0\n")
    out.append("triSimFrames 150\n")
    out.append('logInfo format ["TEXRES detail demo, 150 frames at 60 m: %1", triPerfStats 0]\n')
    out.append('logInfo format ["TEXRES end: time %1 gl errors %2", time, triGetGLErrorCount]\n')
    out.append("triEndTest\n")
    return "".join(out)


TOML = '''# %(title)s (experiment, kept out of tests/integration so a sweep never
# runs it). Needs @kj_texres (features/texres/build_texres.py) beside the data
# dir; it holds its own uddedushka.pbo and is mounted IN PLACE OF the shared
# @uddedushka. Run it through features/texres/run-texres.ps1: one game, hidden
# desktop, --render gl33. Quiet: nobody fires.
tags = ["kaiju", "experiment", "texres"]
binary = "PoseidonGame"
mission = "experiments/kaiju/features/texres/TexRes.Eden"
timeout = %(timeout)d
assert_timeout = 120
extra_args = ["--nosound", "--no-strict", "--width", "1920", "--height", "1080", "--mods-dir", "..", "--mod", "@kj_texres"]
'''


def main():
    for name, text, title, timeout in (("texres_charts", charts(), "Dedushka texture charts", 700),
                                       ("texres_limits", limits(), "Texture limits panels", 420),
                                       ("texres_detail", detail(), "Detail layer demo frames", 300)):
        with open(os.path.join(HERE, name + ".test.sqf"), "w", encoding="utf-8", newline="\n") as fh:
            fh.write(text)
        with open(os.path.join(HERE, name + ".test.toml"), "w", encoding="utf-8", newline="\n") as fh:
            fh.write(TOML % {"title": title, "timeout": timeout})
        print("wrote %s.test.sqf (%d lines)" % (name, text.count("\n")))


if __name__ == "__main__":
    main()
