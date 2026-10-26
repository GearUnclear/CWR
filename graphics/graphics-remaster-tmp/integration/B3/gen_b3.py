"""B3 lane capture tests (light gate items 1, 5, 6 and 8 for the night step).

    python gen_b3.py

Writes, next to a copy of capture_lib.sqf, tests/b3lane.test.sqf: one Abel
game, each view shot off ("remastered,nightLights=0,moonShadows=0,stars=0")
then on ("remastered"):

    night_road_flat_drive_moonless  the features_ab night hero block, unchanged
                                    (sim clock 5600, cloud pin): moonless 23:00,
                                    driver's eye, the Jeep 3.4 m behind the eye
    b3_road_flat_head               full moon at 36 deg (1985-03-01 20:00), third
                                    person 9 m behind the parked Jeep: moon
                                    shadows, the beam from behind, the Jeep's rear
    b3_town_night                   full moon at 69 deg, street lamps and the Fire
    b3_town_lamps                   moonless, street lamps only
    b3_moon_sky                     looking at the moon (pitch 26): stars, crowns
    b3_jeep_side                    moonless, a Jeep with headlights crossing the
                                    view at 8 m: its own body, the beam on the verge
    b3_day_town                     noon in the town: every night gate off
    b3_nv_town                      night vision in the town: every gate off

The hero view also gets each switch alone in the "each" variant (generated with
--each). On the base build the three keys are registered but inert, so off and
on draw the same frame; that build's shots are the BEFORE images.
"""
import argparse
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = Path(r"D:\Arma_CWA_worktrees\gr-B3\arma_CWR\tests\rendering\captures")
sys.path.insert(0, str(CAPTURES))
import capgen  # noqa: E402
import views  # noqa: E402

OFF = "remastered,nightLights=0,moonShadows=0,stars=0"
ON = "remastered"

JEEP_AHEAD = ['capJeep = [["Jeep"], 9, 0, 0, "veh"] call capSpawn;',
              'capDrv = objNull;',
              'if (not (isNull capJeep)) then { capJeep lock false; "SoldierWB" createUnit [[capOX, capOZ, 0], '
              'capGrpW, "capDrv = this", 0.6, "PRIVATE"]; capDrv setCaptive true; capDrv allowDamage false; '
              'capDrv moveInDriver capJeep; capDrv disableAI "MOVE"; capDrv setBehaviour "SAFE" };',
              'format ["view=%1;event=jeep;jeep=%2;driver=%3", capView, capJeep, driver capJeep] call capLog;']
JEEP_SIDE = ['capJeep = [["Jeep"], 8, 2.5, -75, "veh"] call capSpawn;',
             'capDrv = objNull;',
             'if (not (isNull capJeep)) then { capJeep lock false; "SoldierWB" createUnit [[capOX, capOZ, 0], '
             'capGrpW, "capDrv = this", 0.6, "PRIVATE"]; capDrv setCaptive true; capDrv allowDamage false; '
             'capDrv moveInDriver capJeep; capDrv disableAI "MOVE"; capDrv setBehaviour "SAFE" };',
             'format ["view=%1;event=jeep;jeep=%2;driver=%3", capView, capJeep, driver capJeep] call capLog;']
NV_ON = ('call { player addWeapon "NVGoggles"; player action ["NVGOGGLES", player]; '
         'format ["view=%1;event=nv_on", capView] call capLog }')
NV_OFF = 'call { player action ["NVGOGGLES", player]; format ["view=%1;event=nv_off", capView] call capLog }'


def block(v, name, sim_time, extra_shots=(), pre=None, post=None):
    out = [f"\n// ---- {name} (off = {OFF}; on = {ON})"]
    out += capgen.begin(v, name, sim_time=sim_time)
    if pre:
        out.append(pre)
    out.append("triSimFrames 150")
    out += capgen.shot(v, name, OFF, f"{name}__off")
    out += capgen.shot(v, name, ON, f"{name}__on")
    for label, setting in extra_shots:
        out += capgen.shot(v, name, setting, f"{name}__{label}")
    if post:
        out.append(post)
    out += capgen.end(v)
    return out


