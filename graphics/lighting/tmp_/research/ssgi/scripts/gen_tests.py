"""Generates the SSGI research capture tests (research copies, never the committed
tests/rendering/captures files).

python gen_tests.py <out dir> <name> <views comma list> <variant set>

Each view shoots: classic, remastered (gi=0), then every GI variant of the set,
all with the same pinned camera, date, weather, sim clock and props.
"""
import sys
from pathlib import Path

VIEWS = {
    "lowsun_fp": ("Abel", '["lowsun_fp", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin\n'
                  'call {\n'
                  '    [["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;\n'
                  '    [["M113"], 16, 5, 72.5, "veh"] call capSpawn;\n'
                  '    [["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;\n'
                  '    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;\n'
                  '}\n', False),
    "noon_fields": ("Abel", '["noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin\n'
                    'call {\n'
                    '    [["Jeep"], 13, 3, 60, "veh"] call capSpawn;\n'
                    '    [["SoldierWB"], 8, -2, 160, "man"] call capSpawn;\n'
                    '}\n', False),
    "forest_edge_golden": ("Abel", '["forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin\n'
                           'call {\n'
                           '    [["SoldierWB"], 12, 2, 215, "man"] call capSpawn;\n'
                           '}\n', False),
    "town_night": ("Abel", '["town_night", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0, 800] call capBegin\n'
                   'call {\n'
                   '    capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;\n'
                   '    if (not (isNull capFire)) then { capFire inflame true };\n'
                   '}\n', True),
    "sinai_lowsun_fp": ("Sinai", '["sinai_lowsun_fp", 8350, 2950, 1.75, 338.8, -3, [2005, 7, 21, 16, 10], 0, 0, 2000] call capBegin\n'
                        'call {\n'
                        '    [["LoBoHMWV_Mag_Patrol", "Jeep"], 10, -5, -107.8, "veh"] call capSpawn;\n'
                        '    [["LoBo_Zelda", "M113"], 16, 5, 72.2, "veh"] call capSpawn;\n'
                        '    [["LoBoGolaniWB", "SoldierWB"], 7, 1, -112.8, "man"] call capSpawn;\n'
                        '    [["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.8, "veh"] call capSpawn;\n'
                        '}\n', False),
    # Research-only view: Saint Louis (the town_night spot) at golden hour, so
    # sunlit west walls can light the street and the shaded walls.
    "town_golden": ("Abel", '["town_golden", 7152, 8915, 1.75, 345, 3, [1985, 7, 5, 18, 15], 0, 0, 1200] call capBegin\n'
                    'call {\n'
                    '    [["Jeep"], 11, -3, 75, "veh"] call capSpawn;\n'
                    '    [["SoldierWB"], 7, 1, 160, "man"] call capSpawn;\n'
                    '}\n', False),
}

