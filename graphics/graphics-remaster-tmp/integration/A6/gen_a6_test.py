"""Writes the A6 light-list capture test (a6_lights.test.sqf / .toml) into the
captures folder of a worktree, using that folder's capgen and views.

    python gen_a6_test.py <captures dir>

Views (all Abel, 1920x1080, scale 1.0, 4x MSAA, presented frame):
  lighttiles_town_night  the features_ab hero: auto street lamps and the Fire at 9 m
  lighttiles_town_lamps  the same eye without the fire (repeatable), plus two
                         parked Jeeps with SAFE AI drivers and their headlights on:
                         one facing the camera at 20 m, one across the street at 34 m;
                         a third shot with debug=6 (lights per tile) on the A6 build
  lighttiles_town_moonless  the lamps view on a moonless night (1985-03-17 23:00,
                         the night track's date), where the local lights carry the frame
  lighttiles_sunrise     sunrise_ridge (day: the gate holds the list off), a view
                         that repeats to the pixel between boots
Each view: <view>__off (remastered,lightTiles=0) then <view>__on (remastered).
"""
import sys
from pathlib import Path

cap = Path(sys.argv[1]).resolve()
sys.path.insert(0, str(cap))
import capgen  # noqa: E402
import views  # noqa: E402

JEEPS = [
    'capJeep = [["Jeep"], 20, 2.5, 180, "veh"] call capSpawn;',
    'capDrv = objNull;',
    'if (not (isNull capJeep)) then { capJeep lock false; "SoldierWB" createUnit [[capOX, capOZ, 0], capGrpW, '
    '"capDrv = this", 0.6, "PRIVATE"]; capDrv setCaptive true; capDrv allowDamage false; capDrv moveInDriver '
    'capJeep; capDrv disableAI "MOVE"; capDrv setBehaviour "SAFE" };',
    'capJeep2 = [["Jeep"], 34, -4, 90, "veh"] call capSpawn;',
    'capDrv2 = objNull;',
    'if (not (isNull capJeep2)) then { capJeep2 lock false; "SoldierWB" createUnit [[capOX, capOZ, 0], capGrpW, '
    '"capDrv2 = this", 0.6, "PRIVATE"]; capDrv2 setCaptive true; capDrv2 allowDamage false; capDrv2 moveInDriver '
    'capJeep2; capDrv2 disableAI "MOVE"; capDrv2 setBehaviour "SAFE" };',
    'format ["view=%1;event=jeeps;jeep=%2;driver=%3;jeep2=%4;driver2=%5", capView, capJeep, driver capJeep, '
    'capJeep2, driver capJeep2] call capLog;',
]
JEEPS_CLEANUP = ('call { if (not (isNull capDrv)) then { deleteVehicle capDrv; capDrv = objNull }; '
                 'if (not (isNull capDrv2)) then { deleteVehicle capDrv2; capDrv2 = objNull } }')

town = dict(views._timing("town_night"))
town.pop("note", None)
lamps = dict(town)
lamps.pop("refresh", None)
lamps.update(props=JEEPS, cleanup=JEEPS_CLEANUP)
sunrise = dict(views._timing("sunrise_ridge"))
sunrise.pop("note", None)
sunrise.pop("f10", None)

moonless = dict(lamps)
moonless.update(date=[1985, 3, 17, 23, 0])

VIEWS_ALL = [("lighttiles_town_night", town, 3200, False),
         ("lighttiles_town_lamps", lamps, 3800, True),
         ("lighttiles_town_moonless", moonless, 4400, True),
         ("lighttiles_sunrise", sunrise, 5000, False)]

import os
if os.environ.get("A6_PROBE") == "cpu":
    VIEWS = [("cpu_lamps", lamps, 3800, False)]
    SHOTS = [(s, f"{l}{r}") for r in "ab" for s, l in (("remastered,lightTiles=0", "t0"), ("remastered", "t1"),
                                                        ("remastered,lightTiles=0,drawGrouping=0", "t0ng"),
                                                        ("remastered,drawGrouping=0", "t1ng"))]
elif os.environ.get("A6_PROBE") == "timing":
    VIEWS = [("timing_lamps", lamps, 3800, False)]
    SHOTS = [(s, f"{l}{r}") for r in "abc" for s, l in (("remastered,lightTiles=0", "t0"), ("remastered", "t1"))]
elif os.environ.get("A6_PROBE"):
    VIEWS = [("probe_lamps", lamps, 3800, False)]
    SHOTS = [("remastered,lightTiles=0", "t0"), ("remastered", "t1"), ("remastered,lightTiles=0,localLights=0", "t0nl"),
             ("remastered,localLights=0", "t1nl"), ("remastered,drawGrouping=0", "t1ng"),
             ("remastered,lightTiles=0,drawGrouping=0", "t0ng")]
else:
    VIEWS = VIEWS_ALL
    SHOTS = None
lines = [capgen.header("Abel", town, "A6 light list: off (lightTiles=0) then on, four views.")]
lines.append("capJeep2 = objNull")
lines.append("capDrv2 = objNull")
for name, v, simt, tiles in VIEWS:
    lines.append(f"\n// ---- {name}")
    lines += capgen.begin(v, name, False, sim_time=simt)
    lines.append("triSimFrames 150")
    if SHOTS:
        for setting, lab in SHOTS:
            lines += capgen.shot(v, name, setting, f"{name}__{lab}")
    else:
        lines += capgen.shot(v, name, "remastered,lightTiles=0", f"{name}__off")
        lines += capgen.shot(v, name, "remastered", f"{name}__on")
    if tiles:
        lines += capgen.shot(v, name, "remastered,debug=6", f"{name}__tiles")
    lines += capgen.end(v, False)
lines.append('\ncall { "view=done;event=done" call capLog }')
lines.append("triEndTest")
stem = {"timing": "a6_timing", "cpu": "a6_cpu"}.get(os.environ.get("A6_PROBE", ""), "a6_probe") if os.environ.get("A6_PROBE") else "a6_lights"
capgen.write(cap / f"{stem}.test.sqf", "\n".join(lines) + "\n")
capgen.write(cap / f"{stem}.test.toml",
             capgen.toml("Abel", "A6 light list: off then on, four views.", views.WORLDS, timeout=2400))
print("wrote", cap / "a6_lights.test.sqf")
