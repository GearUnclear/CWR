"""Generate the research capture tests for the shadows track.

    python gen_tests.py <round>

Writes <gfx-shadows>/arma_CWR/tests/rendering/research/shadows/
{abel,sinai}_shadows_<round>.test.sqf + .toml. Each view shoots Classic, then
every remastered variant (triSetLighting key=value list + shadow-map res).
"""
import sys
from pathlib import Path

OUT = Path(r"D:\Arma_CWA_worktrees\gfx-shadows\arma_CWR\tests\rendering\research\shadows")

STOCK = ["debug=0", "objShadows=0", "pcssSun=0", "contact=0", "contactFar=0", "contactSteps=16", "contactFade=120", "casterLod=3"]
FAR = ["contact=1", "contactLen=1", "contactThick=0.35", "contactFar=0.04", "contactFarThick=0.02", "contactFarMax=40", "contactSteps=24", "contactFade=1500"]

ROUNDS = {
    "r1": [
        ("rem", STOCK, 2048),
        ("contact", ["debug=0", "pcssSun=0", "contact=1", "contactLen=1", "contactThick=0.35", "casterLod=3"], 2048),
        ("pcss05", ["debug=0", "pcssSun=0.53", "contact=1", "casterLod=3"], 4096),
        ("pcss2", ["debug=0", "pcssSun=2", "contact=1", "casterLod=3"], 4096),
        ("pcss2lod1", ["debug=0", "pcssSun=2", "contact=1", "casterLod=1"], 4096),
    ],
}
ROUNDS["r2"] = [
    ("rem", STOCK, 2048),
    ("pcss1", STOCK + ["pcssSun=1"], 4096),
    ("pcss1tight", STOCK + ["pcssSun=1"], 4096, (0.04, 0.15)),
    ("far", STOCK + FAR, 2048),
    ("combo", STOCK + FAR + ["pcssSun=1", "casterLod=1"], 4096, (0.04, 0.15)),
    ("combo3", STOCK + FAR + ["pcssSun=3", "casterLod=1"], 4096, (0.04, 0.15)),
]
OBJ = ["objShadows=1"]
NOOBJ = ["objShadows=0"]
ROUNDS["r3"] = [
    ("rem", STOCK + NOOBJ, 2048),
    ("obj", STOCK + OBJ, 2048),
    ("objpcss", STOCK + OBJ + ["pcssSun=1", "casterLod=1"], 4096, (0.04, 0.15)),
    ("objfull", STOCK + OBJ + ["pcssSun=2", "casterLod=1", "contact=1", "contactLen=1", "contactThick=0.35"], 4096, (0.04, 0.15)),
]
_C = [("rem", STOCK + NOOBJ, 2048),
      ("obj", STOCK + OBJ, 2048),
      ("objpcss", STOCK + OBJ + ["pcssSun=1", "casterLod=1"], 4096, (0.04, 0.15)),
      ("objfull", STOCK + OBJ + ["pcssSun=2", "casterLod=1", "contact=1", "contactLen=1", "contactThick=0.35"], 4096, (0.04, 0.15)),
      ("pcssonly", STOCK + NOOBJ + ["pcssSun=2", "casterLod=3"], 2048),
      ("contactonly", STOCK + NOOBJ + ["contact=1", "contactLen=1", "contactThick=0.35"], 2048)]
ROUNDS["cost"] = [(lab + "_%d" % k,) + tuple(v[1:]) for k in range(1, 4) for (lab, *rest) in _C for v in [(lab, *rest)]]
BEST = STOCK + OBJ + ["pcssSun=1", "casterLod=1", "contact=1", "contactLen=1", "contactThick=0.35"]
ROUNDS["r4"] = [
    ("rem", STOCK + NOOBJ, 2048),
    ("best", BEST, 4096, (0.04, 0.15)),
    ("bestlong", BEST, 4096, (0.04, 0.15), 2.0),
]
_C = [("rem", STOCK + NOOBJ, 2048),
      ("obj", STOCK + OBJ, 2048),
      ("best", BEST, 4096, (0.04, 0.15)),
      ("bestlong", BEST, 4096, (0.04, 0.15), 2.0),
      ("best2048", BEST, 2048),
      ("pcssonly", STOCK + NOOBJ + ["pcssSun=1", "casterLod=3"], 2048),
      ("contactonly", STOCK + NOOBJ + ["contact=1", "contactLen=1", "contactThick=0.35"], 2048)]
