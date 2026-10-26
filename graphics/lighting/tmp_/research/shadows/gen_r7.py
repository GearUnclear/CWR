"""Round 7 (resumed session): woodland views for tree casters, and a sun-visibility
motion sequence that isolates shadow swimming.

    python gen_r7.py

Writes <gfx-shadows>/arma_CWR/tests/rendering/research/shadows/abel_shadows_r7.test.sqf + .toml.
Spots were picked from `PoseidonTools terrain objects abel.wrp` (dense pine and mixed
woodland, flat within 25 m, no object within 4 m of the eye, nearest tree along the
heading 19 m / 6 m off-axis).
"""
from gen_tests import OUT, HEADER, STOCK, OBJ, NOOBJ, BEST, ABEL_VIEWS, sqf_list, shot_block
from gen_motion import MOTION_LIB

STILL = [
    ("rem", STOCK + NOOBJ, 2048),
    ("obj", STOCK + OBJ, 2048),
    ("objpcss2k", STOCK + OBJ + ["pcssSun=1", "casterLod=1"], 2048),
    ("best", BEST, 4096, (0.04, 0.15)),
]
WOODS = [
    ("woods_pine_morning", '["woods_pine_morning", 6888, 9944, 1.75, 210, -6, [1985, 7, 5, 10, 0], 0, 0, 1500] call capBegin',
     ['[["Jeep"], 12, 2, 70, "veh"] call capSpawn;',
      '[["SoldierWB"], 7, -1.5, 150, "man"] call capSpawn;']),
    ("woods_pine_evening", '["woods_pine_evening", 6888, 9944, 1.75, 210, -6, [1985, 7, 5, 17, 30], 0, 0, 1500] call capBegin',
     ['[["Jeep"], 12, 2, 70, "veh"] call capSpawn;',
      '[["SoldierWB"], 7, -1.5, 150, "man"] call capSpawn;']),
    ("woods_mixed_noon", '["woods_mixed_noon", 2864, 6656, 1.75, 300, -5, [1985, 7, 5, 14, 0], 0, 0, 1500] call capBegin',
     ['[["SoldierWB"], 8, 1, 200, "man"] call capSpawn;']),
]
NSEQ = 12
STEP = 0.02


def main():
    lines = [HEADER.format(world="Abel", ox=6888, oz=9944), MOTION_LIB]
    for view, begin, spawns in WOODS:
        lines += ["", "// ---- " + view, begin, "call {"] + ["    " + s for s in spawns] + ["}", "triSimFrames 150", '"classic" call capMode']
        lines += shot_block(view, "classic")
        for v in STILL:
            label, settings, res = v[0], v[1], v[2]
            omni = v[3] if len(v) > 3 else (0.08, 0.20)
            lines.append('["%s", %s, %d, %g, %g, 1.0] call capVariant' % (label, sqf_list(settings), res, omni[0], omni[1]))
            lines += shot_block(view, label)
        lines.append('["dbgsunvisbest", %s, 4096, 0.04, 0.15, 1.0] call capVariant' % sqf_list(BEST + ["debug=1"]))
        lines += shot_block(view, "dbgsunvisbest")
        lines.append('["reset", %s, 2048, 0.08, 0.2] call capVariant' % sqf_list(STOCK))
        lines.append("[] call capClear")
    # sun-visibility (debug=1) dolly on forest_edge_golden
    fe = [v for v in ABEL_VIEWS if v[0] == "forest_edge_golden"][0]
    view, begin, spawns = fe
    lines += ["", "// ---- motion, sun visibility", begin, "call {"] + ["    " + s for s in spawns] + ["}", "capBX = capOX", "capBZ = capOZ", "triSimFrames 150"]
    for label, settings, res, omni in [("vrem", STOCK + NOOBJ + ["debug=1"], 2048, (0.08, 0.20)),
                                       ("vobj", STOCK + OBJ + ["debug=1"], 2048, (0.08, 0.20)),
                                       ("vbest", BEST + ["debug=1"], 4096, (0.04, 0.15))]:
        lines.append('["%s", %s, %d, %g, %g, 1.0] call capVariant' % (label, sqf_list(settings), res, omni[0], omni[1]))
        lines += ["triSimFrames 90", "[] call capPin"]
        for k in range(NSEQ):
            lines.append("[%d, %g] call capMoveTo" % (k, (k + 1) * STEP))
            lines.append("triSimFrames 2")
            lines.append('triScreenshot "%s__%s_m%02d"' % (view, label, k))
        lines.append("[0, 0] call capMoveTo")
    lines.append('["reset", %s, 2048, 0.08, 0.2] call capVariant' % sqf_list(STOCK))
    lines.append("[] call capClear")
    lines.append('call { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    sqf = OUT / "abel_shadows_r7.test.sqf"
    sqf.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
    (OUT / "abel_shadows_r7.test.toml").write_text(
        'binary = "PoseidonGame"\n'
        'mission = "tests/rendering/captures/missions/udcap_abel.Abel"\n'
        'tags = ["lighting_capture", "headful", "exclusive"]\n'
        "timeout = 900\n"
        "assert_timeout = 180\n"
        'extra_args = ["--no-strict", "--display-mode", "windowed", "--width", "960", "--height", "640"]\n',
        encoding="utf-8", newline="\n")
    print("wrote", sqf)


if __name__ == "__main__":
    main()
