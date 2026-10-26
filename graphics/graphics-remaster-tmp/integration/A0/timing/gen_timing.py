"""Generate the A0 timing-baseline tests (plan section 5 A0, gate item 7).

    python gen_timing.py [--rounds 3] [--out <dir>]

Writes abel_timing, eden_timing and sinai_timing (.test.sqf + .test.toml) next
to this file. Every test runs at 1920x1080 windowed, render scale 1.0, 4x MSAA,
the presented frame, and visits its hero views in `rounds` interleaved rounds
(view order A B C .. A B C ..). Per view and round it shoots Classic then
Remastered with the repo rig's block (capMode, 90 frames, capReadTimers, capPin,
3 frames, triScreenshot, capRecord), so make_pairs.py reads the output; the
round is part of the view label (<view>_r<k>).

Views: the start view of the first row of each feature in plan section 7.3
(hero stills of gate item 5), plus the gate-off references noon_fields and
sea_noon (noon, dry) and the Eden and Sinai rows. Deviations from 7.3 are in
the `note` field of each view below; the list is repeated in the timing
bullet of INTEGRATION-PLAN.md section 5 A0. The notes are documentation only:
they do not reach the generated tests.
"""
import argparse
from pathlib import Path

HERE = Path(__file__).resolve().parent

# name, x, z, eye, heading, pitch, date, overcast, fog, vd, rain, eyeAbs,
# props (SQF statements inside one call {}), refresh (SQF before each shot),
# cleanup (SQF after capClear), feature
FIRE_REFRESH = "call { if (not (isNull capFire)) then { capFire inflame true } }"
ABEL = [
    dict(name="sunrise_ridge", x=4400, z=3300, eye=1.75, h=60, p=2, date=[1985, 7, 5, 5, 8], vd=4000,
         feature="perf", f10=True),
    dict(name="forest_edge_golden", x=4674, z=10156, eye=1.75, h=75, p=3, date=[1985, 7, 5, 18, 15], vd=1500,
         props=['[["SoldierWB"], 12, 2, 215, "man"] call capSpawn;',
                '[["Jeep"], 25, 3, 90, "veh"] call capSpawn;'],
         feature="temporal, texsharp",
         note="Jeep parked at 25 m (the video teleports it across); also stands in for texsharp's first row "
              "(7.3: p 0, no props)"),
    dict(name="town_night", x=7152, z=8915, eye=1.75, h=345, p=3, date=[1985, 2, 27, 22, 0], vd=800,
         props=['capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;',
                'if (not (isNull capFire)) then { capFire inflame true };'],
         refresh=FIRE_REFRESH, feature="light list",
         note="capture-set version: auto street lamps and the Fire at 9 m, no driven Jeeps (7.3 has two)"),
    dict(name="coastline_sunset", x=1550, z=5150, eye=3, h=278, p=-4, date=[1985, 7, 5, 18, 35], vd=4000,
         feature="sky table, water",
         note="capture-set heading 278 (7.3 water row; the sky row says 250); eye 3 m above the beach probe "
              "(the sky row's convention; the water row says 3 m ASL)"),
    dict(name="woods_pine2_evening", x=6940, z=9964, eye=1.75, h=240, p=-2, date=[1985, 7, 5, 17, 30], vd=1500,
         props=['[["Jeep"], 14, 2, 70, "veh"] call capSpawn;',
                '[["SoldierWB"], 9, -2, 150, "man"] call capSpawn;'],
         feature="shadows",
         note="props from shadows/gen_r8.py (Jeep at 14 m, SoldierWB at 9 m; 7.3 says a Jeep at 15 m); "
              "pitch -2 per 7.3 (the research shot -5)"),
    dict(name="lowsun_fp", x=7300, z=10900, eye=1.75, h=152.5, p=-3, date=[1985, 3, 1, 8, 0], vd=1500,
         props=['[["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;',
                '[["M113"], 16, 5, 72.5, "veh"] call capSpawn;',
                '[["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;',
                '[["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;'],
         feature="shade", note="capture-set camera instead of the orbit start"),
    dict(name="road_flat_drive_moonless", x=6895.2, z=8836.6, eye=1.5, h=69.6, p=-2, date=[1985, 3, 17, 23, 0],
         vd=800,
         props=['capJeep = [["Jeep"], -3.4, 0, 0, "veh"] call capSpawn;',
                'capDrv = objNull;',
                'if (not (isNull capJeep)) then { capJeep lock false; "SoldierWB" createUnit [[capOX, capOZ, 0], capGrpW, "capDrv = this", 0.6, "PRIVATE"]; capDrv setCaptive true; capDrv allowDamage false; capDrv moveInDriver capJeep; capDrv disableAI "MOVE"; capDrv setBehaviour "SAFE" };',
                'format ["view=%1;event=jeep;jeep=%2;driver=%3", capView, capJeep, driver capJeep] call capLog;'],
         cleanup='call { if (not (isNull capDrv)) then { deleteVehicle capDrv; capDrv = objNull } }',
         feature="night",
         note="night track's driver's-eye setup: the Jeep stands 3.4 m behind the eye with a SAFE AI driver, "
              "not driven (7.3 teleports it 0.4 m per frame)"),
    dict(name="ffh", x=7600, z=10850, eye=5, h=0, p=-12, date=[1985, 7, 12, 23, 0], vd=1000,
         props=['{ [["SoldierWB"], _x select 0, _x select 1, 70, "man"] call capSpawn } forEach [[13, -7], [15.5, -4.5], [12.5, -2.5]];',
                '{ [["SoldierEB"], _x select 0, _x select 1, -110, "man", capGrpE] call capSpawn } forEach [[16, 3.5], [18, 6], [15, 8.5]];',
                'capBurnV = [["Ural", "Truck5t"], 36, 13, 30, "veh"] call capSpawn;',
                'if (not (isNull capBurnV)) then { capBurnV allowDamage true; capBurnV setDammage 1; capBurnV inflame true };',
                'capFire = [["Fire"], 21, -12, 0, "veh"] call capSpawn;',
                'if (not (isNull capFire)) then { capFire inflame true };'],
         refresh='call { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }',
         feature="combat", note="static stand-in of capFFStageN: posed men, burning Ural and Fire, no volleys or shells"),
    dict(name="bay_offshore", x=9240, z=6950, eye=3, eyeAbs=True, h=272, p=-3, date=[1985, 7, 5, 6, 30], vd=3000,
         props=['[["BoatW", "BoatE", "Boat"], 45, 10, -202, "veh"] call capSpawn;'],
         feature="ssr", note="eye 3 m ASL (timing_lib capEyeAbs); boat heading 70"),
    dict(name="town_night_rain", x=7152, z=8915, eye=1.75, h=345, p=3, date=[1985, 2, 27, 22, 0], vd=800,
         overcast=0.9, rain=1,
         props=['capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;',
                'if (not (isNull capFire)) then { capFire inflame true };'],
         refresh=FIRE_REFRESH, feature="materials"),
    dict(name="pinewood_morn", x=6830, z=9905, eye=1.75, h=72, p=6, date=[1985, 7, 5, 6, 0], vd=1500, fog=0.1,
         feature="air (beams)", f10=True),
    dict(name="valley_0515", x=6100, z=5000, eye=1.75, h=40, p=-6, date=[1985, 7, 5, 5, 15], vd=4000,
         feature="air (haze, valley mist)"),
    dict(name="hill_1730", x=5500, z=6250, eye=15, h=185, p=-14, date=[1985, 9, 20, 17, 30], vd=2000,
         feature="hfrt"),
    dict(name="coast_oc_up", x=1550, z=5150, eye=3, h=260, p=14, date=[1985, 7, 5, 18, 35], vd=4000, overcast=0.5,
         feature="clouds"),
    dict(name="sea_noon", x=1550, z=5150, eye=12, h=278, p=-12, date=[1985, 7, 5, 12, 30], vd=4000,
         feature="water (noon); gate-off reference for low-sun features"),
    dict(name="noon_fields", x=6250, z=10800, eye=1.75, h=150, p=-2, date=[1985, 7, 5, 12, 30], vd=2500,
         props=['[["Jeep"], 13, 3, 60, "veh"] call capSpawn;',
                '[["SoldierWB"], 8, -2, 160, "man"] call capSpawn;'],
         feature="gate-off reference (noon, dry)"),
]
EDEN = [
    dict(name="overcast_broken", x=9200, z=3700, eye=1.75, h=288, p=-4, date=[1985, 7, 5, 14, 30], vd=3500,
         overcast=0.5, props=['[["Jeep"], 12, -5, 70, "veh"] call capSpawn;'], feature="texsharp (meadow)"),
    dict(name="broken_high", x=9200, z=3700, eye=250, h=288, p=-12, date=[1985, 7, 5, 14, 30], vd=5000,
         overcast=0.5, feature="clouds (shadows clip)",
         note="sky drift not stepped (still; 7.3 steps it 0 to 15 deg over the clip)"),
]
SINAI = [
    dict(name="sinai_lowsun_fp", x=8350, z=2950, eye=1.75, h=338.8, p=-3, date=[2005, 7, 21, 16, 10], vd=2000,
         props=['[["LoBoHMWV_Mag_Patrol", "Jeep"], 10, -5, -107.8, "veh"] call capSpawn;',
                '[["LoBo_Zelda", "M113"], 16, 5, 72.2, "veh"] call capSpawn;',
                '[["LoBoGolaniWB", "SoldierWB"], 7, 1, -112.8, "man"] call capSpawn;',
                '[["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.8, "veh"] call capSpawn;'],
         feature="temporal, shade (Sinai clips)"),
    dict(name="sinai_desert_noon", x=7300, z=10300, eye=1.75, h=210, p=1, date=[2005, 7, 21, 12, 15], vd=4000,
         props=['[["LoBoHMWV_Mag_Patrol", "Jeep"], 14, 4, 60, "veh"] call capSpawn;',
                '[["LoBoGolaniWB", "SoldierWB"], 8, -2, 160, "man"] call capSpawn;'],
         feature="gate-off reference (Sinai noon)"),
]