ROUNDS["cost"] = [(v[0] + "_%d" % k,) + tuple(v[1:]) for k in range(1, 4) for v in _C]
_C2 = [("rem", STOCK + NOOBJ, 2048),
       ("pcssonly", STOCK + NOOBJ + ["pcssSun=1", "casterLod=3"], 2048),
       ("contactonly", STOCK + NOOBJ + ["contact=1", "contactLen=1", "contactThick=0.35"], 2048),
       ("obj", STOCK + OBJ, 2048),
       ("best", BEST, 4096, (0.04, 0.15))]
ROUNDS["cost2"] = [(v[0] + "_%d" % k,) + tuple(v[1:]) for k in range(1, 9) for v in _C2]
# Extra diagnostic shots (r2: on every view).
DEBUG = {
    "r4": [("dbgsunvisbest", BEST + ["debug=1"], 4096, (0.04, 0.15)),
           ("dbgsunvisbestlong", BEST + ["debug=1"], 4096, (0.04, 0.15), 2.0)],
    "r3": [
        ("dbgsunvisobj", STOCK + OBJ + ["debug=1"], 2048),
        ("dbgsunvisobjpcss", STOCK + OBJ + ["pcssSun=1", "casterLod=1", "debug=1"], 4096, (0.04, 0.15)),
    ],
    "r2": [
        ("dbgsunvisrem", STOCK + ["debug=1"], 2048),
        ("dbgsunviscombo", STOCK + ["pcssSun=1", "casterLod=1", "debug=1"], 4096, (0.04, 0.15)),
        ("dbgfar", STOCK + FAR + ["debug=5"], 2048),
    ],
    "r1": [
        ("dbgcontact", ["pcssSun=0", "contact=1", "casterLod=3", "debug=5"], 2048),
        ("dbgsunvisrem", ["pcssSun=0", "contact=0", "casterLod=3", "debug=1"], 2048),
        ("dbgsunvispcss2", ["pcssSun=2", "contact=0", "casterLod=3", "debug=1"], 4096),
    ],
}

ABEL_VIEWS = [
    ("lowsun_fp", '["lowsun_fp", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin',
     ['[["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;',
      '[["M113"], 16, 5, 72.5, "veh"] call capSpawn;',
      '[["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;',
      '[["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;']),
    ("noon_fields", '["noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin',
     ['[["Jeep"], 13, 3, 60, "veh"] call capSpawn;',
      '[["SoldierWB"], 8, -2, 160, "man"] call capSpawn;']),
    ("forest_edge_golden", '["forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin',
     ['[["SoldierWB"], 12, 2, 215, "man"] call capSpawn;']),
]
ABEL_VIEWS_R3 = ABEL_VIEWS + [
    ("forest_side_golden", '["forest_side_golden", 4674, 10156, 1.75, 40, 2, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin',
     ['[["SoldierWB"], 10, 3, 200, "man"] call capSpawn;']),
    ("town_golden", '["town_golden", 7152, 8915, 1.75, 345, 2, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin',
     ['[["Jeep"], 12, 4, 60, "veh"] call capSpawn;',
      '[["SoldierWB"], 8, -2, 160, "man"] call capSpawn;']),
]
SINAI_VIEWS = [
    ("sinai_lowsun_fp", '["sinai_lowsun_fp", 8350, 2950, 1.75, 338.8, -3, [2005, 7, 21, 16, 10], 0, 0, 2000] call capBegin',
     ['[["LoBoHMWV_Mag_Patrol", "Jeep"], 10, -5, -107.8, "veh"] call capSpawn;',
      '[["LoBo_Zelda", "M113"], 16, 5, 72.2, "veh"] call capSpawn;',
      '[["LoBoGolaniWB", "SoldierWB"], 7, 1, -112.8, "man"] call capSpawn;',
      '[["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.8, "veh"] call capSpawn;']),
]

