"""Generate the shade-track research capture tests (Abel + Sinai).

Each view: Classic, an albedo shot (remastered debug 3 = albedo, post off),
Remastered defaults, then one shot per variant (knobs via triSetLighting, reset
to defaults first). Labels: <view>__classic, <view>__albedo, <view>__remastered,
<view>__<variant>.

    python gen_tests.py <run-name> <variant-set>

writes arma_CWR/tests/rendering/research/shade/<run>_{abel,sinai}.test.{sqf,toml}
"""
import sys
from pathlib import Path

OUT = Path(r"D:/Arma_CWA_worktrees/lighting/arma_CWR/tests/rendering/research/shade")

# id, world, x, z, eye, heading, pitch, date, overcast, fog, vd, props
# props: (classes, fwd, right, reldir, kind)
VIEWS = {
    "lowsun_fp": ("Abel", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500, [
        (["Jeep"], 10, -5, -107.5, "veh"),
        (["M113"], 16, 5, 72.5, "veh"),
        (["SoldierWB"], 7, 1, -112.5, "man"),
        (["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"),
    ]),
    # Side-lit calibration box: sun 90 deg left of the heading; the M113 and the
    # Jeep show their left side to the sun at 45 deg (lit) and their rear 135 deg
    # from the sun (shade), both 45 deg to the camera.
    "lowsun_box": ("Abel", 7300, 10900, 1.75, 207.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500, [
        (["M113"], 15, 3, -45, "veh"),
        (["Jeep"], 10, -4.5, -45, "veh"),
        (["SoldierWB"], 6, 0.5, -60, "man"),
    ]),
    "forest_edge_golden": ("Abel", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500, [
        (["SoldierWB"], 12, 2, 215, "man"),
    ]),
    # Evening hull warmth: 18:45 on 5 July, sun 6.7 deg at az 294.5, 90 deg left.
    "evening_box": ("Abel", 4674, 10156, 1.75, 24.5, -3, [1985, 7, 5, 18, 45], 0, 0, 1500, [
        (["M113"], 15, 3, -45, "veh"),
        (["Jeep"], 10, -4.5, -45, "veh"),
        (["SoldierWB"], 6, 0.5, -60, "man"),
    ]),
    # Evening hull warmth on the west coast: 18:35 on 5 July, sun 8.4 deg at az
    # 293 over the open sea, 90 deg left of the heading; props inland (right).
    "coast_box": ("Abel", 1550, 5150, 1.75, 23, -3, [1985, 7, 5, 18, 35], 0, 0, 2500, [
        (["M113"], 15, 6, -45, "veh"),
        (["Jeep"], 10, 2, -45, "veh"),
        (["SoldierWB"], 6, 1.5, -60, "man"),
    ]),
    "sunrise_ridge": ("Abel", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000, []),
    "coastline_sunset": ("Abel", 1550, 5150, 3, 278, -4, [1985, 7, 5, 18, 35], 0, 0, 4000, []),
    "noon_fields": ("Abel", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500, [
        (["Jeep"], 13, 3, 60, "veh"),
        (["SoldierWB"], 8, -2, 160, "man"),
    ]),
    "sinai_lowsun_fp": ("Sinai", 8350, 2950, 1.75, 338.8, -3, [2005, 7, 21, 16, 10], 0, 0, 2000, [
        (["LoBoHMWV_Mag_Patrol", "Jeep"], 10, -5, -107.8, "veh"),
        (["LoBo_Zelda", "M113"], 16, 5, 72.2, "veh"),
        (["LoBoGolaniWB", "SoldierWB"], 7, 1, -112.8, "man"),
        (["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.8, "veh"),
    ]),
    # Sinai side-lit box: 16:10 on 21 July, sun 12.2 deg at az 303.8, 90 deg left.
    "sinai_box": ("Sinai", 8350, 2950, 1.75, 33.8, -3, [2005, 7, 21, 16, 10], 0, 0, 2000, [
        (["LoBo_Zelda", "M113"], 15, 3, -45, "veh"),
        (["LoBoHMWV_Mag_Patrol", "Jeep"], 10, -4.5, -45, "veh"),
        (["LoBoGolaniWB", "SoldierWB"], 6, 0.5, -60, "man"),
    ]),
    "sinai_desert_noon": ("Sinai", 7300, 10300, 1.75, 210, 1, [2005, 7, 21, 12, 15], 0, 0, 4000, [
        (["LoBoHMWV_Mag_Patrol", "Jeep"], 14, 4, 60, "veh"),
        (["LoBoGolaniWB", "SoldierWB"], 8, -2, 160, "man"),
    ]),
}

VARIANT_SETS = {
    "none": [],
    "v4set": [
        ("v4s", ["localLights=4", "ao=0.71"], True),
        ("v9s", ["localLights=9", "ao=0.71"]),
        ("v10s", ["localLights=10", "ao=0.71"]),
        ("v11s", ["localLights=11", "ao=0.71"]),
    ],
    "v3set": [
        ("v2", ["localLights=2"]),
        ("v4", ["localLights=4"]),
        ("v4s", ["localLights=4", "ao=0.71"]),
        ("v7", ["localLights=7"]),
        ("v8", ["localLights=8"]),
        ("v8s", ["localLights=8", "ao=0.71"], True),
        ("v1s", ["ao=0.71"]),
    ],
    "v2set": [
        ("v2", ["localLights=2"]),
        ("v3", ["localLights=3"]),
        ("v4", ["localLights=4"]),
        ("v5", ["localLights=5"], True),
        ("v6", ["localLights=6"]),
        ("v4ao35", ["localLights=4", "ao=0.35"]),
        ("v1sky15", ["sky=1.5"]),
    ],
    "diag2": [
        ("prelit", ["debug=2", "post=0"]),
    ],
    "diag": [
        ("post0", ["post=0"]),
        ("dbg1", ["debug=1", "post=0"]),
        ("dbg2", ["debug=2", "post=0"]),
        ("shafts0", ["shafts=0"]),
        ("bloom0", ["bloom=0"]),
    ],
    "sweep1": [
        ("ao0", ["ao=0"]),
        ("sky15", ["sky=1.5"]),
        ("sky20", ["sky=2"]),
        ("bnc25", ["bounce=2.5"]),
        ("sky15b2", ["sky=1.5", "bounce=2"]),
        ("exp12", ["exposure=1.2"]),
        ("warm16", ["warmth=1.6"]),
        ("sat100", ["saturation=1"]),
        ("tocc0", ["terrainOcclusion=0"]),
    ],
}

HEADER = """// GENERATED by tmp/research/shade/scripts/gen_tests.py - shade research only.
#include "shade_lib.sqf"

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
capProbe = "HeliHEmpty" createVehicle [{x}, {z}, 0]
capCam = "camera" camCreate [{x}, {z}, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call {{ "view=setup;event=setup;world={world};msaa=4;scale=1.5;window=960x640;capture=present;run={run}" call capLog }}
"""


def sqf_list(v):
    return "[" + ", ".join(str(x) for x in v) + "]"


def view_block(vid, spec, variants):
    world, x, z, eye, h, p, date, oc, fog, vd, props = spec
    out = [f"// ---- {vid}",
           f'["{vid}", {x}, {z}, {eye}, {h}, {p}, {sqf_list(date)}, {oc}, {fog}, {vd}] call capBegin']
    if props:
        out.append("call {")
        for cls, f, r, d, kind in props:
            cl = "[" + ", ".join(f'"{c}"' for c in cls) + "]"
            out.append(f'    [{cl}, {f}, {r}, {d}, "{kind}"] call capSpawn;')
        out.append("}")
    out += ["triSimFrames 150",
            '"classic" call capMode',
            "triSimFrames 90",
            "[] call capReadTimers",
            "[] call capPin",
            "triSimFrames 3",
            f'triScreenshot "{vid}__classic"',
            f'"{vid}__classic" call capRecord',
            '"remastered" call capMode',
            "call { [] call capResetTuning }",
            "triSimFrames 90",
            "[] call capReadTimers",
            "[] call capPin",
            "triSimFrames 3",
            f'triScreenshot "{vid}__remastered"',
            f'"{vid}__remastered" call capRecord']
    for var in [("albedo", ["debug=3", "post=0"])] + variants:
        label, knobs = var[0], var[1]
        timed = len(var) > 2 and var[2]
        kn = "[" + ", ".join(f'"{k}"' for k in knobs) + "]"
        out += [f'call {{ ["{label}", {kn}] call capVariant }}']
        if timed:
            out += ["call { capGpuOff = triGpuTimes false; capGpuOn = triGpuTimes true; capPerfRes = triPerfReset 0 }",
                    "triSimFrames 90", "[] call capReadTimers"]
        else:
            out += ["triSimFrames 8"]
        out += ["[] call capPin",
                "triSimFrames 3",
                f'triScreenshot "{vid}__{label}"',
                f'"{vid}__{label}" call capRecord']
    out += ["call { [] call capResetTuning }", "[] call capClear", ""]
    return "\n".join(out)


TOML = {
    "Abel": """binary = "PoseidonGame"
mission = "tests/rendering/captures/missions/udcap_abel.Abel"
tags = ["lighting_capture", "headful", "exclusive"]
timeout = 900
assert_timeout = 180
extra_args = ["--no-strict", "--display-mode", "windowed", "--width", "960", "--height", "640"]
""",
    "Sinai": """binary = "PoseidonGame"
mission = "tests/rendering/captures/missions/udcap_sinai.Sinai"
tags = ["lighting_capture", "lobo", "headful", "exclusive"]
timeout = 900
assert_timeout = 240
extra_args = ["--no-strict", "--mods-dir", ".", "--mod", "@LoBo", "--display-mode", "windowed", "--width", "960", "--height", "640"]
""",
}


def main():
    run = sys.argv[1]
    vset = VARIANT_SETS[sys.argv[2]]
    views = sys.argv[3].split(",") if len(sys.argv) > 3 else list(VIEWS)
    for world, tag in (("Abel", "abel"), ("Sinai", "sinai")):
        vids = [v for v in views if VIEWS[v][0] == world]
        if not vids:
            continue
        x, z = VIEWS[vids[0]][1], VIEWS[vids[0]][2]
        body = HEADER.format(world=world, x=x, z=z, run=run)
        body += "\n".join(view_block(v, VIEWS[v], vset) for v in vids)
        body += '\ncall { "view=done;event=done" call capLog }\ntriEndTest\n'
        name = f"{run}_{tag}"
        (OUT / f"{name}.test.sqf").write_text(body, encoding="utf-8", newline="\n")
        (OUT / f"{name}.test.toml").write_text(TOML[world], encoding="utf-8", newline="\n")
        print("wrote", OUT / f"{name}.test.sqf", len(vids), "views")


if __name__ == "__main__":
    main()