TESTS = {
    "abel_timing": dict(world="Abel", mission="tests/rendering/captures/missions/udcap_abel.Abel", views=ABEL,
                        tags='["lighting_capture", "headful", "exclusive"]', mods=[]),
    "eden_timing": dict(world="Eden", mission="tests/rendering/captures/missions/udcap_eden.Eden", views=EDEN,
                        tags='["lighting_capture", "headful", "exclusive"]', mods=[]),
    "sinai_timing": dict(world="Sinai", mission="tests/rendering/captures/missions/udcap_sinai.Sinai", views=SINAI,
                         tags='["lighting_capture", "lobo", "headful", "exclusive"]',
                         mods=['"--mods-dir"', '"."', '"--mod"', '"@LoBo"']),
}


def num(v):
    return f"{v:g}" if isinstance(v, float) else str(v)


def header(world, v0):
    return f"""// A0 timing baseline on {world}: generated by gen_timing.py, do not edit.
// 1920x1080 windowed, render scale 1.0, 4x MSAA, presented frame.
#include "capture_lib.sqf"
#include "timing_lib.sqf"

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
capBurnV = objNull
capJeep = objNull
capDrv = objNull

call {{ format ["view=setup;event=window_boot;size=%1", triGetWindowSize] call capLog }}
triCaptureAtPresent true
triSetMsaa 4
triSetRenderScale 1.0
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false

createCenter west
createCenter east
capGrpW = createGroup west
capGrpE = createGroup east
capProbe = "HeliHEmpty" createVehicle [{num(v0['x'])}, {num(v0['z'])}, 0]
capCam = "camera" camCreate [{num(v0['x'])}, {num(v0['z'])}, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call {{ format ["view=setup;event=setup;world={world};msaa=4;scale=1.0;window=%1;capture=present;lobo=%2", triGetWindowSize, "LoBo_camnet" call capHas] call capLog }}
"""


