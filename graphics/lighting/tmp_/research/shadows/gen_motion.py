"""Generate the motion capture test for the shadows track (round 6).

    python gen_motion.py

Writes <gfx-shadows>/arma_CWR/tests/rendering/research/shadows/abel_shadows_motion.test.sqf
+ .toml. Per view and variant (rem, obj, best): NSTATIC frames with the camera
fixed, then NMOVE frames with the camera dollying sideways by STEP metres per
frame. The sim clock is pinned per frame index, so every variant sees the
same camera and the same clock on frame k; differences between variants are
the variant's own (shadow-map swimming, PCSS dither, acne).
"""
from gen_tests import OUT, HEADER, STOCK, OBJ, NOOBJ, BEST, ABEL_VIEWS, sqf_list

NSTATIC = 4
NMOVE = 16
STEP = 0.02  # metres per frame, to the right of the view heading

VARIANTS = [
    ("rem", STOCK + NOOBJ, 2048, (0.08, 0.20)),
    ("obj", STOCK + OBJ, 2048, (0.08, 0.20)),
    ("best", BEST, 4096, (0.04, 0.15)),
]

MOTION_LIB = """
// [frame index, metres to the right] call capMoveTo: camera to the base view
// shifted sideways, sim clock pinned to base + k frames.
capMoveTo = {
    private ["_k", "_s"];
    _k = _this select 0;
    _s = _this select 1;
    capOX = capBX + _s * (cos capH);
    capOZ = capBZ - _s * (sin capH);
    capSimRes = triSetSimTime (capSimT + _k / 30);
    [] call capPinView;
}
"""


def main():
    views = [v for v in ABEL_VIEWS if v[0] in ("forest_edge_golden", "lowsun_fp")]
    lines = [HEADER.format(world="Abel", ox=7300, oz=10900), MOTION_LIB]
    for view, begin, spawns in views:
        lines += ["", "// ---- " + view, begin]
        lines.append("call {")
        lines += ["    " + s for s in spawns]
        lines.append("}")
        lines.append("capBX = capOX")
        lines.append("capBZ = capOZ")
        lines.append("triSimFrames 150")
        for label, settings, res, omni in VARIANTS:
            lines.append('["m%s", %s, %d, %g, %g, 1.0] call capVariant' % (label, sqf_list(settings), res, omni[0], omni[1]))
            lines.append("triSimFrames 90")
            lines.append("[] call capReadTimers")
            lines.append("[] call capPin")
            for k in range(NSTATIC + NMOVE):
                s = 0.0 if k < NSTATIC else (k - NSTATIC + 1) * STEP
                tag = ("s%02d" % k) if k < NSTATIC else ("m%02d" % (k - NSTATIC))
                lines.append("[%d, %g] call capMoveTo" % (k, s))
                lines.append("triSimFrames 2")
                lines.append('triScreenshot "%s__%s_%s"' % (view, label, tag))
            lines.append('"%s__%s_seq" call capRecord' % (view, label))
            lines.append("[0, 0] call capMoveTo")
        lines.append('["reset", %s, 2048, 0.08, 0.2] call capVariant' % sqf_list(STOCK))
        lines.append("[] call capClear")
    lines.append('call { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    sqf = OUT / "abel_shadows_motion.test.sqf"
    sqf.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
    toml = OUT / "abel_shadows_motion.test.toml"
    toml.write_text(
        'binary = "PoseidonGame"\n'
        'mission = "tests/rendering/captures/missions/udcap_abel.Abel"\n'
        'tags = ["lighting_capture", "headful", "exclusive"]\n'
        "timeout = 900\n"
        "assert_timeout = 180\n"
        'extra_args = ["--no-strict", "--display-mode", "windowed", "--width", "960", "--height", "640"]\n',
        encoding="utf-8", newline="\n")
    print("wrote", sqf, toml)


if __name__ == "__main__":
    main()