HEADER = """// RESEARCH (shadows track) - generated by tmp/research/shadows/gen_tests.py. Not committed.
#include "capture_lib.sqf"

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

triCaptureAtPresent true
triSetMsaa 4
triSetRenderScale 1.5
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false

createCenter west
createCenter east
capGrpW = createGroup west
capProbe = "HeliHEmpty" createVehicle [{ox}, {oz}, 0]
capCam = "camera" camCreate [{ox}, {oz}, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call {{ "view=setup;event=setup;world={world};msaa=4;scale=1.5;window=960x640;capture=present" call capLog }}
"""


def sqf_list(items):
    return "[" + ", ".join('"%s"' % s for s in items) + "]"


def shot_block(view, label):
    if COST:
        return [
            "triSimFrames 120",
            "[] call capReadTimers",
            '"%s__%s" call capRecord' % (view, label),
        ]
    return [
        "triSimFrames 90",
        "[] call capReadTimers",
        "[] call capPin",
        "triSimFrames 3",
        'triScreenshot "%s__%s"' % (view, label),
        '"%s__%s" call capRecord' % (view, label),
    ]


COST = False


def view_block(view, begin, spawns, variants, debug):
    lines = ["", "// ---- " + view, begin]
    if spawns:
        lines.append("call {")
        lines += ["    " + s for s in spawns]
        lines.append("}")
    lines.append("triSimFrames 150")
    lines.append('"classic" call capMode')
    lines += shot_block(view, "classic")
    for v in variants + debug:
        label, settings, res = v[0], v[1], v[2]
        omni = v[3] if len(v) > 3 else (0.08, 0.20)
        dist = v[4] if len(v) > 4 else 1.0
        lines.append('["%s", %s, %d, %g, %g, %g] call capVariant' % (label, sqf_list(settings), res, omni[0], omni[1], dist))
        lines += shot_block(view, label)
    # leave the next view on stock settings
    lines.append('["reset", %s, 2048, 0.08, 0.2] call capVariant' % sqf_list(STOCK))
    lines.append("[] call capClear")
    return lines


def write(name, world, ox, oz, views, rnd, toml_extra, mission, timeout, assert_timeout):
    variants = ROUNDS[rnd]
    debug = DEBUG.get(rnd, [])
    lines = [HEADER.format(world=world, ox=ox, oz=oz)]
    for i, (view, begin, spawns) in enumerate(views):
        lines += view_block(view, begin, spawns, variants, debug if (i == 0 or rnd != "r1") else [])
    lines.append('call { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    sqf = OUT / ("%s_%s.test.sqf" % (name, rnd))
    sqf.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
    toml = OUT / ("%s_%s.test.toml" % (name, rnd))
    toml.write_text(
        'binary = "PoseidonGame"\n'
        'mission = "%s"\n'
        'tags = ["lighting_capture", "headful", "exclusive"]\n'
        "timeout = %d\n"
        "assert_timeout = %d\n"
        "extra_args = %s\n" % (mission, timeout, assert_timeout, toml_extra),
        encoding="utf-8", newline="\n")
    print("wrote", sqf, toml)


def main():
    global COST
    rnd = sys.argv[1] if len(sys.argv) > 1 else "r1"
    if rnd.startswith("cost"):
        COST = True
    OUT.mkdir(parents=True, exist_ok=True)
    views = ABEL_VIEWS_R3 if rnd in ("r3", "r4") else ABEL_VIEWS
    if COST:
        keep = ("lowsun_fp", "forest_edge_golden") if rnd == "cost2" else ("lowsun_fp", "forest_edge_golden", "town_golden")
        views = [v for v in ABEL_VIEWS_R3 if v[0] in keep]
    write("abel_shadows", "Abel", 7300, 10900, views, rnd,
          '["--no-strict", "--display-mode", "windowed", "--width", "960", "--height", "640"]',
          "tests/rendering/captures/missions/udcap_abel.Abel", 900, 180)
    if COST:
        return
    write("sinai_shadows", "Sinai", 8350, 2950, SINAI_VIEWS, rnd,
          '["--no-strict", "--mods-dir", ".", "--mod", "@LoBo", "--display-mode", "windowed", "--width", "960", "--height", "640"]',
          "tests/rendering/captures/missions/udcap_sinai.Sinai", 900, 240)


if __name__ == "__main__":
    main()