def view_block(v, rnd, first_round):
    label = f"{v['name']}_r{rnd}"
    d = v["date"]
    out = [f"\n// ---- {v['name']} round {rnd} ({v['feature']})"]
    out.append(f"call {{ capRain = {num(v.get('rain', 0))}; capEyeAbs = {'true' if v.get('eyeAbs') else 'false'} }}")
    out.append(f'["{label}", {num(v["x"])}, {num(v["z"])}, {num(v["eye"])}, {num(v["h"])}, {num(v["p"])}, '
               f'[{", ".join(str(x) for x in d)}], {num(v.get("overcast", 0))}, {num(v.get("fog", 0))}, '
               f'{num(v["vd"])}] call capBegin')
    if v.get("props"):
        out.append("call {")
        out += ["    " + s for s in v["props"]]
        out.append("}")
    out.append("triSimFrames 150")
    for mode in ("classic", "remastered"):
        out.append(f'"{mode}" call capMode')
        out.append("triSimFrames 90")
        out.append("[] call capReadTimers")
        out.append("[] call capPin")
        if v.get("refresh"):
            out.append(v["refresh"])
        out.append("triSimFrames 3")
        out.append(f'triScreenshot "{label}__{mode}"')
        out.append(f'"{label}__{mode}" call capRecord')
    if first_round and v.get("f10"):
        # F10 in game: a view with trees and no spawned unit. Before F10 the
        # static casters were off at boot and the shadow map went inactive
        # here, so the dump answered FAIL. Wrapped in a call: a top-level FAIL
        # would abort the test.
        out.append(f'call {{ capDump = triShadowSceneDump "f10_{v["name"]}"; '
                   f'format ["view=%1;event=f10;dump=%2", capView, capDump] call capLog }}')
    out.append("[] call capClear")
    if v.get("cleanup"):
        out.append(v["cleanup"])
    return out


def toml(name, t):
    args = ['"--no-strict"'] + t["mods"] + ['"--display-mode"', '"windowed"', '"--width"', '"1920"', '"--height"',
                                           '"1080"']
    return f"""# A0 timing baseline ({t['world']}): generated by gen_timing.py. A photo and
# timing generator, not a regression test; run it with run-captures.ps1.
binary = "PoseidonGame"
mission = "{t['mission']}"
tags = {t['tags']}
timeout = 5400
assert_timeout = 240
extra_args = [{", ".join(args)}]
"""


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--rounds", type=int, default=3)
    ap.add_argument("--out", default=str(HERE))
    a = ap.parse_args()
    out = Path(a.out)
    for name, t in TESTS.items():
        lines = [header(t["world"], t["views"][0])]
        for rnd in range(1, a.rounds + 1):
            for v in t["views"]:
                lines += view_block(v, rnd, rnd == 1)
        lines.append('\ncall { "view=done;event=done" call capLog }')
        lines.append("triEndTest")
        (out / f"{name}.test.sqf").write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
        (out / f"{name}.test.toml").write_text(toml(name, t), encoding="utf-8", newline="\n")
        print(f"{name}: {len(t['views'])} views x {a.rounds} rounds")


if __name__ == "__main__":
    main()