GI_OFF = ["gi=0", "giDebug=0", "bounce=1", "moonlight=1"]
V3 = ["giRadius=1.5", "giRays=6", "giSteps=20", "giAlbedoCap=0.8", "giFallbackShare=0.5"]
VARIANT_SETS = {
    "smoke": [
        ("gi_m1s1", ["giMode=1", "gi=1", "giRadius=1", "giRays=4", "giSteps=16", "giTemporal=1", "giDebug=0"]),
        ("gi_m1s1_dbg", ["giMode=1", "gi=1", "giRadius=1", "giRays=4", "giSteps=16", "giTemporal=1", "giDebug=1"]),
    ],
    "main": [
        ("gi_m1s1", ["giMode=1", "gi=1", "giRadius=1", "giRays=4", "giSteps=16", "giTemporal=1", "giDebug=0"]),
        ("gi_m1s4", ["giMode=1", "gi=4", "giRadius=1.5", "giRays=6", "giSteps=20", "giTemporal=1", "giDebug=0"]),
        ("gi_m1s4_dbg", ["giMode=1", "gi=4", "giRadius=1.5", "giRays=6", "giSteps=20", "giTemporal=1", "giDebug=1"]),
        ("gi_m1s4_notemporal", ["giMode=1", "gi=4", "giRadius=1.5", "giRays=6", "giSteps=20", "giTemporal=0", "giDebug=0"]),
        ("gi_m3s1", ["giMode=3", "gi=1", "giRadius=1", "giRays=6", "giSteps=20", "giTemporal=1", "giDebug=0"]),
        ("gi_m3s1_dbg", ["giMode=3", "gi=1", "giRadius=1", "giRays=6", "giSteps=20", "giTemporal=1", "giDebug=1"]),
        ("gi_m2s3", ["giMode=2", "gi=3", "giRadius=1", "giRays=4", "giSteps=16", "giTemporal=1", "giDebug=0"]),
    ],
    "v3": [
        ("gi_m1s1", ["giMode=1", "gi=1", "giTemporal=1", "giDebug=0"] + V3),
        ("gi_m1s2", ["giMode=1", "gi=2", "giTemporal=1", "giDebug=0"] + V3),
        ("gi_m1s2_dbg", ["giMode=1", "gi=2", "giTemporal=1", "giDebug=1"] + V3),
        ("gi_m1s2_notemporal", ["giMode=1", "gi=2", "giTemporal=0", "giDebug=0"] + V3),
        ("gi_m3s1", ["giMode=3", "gi=1", "giTemporal=1", "giDebug=0"] + V3),
        ("gi_m3s1_dbg", ["giMode=3", "gi=1", "giTemporal=1", "giDebug=1"] + V3),
        ("gi_m2s2", ["giMode=2", "gi=2", "giTemporal=1", "giDebug=0", "giRadius=1", "giRays=4", "giSteps=16"]),
    ],
    # v5 (night): lamp and fire pools as the bounce sources, with the moon
    # turned down to 0.3 so moon bounce does not dominate.
    "night": [
        ("rem_moon03", ["gi=0", "giDebug=0", "moonlight=0.3"]),
        ("gi_moon03_m1s1", ["moonlight=0.3", "giMode=1", "gi=1", "giTemporal=1", "giDebug=0"] + V3),
        ("gi_moon03_m1s2", ["moonlight=0.3", "giMode=1", "gi=2", "giTemporal=1", "giDebug=0"] + V3),
        ("gi_moon03_m1s1_dbg", ["moonlight=0.3", "giMode=1", "gi=1", "giTemporal=1", "giDebug=1"] + V3),
        ("gi_moon1_m1s1", ["moonlight=1", "giMode=1", "gi=1", "giTemporal=1", "giDebug=0"] + V3),
    ],
    # v6 (override_v6): crossing-tolerant hit test; hit fraction, physical and upper-bound modes.
    "v6": [
        ("diag_hits", ["bounce=1", "giMode=7", "gi=1", "giTemporal=1", "giDebug=2"] + V3),
        ("gi_m1s1", ["bounce=1", "giMode=1", "gi=1", "giTemporal=1", "giDebug=0"] + V3),
        ("gi_m1s1_dbg", ["bounce=1", "giMode=1", "gi=1", "giTemporal=1", "giDebug=1"] + V3),
        ("gi_m6s1", ["bounce=1", "giMode=6", "gi=1", "giTemporal=1", "giDebug=0"] + V3),
    ],
    # Diagnostics (override_v5): raw GI texture views.
    "diag": [
        ("diag_hits", ["bounce=1", "giMode=7", "gi=1", "giTemporal=1", "giDebug=2"] + V3),
        ("diag_hitL", ["bounce=1", "giMode=8", "gi=1", "giTemporal=1", "giDebug=2"] + V3),
        ("diag_ratio6", ["bounce=1", "giMode=6", "gi=1", "giTemporal=1", "giDebug=2"] + V3),
        ("diag_share", ["bounce=1", "giMode=6", "gi=1", "giTemporal=1", "giDebug=3"] + V3),
        ("diag_add6", ["bounce=1", "giMode=6", "gi=1", "giTemporal=1", "giDebug=1"] + V3),
    ],
    # v4: the rig's uniform ground bounce off (bounce=0) alone and replaced by the
    # traced result (mode 3), and the art-directed fill (mode 6, needs override_v4).
    "v4": [
        ("rem_b0", ["gi=0", "giDebug=0", "bounce=0"]),
        ("gi_b0m3", ["bounce=0", "giMode=3", "gi=1", "giTemporal=1", "giDebug=0"] + V3),
        ("gi_b0m3_dbg", ["bounce=0", "giMode=3", "gi=1", "giTemporal=1", "giDebug=1"] + V3),
        ("gi_m6s05", ["bounce=1", "giMode=6", "gi=0.5", "giTemporal=1", "giDebug=0"] + V3),
        ("gi_m6s1", ["bounce=1", "giMode=6", "gi=1", "giTemporal=1", "giDebug=0"] + V3),
        ("gi_m6s1_dbg", ["bounce=1", "giMode=6", "gi=1", "giTemporal=1", "giDebug=1"] + V3),
    ],
}

# A short strafe (camera 0.3 m right per frame) with temporal on, then off: the
# accumulation's reprojection and ghosting under motion.
MOTION = [
    ("motion_t", ["bounce=1", "giMode=6", "gi=1", "giTemporal=1", "giDebug=0"] + V3),
    ("motion_n", ["bounce=1", "giMode=6", "gi=1", "giTemporal=0", "giDebug=0"] + V3),
]
# Night motion: the added light alone (debug), temporal on and off.
MOTION_NIGHT = [
    ("motion_dbg_t", ["moonlight=0.3", "giMode=1", "gi=1", "giTemporal=1", "giDebug=1"] + V3),
    ("motion_dbg_n", ["moonlight=0.3", "giMode=1", "gi=1", "giTemporal=0", "giDebug=1"] + V3),
    ("motion_img_t", ["moonlight=0.3", "giMode=1", "gi=1", "giTemporal=1", "giDebug=0"] + V3),
]

