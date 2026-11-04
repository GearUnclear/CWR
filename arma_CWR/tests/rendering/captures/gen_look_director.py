"""Generate the look-director capture tests: the combined photoreal overhaul
(the look lanes light, fog and surface merged) against the Original, over the
hero views of the three lanes plus rain and a long noon vista.

    python gen_look_director.py --modes "classic=classic;overhaul=remastered" [--tag NAME]
                                [--only view,view] [--frames N]

Writes look_<tag>_<world>.test.sqf + .test.toml next to this file (one test per
world that has a selected view). Each mode is `label=setting` (a triSetLighting
setting, knobs allowed: "remastered,camEv=0.5"); the shot of view V in mode L is
saved as <V>__<L>.png. 1920x1080 windowed, render scale 1.0, 4x MSAA, the
presented frame (capgen.header). The full Classic | Remastered set of
run-captures.ps1 (abel_lighting, eden_lighting, sinai_lobo_lighting) covers the
firefight, the tank interior and the gunner's optic.
"""
import argparse
from pathlib import Path

import capgen
import views

TABLES = {"Abel": views.TIMING_ABEL, "Eden": views.TIMING_EDEN, "Sinai": views.TIMING_SINAI}

# A Jeep 22 m up the street with its headlights toward the eye (look-fog).
FOG_JEEP = ['capJeep = [["Jeep"], 22, 1.5, 180, "veh"] call capSpawn;',
            'capDrv = objNull;',
            'if (not (isNull capJeep)) then { capJeep lock false; "SoldierWB" createUnit [[capOX, capOZ, 0], capGrpW, '
            '"capDrv = this", 0.6, "PRIVATE"]; capDrv setCaptive true; capDrv allowDamage false; capDrv moveInDriver '
            'capJeep; capDrv disableAI "MOVE"; capDrv setBehaviour "SAFE" };',
            'format ["view=%1;event=jeep;jeep=%2;driver=%3", capView, capJeep, driver capJeep] call capLog;']


def t(src, **kw):
    for table in TABLES.values():
        for v in table:
            if v["name"] == src:
                d = dict(v)
                d.pop("note", None)
                d.pop("f10", None)
                d.pop("feature", None)
                d.update(kw)
                return d
    raise KeyError(src)


VIEWS = {
    "Abel": [
        t("noon_fields"),
        t("lowsun_fp"),
        t("forest_edge_golden"),
        t("sunrise_ridge"),
        t("coastline_sunset"),
        t("woods_pine2_evening"),
        # Look lane "light": the blue hour over the west coast.
        dict(name="blue_hour_coast", x=1550, z=5150, eye=3, h=295, p=2, date=[1985, 7, 5, 19, 55], vd=4000),
        # Look lane "fog": a misty dawn valley, sun rays in a pine wood, a lit
        # street in fog at night with a Jeep's headlights toward the eye.
        t("valley_0515", name="dawn_valley", date=[1985, 7, 1, 5, 45]),
        t("pinewood_morn", name="pine_rays"),
        t("town_night"),
        t("town_night", name="town_fog_night", fog=0.35, props=views.TOWN_FIRE + FOG_JEEP,
          cleanup=views.NIGHT_JEEP_CLEANUP),
        t("town_night_rain"),
        # Look lane "surface": a parked Jeep close up at noon, the town by day,
        # a wood at mid morning.
        dict(name="jeep_close_noon", x=6250, z=10800, eye=1.75, h=150, p=-12, date=[1985, 7, 5, 12, 30], vd=1500,
             props=['[["Jeep"], 6.5, 0.5, 70, "veh"] call capSpawn;',
                    '[["SoldierWB"], 5, -2.2, 200, "man"] call capSpawn;',
                    '[["M113"], 14, 4, 30, "veh"] call capSpawn;']),
        dict(name="town_noon", x=7152, z=8915, eye=1.75, h=345, p=3, date=[1985, 7, 5, 11, 0], vd=800,
             props=['[["Jeep"], 14, -3, 30, "veh"] call capSpawn;']),
        dict(name="forest_morning", x=4674, z=10156, eye=1.75, h=75, p=0, date=[1985, 7, 5, 9, 30], vd=1500,
             props=['[["SoldierWB"], 10, 1, 200, "man"] call capSpawn;']),
    ],
    "Eden": [
        t("overcast_broken"),
        t("overcast_broken", name="overcast_heavy", overcast=0.9),
        # Daytime rain under a full deck (the materials and the wet air).
        t("overcast_broken", name="rain_day", overcast=1.0, rain=0.8),
        # A clear-noon long vista, terrain from 1 to 5 km (look-fog round 2).
        t("broken_high", name="noon_vista", eye=150, p=-5, date=[1985, 7, 5, 12, 30], vd=5000, overcast=0.05),
    ],
    "Sinai": [
        t("sinai_lowsun_fp"),
        t("sinai_desert_noon"),
    ],
}
SIM_T0, SIM_DT = 2000, 600


def shot(v, setting, shot_label, frames):
    out = [f'"{setting}" call capMode',
           f"triSimFrames {frames}",
           "[] call capReadTimers",
           "[] call capPin"]
    if v.get("refresh"):
        out.append(v["refresh"])
    out += ["triSimFrames 3",
            f'triScreenshot "{shot_label}"',
            f'"{shot_label}" call capRecord']
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--modes", default="classic=classic;overhaul=remastered", help="label=setting;label=setting;...")
    ap.add_argument("--tag", default="director")
    ap.add_argument("--only", default="")
    ap.add_argument("--frames", type=int, default=90)
    ap.add_argument("--out", default=str(capgen.HERE))
    a = ap.parse_args()
    modes = []
    for item in a.modes.split(";"):
        item = item.strip()
        if item:
            label, setting = item.split("=", 1)
            modes.append((label.strip(), setting.strip()))
    want = {s.strip() for s in a.only.split(",") if s.strip()}
    written = []
    index = 0
    for world, vs in VIEWS.items():
        vs = [v for v in vs if not want or v["name"] in want]
        if not vs:
            continue
        title = f"look-director views on {world}: gen_look_director.py, modes {', '.join(m[0] for m in modes)}."
        lines = [capgen.header(world, vs[0], title)]
        for v in vs:
            lines.append(f"\n// ---- {v['name']}")
            lines += capgen.begin(v, v["name"], False, sim_time=SIM_T0 + SIM_DT * index)
            index += 1
            lines.append("triSimFrames 150")
            for label, setting in modes:
                lines += shot(v, setting, f"{v['name']}__{label}", a.frames)
            lines += capgen.end(v, False)
        lines.append('\ncall { "view=done;event=done" call capLog }')
        lines.append("triEndTest")
        name = f"look_{a.tag}_{world.lower()}"
        out = Path(a.out)
        capgen.write(out / f"{name}.test.sqf", "\n".join(lines) + "\n")
        capgen.write(out / f"{name}.test.toml", capgen.toml(world, title, views.WORLDS, timeout=5400))
        written.append(f"tests/rendering/captures/{name}.test.sqf")
    print(",".join(written))


if __name__ == "__main__":
    main()
