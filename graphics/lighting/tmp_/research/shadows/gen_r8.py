"""Round 8 (resumed session): a cheaper production candidate and the "only darker" risk.

    python gen_r8.py

Variants per view: classic, rem, prod (tree/building casters + PCSS 1 deg on the stock
2048 map with stock caster LOD and the tighter near tiers), prodexp (prod with
exposure x1.25, a stand-in for a scene-adaptive exposure meter), best (r4 best),
and the sun-visibility debug of prod.
Writes <gfx-shadows>/arma_CWR/tests/rendering/research/shadows/abel_shadows_r8.test.sqf + .toml.
"""
from gen_tests import OUT, HEADER, STOCK, OBJ, NOOBJ, BEST, ABEL_VIEWS, ABEL_VIEWS_R3, sqf_list, shot_block
from gen_r7 import WOODS

E1 = ["exposure=1"]  # triSetLighting "remastered" does not reset knobs: round 8 v1 leaked exposure=1.25 into later shots
PROD = STOCK + E1 + OBJ + ["pcssSun=1", "casterLod=3"]
VARIANTS = [
    ("rem", STOCK + E1 + NOOBJ, 2048),
    ("prod", PROD, 2048, (0.04, 0.15)),
    ("prodexp", PROD + ["exposure=1.25"], 2048, (0.04, 0.15)),
    ("best", BEST + E1, 4096, (0.04, 0.15)),
    ("dbgsunvisprod", PROD + ["debug=1"], 2048, (0.04, 0.15)),
]
PINE2 = [
    ("woods_pine2_morning", '["woods_pine2_morning", 6940, 9964, 1.75, 240, -5, [1985, 7, 5, 10, 0], 0, 0, 1500] call capBegin',
     ['[["Jeep"], 14, 2, 70, "veh"] call capSpawn;',
      '[["SoldierWB"], 9, -2, 150, "man"] call capSpawn;']),
    ("woods_pine2_evening", '["woods_pine2_evening", 6940, 9964, 1.75, 240, -5, [1985, 7, 5, 17, 30], 0, 0, 1500] call capBegin',
     ['[["Jeep"], 14, 2, 70, "veh"] call capSpawn;',
      '[["SoldierWB"], 9, -2, 150, "man"] call capSpawn;']),
]


def main():
    by = {v[0]: v for v in ABEL_VIEWS_R3 + WOODS + PINE2}
    views = [by[n] for n in ("woods_pine2_morning", "woods_pine2_evening", "woods_mixed_noon", "town_golden", "forest_edge_golden", "lowsun_fp")]
    lines = [HEADER.format(world="Abel", ox=6940, oz=9964)]
    for view, begin, spawns in views:
        lines += ["", "// ---- " + view, begin, "call {"] + ["    " + s for s in spawns] + ["}", "triSimFrames 150", '"classic" call capMode']
        lines += shot_block(view, "classic")
        for v in VARIANTS:
            label, settings, res = v[0], v[1], v[2]
            omni = v[3] if len(v) > 3 else (0.08, 0.20)
            lines.append('["%s", %s, %d, %g, %g, 1.0] call capVariant' % (label, sqf_list(settings), res, omni[0], omni[1]))
            lines += shot_block(view, label)
        lines.append('["reset", %s, 2048, 0.08, 0.2] call capVariant' % sqf_list(STOCK + E1))
        lines.append("[] call capClear")
    lines.append('call { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    sqf = OUT / "abel_shadows_r8.test.sqf"
    sqf.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
    (OUT / "abel_shadows_r8.test.toml").write_text(
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