HEADER = """// RESEARCH (ssgi): generated by tmp/research/ssgi/scripts/gen_tests.py. Do not commit.
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
capFire = objNull

// "key=value" call capKnob: one tuning knob, answer logged, never returned.
capKnob = {{
    capKnobRes = triSetLighting _this;
    format ["view=%1;event=knob;set=%2;res=%3", capView, _this, capKnobRes] call capLog;
}}

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
capProbe = "HeliHEmpty" createVehicle [{px}, {pz}, 0]
capCam = "camera" camCreate [{px}, {pz}, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call {{ "view=setup;event=setup;world={world};msaa=4;scale=1.5;window=960x640;capture=present;research=ssgi" call capLog }}
"""


def shot(view, label, mode, knobs, fire):
    out = [f'"{mode}" call capMode']
    for k in knobs:
        out.append(f'"{k}" call capKnob')
    out.append("triSimFrames 90")
    out.append("[] call capReadTimers")
    out.append("[] call capPin")
    if fire:
        out.append("call { if (not (isNull capFire)) then { capFire inflame true } }")
    out.append("triSimFrames 3")
    out.append(f'triScreenshot "{view}__{label}"')
    out.append(f'"{view}__{label}" call capRecord')
    return "\n".join(out) + "\n"


def main():
    out_dir = Path(sys.argv[1])
    name = sys.argv[2]
    views = sys.argv[3].split(",")
    variants = VARIANT_SETS[sys.argv[4]]
    world = VIEWS[views[0]][0]
    assert all(VIEWS[v][0] == world for v in views)
    probe = {"Abel": (7300, 10900), "Sinai": (8350, 2950)}[world]
    body = HEADER.format(world=world, px=probe[0], pz=probe[1])
    for v in views:
        _, setup, fire = VIEWS[v]
        body += f"\n// ---- {v}\n" + setup + "triSimFrames 150\n"
        body += shot(v, "classic", "classic", GI_OFF, fire)
        body += shot(v, "remastered", "remastered", GI_OFF, fire)
        for label, knobs in variants:
            body += shot(v, label, "remastered", knobs, fire)
        if len(sys.argv) > 5 and sys.argv[5] == v:
            for label, knobs in (MOTION_NIGHT if v == "town_night" else MOTION):
                body += shot(v, label + "0", "remastered", knobs, fire)
                body += "capOX0 = capOX\ncapOZ0 = capOZ\n"
                for i in range(1, 7):
                    body += "call { capOX = capOX + 0.3 * (cos capH); capOZ = capOZ - 0.3 * (sin capH); [] call capPinView }\n"
                    body += "triSimFrames 1\n"
                    body += f'triScreenshot "{v}__{label}{i}"\n'
                    body += f'"{v}__{label}{i}" call capRecord\n'
                body += "capOX = capOX0\ncapOZ = capOZ0\n"
        body += '"gi=0" call capKnob\n[] call capClear\n'
    body += '\ncall { "view=done;event=done" call capLog }\ntriEndTest\n'
    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / f"{name}.test.sqf").write_text(body, encoding="utf-8", newline="\n")
    if world == "Abel":
        toml = (
            'binary = "PoseidonGame"\n'
            'mission = "tests/rendering/captures/missions/udcap_abel.Abel"\n'
            'tags = ["lighting_capture", "headful", "exclusive", "research"]\n'
            "timeout = 900\nassert_timeout = 180\n"
            'extra_args = ["--no-strict", "--display-mode", "windowed", "--width", "960", "--height", "640"]\n'
        )
    else:
        toml = (
            'binary = "PoseidonGame"\n'
            'mission = "tests/rendering/captures/missions/udcap_sinai.Sinai"\n'
            'tags = ["lighting_capture", "lobo", "headful", "exclusive", "research"]\n'
            "timeout = 900\nassert_timeout = 240\n"
            'extra_args = ["--no-strict", "--mods-dir", ".", "--mod", "@LoBo", "--display-mode", "windowed", "--width", "960", "--height", "640"]\n'
        )
    (out_dir / f"{name}.test.toml").write_text(toml, encoding="utf-8", newline="\n")
    print("wrote", out_dir / f"{name}.test.sqf", len(views), "views", len(variants) + 2, "shots each")


if __name__ == "__main__":
    main()
