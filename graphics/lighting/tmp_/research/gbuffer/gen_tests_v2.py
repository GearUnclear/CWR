"""Second round of gbuffer research capture tests (resumed run).

Reuses gen_tests.py (views, header, block writers) and adds:
  v2      GI v2 (shader override) at three strengths, single-frame noise, debug
  timing  interleaved GPU timing rounds (no screenshots)
usage: python gen_tests_v2.py v2|timing
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_tests as g  # noqa: E402

# Every mode sets every knob (triSetLighting knobs persist between modes).
def knobs(gbuffer=0, gi=0, src=0, vbo=0, temporal=1, debug=0, radius=6, steps=14, dirs=8, giao=1):
    return ["gbuffer=%g" % gbuffer, "gi=%g" % gi, "debug=%d" % debug, "giSource=%g" % src,
            "lateVbo=%d" % vbo, "giTemporal=%g" % temporal, "giRadius=%g" % radius,
            "giSteps=%d" % steps, "giDirs=%d" % dirs, "giAo=%g" % giao]


MODES_V2 = [
    ("classic", "classic", knobs(), 90),
    ("remastered", "remastered", knobs(), 90),
    ("rm_vbo", "remastered", knobs(vbo=1), 90),
    ("v2gi", "remastered", knobs(gbuffer=1, gi=1, vbo=1), 90),
    ("v2gi_x2", "remastered", knobs(gbuffer=1, gi=2, vbo=1), 60),
    ("v2gi_x3s", "remastered", knobs(gbuffer=1, gi=3, src=1, vbo=1), 60),
    ("v2gi_1f", "remastered", knobs(gbuffer=1, gi=1, vbo=1, temporal=0), 30),
    ("v2dbg_bounce", "remastered", knobs(gbuffer=1, gi=1, vbo=1, debug=9), 60),
    ("v2dbg_vis", "remastered", knobs(gbuffer=1, gi=1, vbo=1, debug=10), 20),
]


MODES_V3 = [
    ("rm_vbo", "remastered", knobs(vbo=1), 90),
    ("v2gi", "remastered", knobs(gbuffer=1, gi=1, vbo=1), 90),
    ("v3r15", "remastered", knobs(gbuffer=1, gi=1, vbo=1, radius=15, steps=20), 90),
    ("v3r15_x2_noao", "remastered", knobs(gbuffer=1, gi=2, vbo=1, radius=15, steps=20, giao=0), 90),
    ("v3r15_x3s_noao", "remastered", knobs(gbuffer=1, gi=3, src=1, vbo=1, radius=15, steps=20, giao=0), 90),
    ("v3dbg_bounce", "remastered", knobs(gbuffer=1, gi=1, vbo=1, radius=15, steps=20, debug=9), 60),
]

# A script-spawned Jeep parked in the shadow of a script-spawned M113 at low
# sun: does the stock Remastered path (late shapes on software T&L, pre-lit)
# receive the shadow-map shadow? Three Jeeps along the M113's shadow.
g.VIEWS["lowsun_shadow"] = dict(
    args='["lowsun_shadow", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500]',
    props=[
        '[["M113"], 16, 5, 72.5, "veh"] call capSpawn;',
        '[["Jeep"], 12.5, 8.0, 160, "veh"] call capSpawn;',
        '[["Jeep"], 9.5, 11.0, 160, "veh"] call capSpawn;',
        '[["Jeep"], 11, 1.5, 160, "veh"] call capSpawn;',
    ],
)
MODES_SH = [
    ("classic", "classic", knobs(), 90),
    ("remastered", "remastered", knobs(), 90),
    ("rm_vbo", "remastered", knobs(vbo=1), 90),
    ("sundbg_stock", "remastered", knobs(debug=1), 30),
    ("sundbg_vbo", "remastered", knobs(vbo=1, debug=1), 30),
]

MODES_STOCK = [
    ("stock_remastered", "remastered", ["debug=0"], 90),
    ("stock_sundbg", "remastered", ["debug=1"], 30),
]
MODES_NIGHT = [
    ("classic", "classic", knobs(), 90),
    ("remastered", "remastered", knobs(), 90),
    ("rm_vbo", "remastered", knobs(vbo=1), 90),
    ("v2gi", "remastered", knobs(gbuffer=1, gi=1, vbo=1), 90),
    ("v3r15_x3s_noao", "remastered", knobs(gbuffer=1, gi=3, src=1, vbo=1, radius=15, steps=20, giao=0), 90),
    ("v3r15_x3_noao", "remastered", knobs(gbuffer=1, gi=3, vbo=1, radius=15, steps=20, giao=0), 90),
    ("v3dbg_bounce", "remastered", knobs(gbuffer=1, gi=1, vbo=1, radius=15, steps=20, debug=9), 60),
    ("v3dbg_direct", "remastered", knobs(gbuffer=1, gi=1, vbo=1, radius=15, steps=20, debug=7), 30),
]

TOML_ABEL_PRE = g.TOML_ABEL.replace("tests/rendering/captures/missions/udcap_abel.Abel",
                                    "tests/rendering/research/gbuffer/missions/udcap_abelpre.Abel")
MODES_PRE = [
    ("pre_remastered", "remastered", knobs(), 90),
    ("pre_gi", "remastered", knobs(gbuffer=1, gi=1), 90),
    ("pre_dbg_normals", "remastered", knobs(gbuffer=1, gi=1, debug=5), 30),
    ("pre_dbg_direct", "remastered", knobs(gbuffer=1, gi=1, debug=7), 30),
    ("pre_sundbg", "remastered", knobs(debug=1), 30),
]

TOML_ABEL_FIRE = g.TOML_ABEL.replace("tests/rendering/captures/missions/udcap_abel.Abel",
                                     "tests/rendering/research/gbuffer/missions/udcap_abelfire.Abel")
MODES_FIRE = [
    ("fire_remastered", "remastered", knobs(), 90),
    ("fire_gi_x3", "remastered", knobs(gbuffer=1, gi=3, radius=15, steps=20, giao=0), 90),
    ("fire_gi_x3s", "remastered", knobs(gbuffer=1, gi=3, src=1, radius=15, steps=20, giao=0), 90),
    ("fire_dbg_direct", "remastered", knobs(gbuffer=1, gi=1, radius=15, steps=20, debug=7), 30),
    ("fire_dbg_bounce", "remastered", knobs(gbuffer=1, gi=1, radius=15, steps=20, debug=9), 60),
]

MODES_SUN = [
    ("pre_sun1", "remastered", knobs() + ["sun=1", "sky=1"], 90),
    ("pre_sun0", "remastered", knobs() + ["sun=0", "sky=1"], 60),
    ("pre_sky0", "remastered", knobs() + ["sun=1", "sky=0"], 60),
    ("pre_back", "remastered", knobs() + ["sun=1", "sky=1"], 60),
]

def main():
    which = sys.argv[1] if len(sys.argv) > 1 else "v2"
    if which == "v2":
        g.write_test("gb_v2_abel", "Abel", (7300, 10900), g.TOML_ABEL,
                     [g.view_block(v, MODES_V2) for v in ("lowsun_fp", "forest_edge_golden", "town_day")])
        g.write_test("gb_v2_sinai", "Sinai", (8350, 2950), g.TOML_SINAI,
                     [g.view_block("sinai_lowsun_fp", MODES_V2)])
    elif which == "timing":
        tm = [MODES_V2[0], MODES_V2[1], MODES_V2[2],
              ("gbufonly", "remastered", knobs(gbuffer=1, vbo=1), 120), MODES_V2[3]]
        tm = [(l, b, k, 120) for (l, b, k, _) in tm]
        g.write_test("gb_v2_timing", "Abel", (7300, 10900), g.TOML_ABEL,
                     [g.timing_block(v, tm, 4) for v in ("lowsun_fp", "town_day", "forest_edge_golden")])
    elif which == "v3":
        g.write_test("gb_v3_abel", "Abel", (7300, 10900), g.TOML_ABEL,
                     [g.view_block(v, MODES_V3) for v in ("lowsun_fp", "town_day")])
        g.write_test("gb_v3_sinai", "Sinai", (8350, 2950), g.TOML_SINAI,
                     [g.view_block("sinai_lowsun_fp", MODES_V3)])
        tm = [("rm_vbo", "remastered", knobs(vbo=1), 120), ("gbufonly", "remastered", knobs(gbuffer=1, vbo=1), 120),
              ("v2gi", "remastered", knobs(gbuffer=1, gi=1, vbo=1), 120)]
        g.write_test("gb_v3_timing", "Abel", (7300, 10900), g.TOML_ABEL,
                     [g.timing_block(v, tm, 8) for v in ("lowsun_fp", "town_day", "noon_fields")])
    elif which == "shadow":
        g.write_test("gb_shadow", "Abel", (7300, 10900), g.TOML_ABEL, [g.view_block("lowsun_shadow", MODES_SH)])
    elif which == "stock":
        g.write_test("gb_stock_shadow", "Abel", (7300, 10900), g.TOML_ABEL, [g.view_block("lowsun_shadow", MODES_STOCK)])
    elif which == "night":
        g.write_test("gb_v3_night", "Abel", (7152, 8915), g.TOML_ABEL, [g.view_block("town_night", MODES_NIGHT)])
    elif which == "pre":
        g.write_test("gb_pre", "Abel", (7300, 10900), TOML_ABEL_PRE, [g.view_block(v, MODES_PRE) for v in ("noon_fields", "lowsun_fp", "lowsun_shadow")])
    elif which == "fire":
        g.write_test("gb_fire", "Abel", (7152, 8915), TOML_ABEL_FIRE, [g.view_block("town_night", MODES_FIRE)])
        g.write_test("gb_night_novbo", "Abel", (7152, 8915), g.TOML_ABEL, [g.view_block("town_night", MODES_FIRE)])
    elif which == "sun":
        g.write_test("gb_sun", "Abel", (7300, 10900), TOML_ABEL_PRE, [g.view_block(v, MODES_SUN) for v in ("noon_fields", "lowsun_fp", "town_day")])
    else:
        raise SystemExit("unknown set " + which)


if __name__ == "__main__":
    main()
