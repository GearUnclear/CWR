"""v7 research tests for the ssgi track (resume run, 2026-10-17 evening).

Best-case scenes for screen-space bounce: local lights in a dark scene, where
the bounce source (a headlight pool on a hull side, a campfire pool) is far
brighter than the sky fill. Plus the daytime acceptance views with the object
sky share forced to 1 (what a production G-buffer share would give shaded
object faces), to bound the object bounce the 0.5 fallback was hiding.

python gen_tests_v7.py <out dir>

Needs override_v7 (chroma albedo when giFallbackShare >= 1.5).
"""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import gen_tests as g  # noqa: E402

LAMPS = "[28419, 66997, 90863, 66998, 67026, 28424]"  # Saint Louis street lamps (object ids)

VIEWS = {
    # town_night camera, street lamps switched OFF (their 50 m flat discs light
    # the whole town), campfire at 9 m, a soldier standing beside it.
    "town_dark_fire": (
        '["town_dark_fire", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0, 800] call capBegin\n'
        "call {\n"
        f'    {{ (object _x) switchLight "OFF" }} forEach {LAMPS};\n'
        '    capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;\n'
        "    if (not (isNull capFire)) then { capFire inflame true };\n"
        '    [["SoldierWB"], 10.5, 1.2, 160, "man"] call capSpawn;\n'
        '    "view=town_dark_fire;event=lamps_off" call capLog;\n'
        "}\n",
        True,
    ),
    # Moonless night on open flat ground (the lowsun_fp spot): a crewed Jeep with
    # its headlights on, aimed at the side of an M113 5-6 m away. The lit hull
    # side is the bounce source; the Jeep's own front (3/4 view from the camera)
    # and a soldier beside the hull are the receivers.
    "headlights_bounce": (
        '["headlights_bounce", 7300, 10900, 1.75, 152.5, -4, [1985, 3, 17, 23, 0], 0, 0, 800] call capBegin\n'
        "call {\n"
        '    [["M113"], 7, -3.5, 0, "veh"] call capSpawn;\n'
        '    [["SoldierWB"], 5.5, -0.5, 200, "man"] call capSpawn;\n'
        '    capJeep = [["Jeep"], 9.5, 4, 247.4, "veh"] call capSpawn;\n'
        "    if (not (isNull capJeep)) then {\n"
        "        capJeep lock false;\n"
        '        "SoldierWB" createUnit [[7250, 10950, 0], capGrpW, "capDrv = this", 0.6, "PRIVATE"];\n'
        "        capDrv setCaptive true; capDrv allowDamage false;\n"
        "        capDrv moveInDriver capJeep;\n"
        '        capDrv disableAI "MOVE";\n'
        '        capDrv setBehaviour "SAFE";\n'
        "    };\n"
        '    format ["view=headlights_bounce;event=jeep;jeep=%1;driver=%2", capJeep, driver capJeep] call capLog;\n'
        "}\n",
        False,
    ),
    "lowsun_fp": (g.VIEWS["lowsun_fp"][1], False),
    "noon_fields": (g.VIEWS["noon_fields"][1], False),
}

BASE = ["giRadius=1.5", "giRays=6", "giSteps=20", "giAlbedoCap=0.8", "giTemporal=1", "bounce=1"]
NIGHT = [
    ("gi_m1s1_chroma", ["giMode=1", "gi=1", "giFallbackShare=2", "giDebug=0"] + BASE),
    ("gi_m1s2_chroma", ["giMode=1", "gi=2", "giFallbackShare=2", "giDebug=0"] + BASE),
    ("gi_m6s1_chroma", ["giMode=6", "gi=1", "giFallbackShare=2", "giDebug=0"] + BASE),
    ("gi_m1s1_share1", ["giMode=1", "gi=1", "giFallbackShare=1", "giDebug=0"] + BASE),
    ("gi_m1s1_chroma_dbg", ["giMode=1", "gi=1", "giFallbackShare=2", "giDebug=1"] + BASE),
]
DAY = [
    ("gi_m1s1_fb1", ["giMode=1", "gi=1", "giFallbackShare=1", "giDebug=0"] + BASE),
    ("gi_m6s1_fb1", ["giMode=6", "gi=1", "giFallbackShare=1", "giDebug=0"] + BASE),
    ("gi_m6s1_fb1_dbg", ["giMode=6", "gi=1", "giFallbackShare=1", "giDebug=1"] + BASE),
    ("gi_m1s1_chroma", ["giMode=1", "gi=1", "giFallbackShare=2", "giDebug=0"] + BASE),
]
PLAN = [("headlights_bounce", NIGHT), ("town_dark_fire", NIGHT), ("lowsun_fp", DAY), ("noon_fields", DAY)]


def main():
    out_dir = Path(sys.argv[1])
    body = g.HEADER.format(world="Abel", px=7300, pz=10900).replace(
        "gen_tests.py", "gen_tests_v7.py"
    )
    body = body.replace("capFire = objNull\n", "capFire = objNull\ncapJeep = objNull\ncapDrv = objNull\n")
    for v, variants in PLAN:
        setup, fire = VIEWS[v]
        body += f"\n// ---- {v}\n" + setup + "triSimFrames 150\n"
        body += g.shot(v, "classic", "classic", g.GI_OFF, fire)
        body += g.shot(v, "remastered", "remastered", g.GI_OFF, fire)
        for label, knobs in variants:
            body += g.shot(v, label, "remastered", knobs, fire)
        body += '"gi=0" call capKnob\n"giFallbackShare=0.5" call capKnob\n'
        body += "call { if (not (isNull capDrv)) then { deleteVehicle capDrv; capDrv = objNull } }\n"
        body += "[] call capClear\n"
    body += '\ncall { "view=done;event=done" call capLog }\ntriEndTest\n'
    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "abel_ssgi_v7.test.sqf").write_text(body, encoding="utf-8", newline="\n")
    toml = (
        'binary = "PoseidonGame"\n'
        'mission = "tests/rendering/captures/missions/udcap_abel.Abel"\n'
        'tags = ["lighting_capture", "headful", "exclusive", "research"]\n'
        "timeout = 900\nassert_timeout = 180\n"
        'extra_args = ["--no-strict", "--display-mode", "windowed", "--width", "960", "--height", "640"]\n'
    )
    (out_dir / "abel_ssgi_v7.test.toml").write_text(toml, encoding="utf-8", newline="\n")
    print("wrote", out_dir / "abel_ssgi_v7.test.sqf")


if __name__ == "__main__":
    main()