def lane_views():
    hero = [h for h in views.HERO if h["name"] == "night_road_flat_drive_moonless"][0]
    town = views._timing("town_night")
    for v in (town,):
        v.pop("f10", None)
        v.pop("note", None)
    head = dict(name="road_flat_head", x=6891.9, z=8835.4, eye=3, h=69.6, p=-6, date=[1985, 3, 1, 20, 0], vd=800,
                props=JEEP_AHEAD, cleanup=views.NIGHT_JEEP_CLEANUP)
    lamps = dict(name="town_lamps", x=7152, z=8915, eye=1.75, h=352, p=0, date=[1985, 3, 17, 23, 0], vd=800)
    moon = dict(name="moon_sky", x=7264, z=8368, eye=1.8, h=108, p=26, date=[1985, 3, 1, 20, 0], vd=800)
    side = dict(name="jeep_side", x=6891.9, z=8835.4, eye=1.7, h=69.6, p=-4, date=[1985, 3, 17, 23, 0], vd=800,
                props=JEEP_SIDE, cleanup=views.NIGHT_JEEP_CLEANUP)
    day = dict(name="day_town", x=7152, z=8915, eye=1.75, h=345, p=3, date=[1985, 7, 5, 12, 0], vd=800)
    nv = dict(name="nv_town", x=7152, z=8915, eye=1.75, h=345, p=3, date=[1985, 2, 27, 22, 0], vd=800)
    return hero, town, head, lamps, moon, side, day, nv


def abel(each):
    hero, town, head, lamps, moon, side, day, nv = lane_views()
    hero_extra = []
    if each:
        hero_extra = [("lightsonly", "remastered,moonShadows=0,stars=0"),
                      ("starsonly", "remastered,nightLights=0,moonShadows=0")]
    head_extra = []
    if each:
        head_extra = [("moononly", "remastered,nightLights=0,stars=0"),
                      ("lightsonly", "remastered,moonShadows=0,stars=0")]
    lines = [capgen.header("Abel", hero, "B3 lane: night hero, moonlit road, town, lamps, moon sky, jeep, day, NV.")]
    # features_ab gives the night hero view sim clock 5600 (SIM_T0 + 6 * SIM_DT).
    lines += block(hero, "night_road_flat_drive_moonless", 5600, hero_extra)
    lines += block(head, "b3_road_flat_head", 6200, head_extra)
    lines += block(town, "b3_town_night", 6800)
    lines += block(lamps, "b3_town_lamps", 7400)
    lines += block(moon, "b3_moon_sky", 8000)
    lines += block(side, "b3_jeep_side", 8600)
    lines += block(day, "b3_day_town", 9200)
    lines += block(nv, "b3_nv_town", 9800, pre=NV_ON, post=NV_OFF)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    return lines


def dev():
    """Tuning shots on the B3 build: lamp and headlight strengths, the moon's
    shadows in the sun-visibility debug view."""
    hero, town, head, lamps, moon, side, day, nv = lane_views()
    lines = [capgen.header("Abel", hero, "B3 dev: strengths and the moon's shadows.")]
    lines += block(hero, "night_road_flat_drive_moonless", 5600, [("spot14", "remastered,nightSpot=1.4")])
    lines += block(head, "b3_road_flat_head", 6200)
    lines += block(town, "b3_town_night", 6800)
    lines += block(lamps, "b3_town_lamps", 7400, [("lamp15", "remastered,nightLamp=1.5")])
    lines += block(moon, "b3_moon_sky", 8000)
    lines += block(side, "b3_jeep_side", 8600, [("spot14", "remastered,nightSpot=1.4")])
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    return lines


def timing():
    """Gate item 7, a single reading: four night views, off then on, three
    interleaved rounds in one game (each shot reads triGpuTimes over 90
    frames, capRecord's gpu= field)."""
    hero, town, head, lamps, moon, side, day, nv = lane_views()
    lines = [capgen.header("Abel", hero, "B3 timing: off and on, three interleaved rounds.")]
    for name, v, t in (("night_road_flat_drive_moonless", hero, 5600), ("b3_road_flat_head", head, 6200),
                       ("b3_town_lamps", lamps, 7400), ("b3_town_night", town, 6800)):
        lines.append(f"\n// ---- {name} timing")
        lines += capgen.begin(v, name, sim_time=t)
        lines.append("triSimFrames 150")
        for r in range(1, 4):
            lines += capgen.shot(v, name, OFF, f"{name}__off{r}")
            lines += capgen.shot(v, name, ON, f"{name}__on{r}")
        lines += capgen.end(v)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    return lines


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--each", action="store_true", help="add each switch alone on the hero and moon views")
    ap.add_argument("--dev", action="store_true", help="the tuning shots instead of the lane stills")
    ap.add_argument("--timing", action="store_true", help="off and on, three interleaved rounds")
    ap.add_argument("--name", default="b3lane")
    a = ap.parse_args()
    out = HERE / "tests"
    out.mkdir(exist_ok=True)
    lines = dev() if a.dev else timing() if a.timing else abel(a.each)
    capgen.write(out / f"{a.name}.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / f"{a.name}.test.toml", capgen.toml("Abel", "B3 lane captures", views.WORLDS, timeout=3600))
    (out / "capture_lib.sqf").write_bytes((CAPTURES / "capture_lib.sqf").read_bytes())
    print(out / f"{a.name}.test.sqf")


if __name__ == "__main__":
    main()
