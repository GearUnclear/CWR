"""B8 lane capture tests (light gate items 1, 5, 6 and 8 for the air pass).

    python gen_b8.py

Writes, next to a copy of capture_lib.sqf, tests/b8lane.test.sqf: one Abel
game, every view shot with the three air switches off, on, and the variants
below. On the base build the keys are registered but inert, so every shot of a
view draws the same frame there; that build's shots are the BEFORE images.

  airbeams_pinewood_morn   the features_ab beams hero block (sim clock 8600):
                           06:00, fog 0.1, sun behind the pine crowns
  airhaze_valley_0515      the features_ab haze hero block (sim clock 9200):
                           sunrise over the valley and the lake
  b8_pine_eve              7.3's gap-beam row: 18:15, clear, cypress row
  b8_lowsun_fp             clear 08:00 in March, open ground toward a low
                           sun (the horizon glow band check)
  b8_sunrise_ridge         05:08 into a 5 deg sun (haze, mist, beams)
  b8_noon_fields           noon, clear: the gate-off reference (beams and
                           mist off, far haze only)
  b8_town_night            lamps and a fire at night (the brown sky check)
  b8_hl_front              7.3's headlight row: a Jeep with its lights on
                           14 m up a forest road at 22:00, fog 0.2

Labels: <view>__off (all three off), __on (remastered), __offnotaa and
__onnotaa (the same with taa=0: shots repeat to the pixel without TAA's
jitter), and on the hero views __beams (haze and mist off), __haze (beams
off), __nomist (valley mist off) and __debug (debug=6: the air's in-scatter
alone).
"""
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = Path(r"D:\Arma_CWA_worktrees\gr-B8\arma_CWR\tests\rendering\captures")
sys.path.insert(0, str(CAPTURES))
import capgen  # noqa: E402
import views  # noqa: E402

OFF = "remastered,airBeams=0,airHaze=0,valleyMist=0"
ON = "remastered"

HL_DRIVER = [
    'capJeep = [["Jeep"], 14.66, 2.49, 170, "veh"] call capSpawn;',
    'capDrv = objNull;',
    'if (not (isNull capJeep)) then { capJeep lock false; "SoldierWB" createUnit [[capOX, capOZ, 0], capGrpW, '
    '"capDrv = this", 0.6, "PRIVATE"]; capDrv setCaptive true; capDrv allowDamage false; capDrv disableAI "MOVE"; '
    'capDrv moveInDriver capJeep; capDrv setBehaviour "SAFE" };',
    'format ["view=%1;event=jeep;jeep=%2;driver=%3", capView, capJeep, driver capJeep] call capLog;',
]


def block(v, name, sim_time, extra=(), settle=150):
    out = [f"\n// ---- {name} (off = {OFF}; on = {ON})"]
    out += capgen.begin(v, name, sim_time=sim_time)
    out.append(f"triSimFrames {settle}")
    out += capgen.shot(v, name, OFF, f"{name}__off")
    out += capgen.shot(v, name, ON, f"{name}__on")
    out += capgen.shot(v, name, OFF + ",taa=0", f"{name}__offnotaa")
    out += capgen.shot(v, name, ON + ",taa=0", f"{name}__onnotaa")
    for label, setting in extra:
        out += capgen.shot(v, name, setting, f"{name}__{label}")
    out += capgen.end(v)
    return out


def abel():
    hero_b = [h for h in views.HERO if h["name"] == "airbeams_pinewood_morn"][0]
    hero_h = [h for h in views.HERO if h["name"] == "airhaze_valley_0515"][0]
    pine_eve = dict(name="pine_eve", x=4355, z=3275, eye=1.75, h=288, p=5, date=[1985, 7, 5, 18, 15], vd=1500)
    low = views._timing("lowsun_fp")
    ridge = views._timing("sunrise_ridge")
    noon = views._timing("noon_fields")
    town = views._timing("town_night")
    hl = dict(name="hl_front", x=7266.4, z=8355.9, eye=1.6, h=200, p=-1, date=[1985, 2, 27, 22, 0], vd=800, fog=0.2,
              props=HL_DRIVER, cleanup=views.NIGHT_JEEP_CLEANUP)
    for v in (low, ridge, noon, town):
        v.pop("f10", None)
        v.pop("note", None)
    split = [("beams", "remastered,airHaze=0,valleyMist=0"), ("haze", "remastered,airBeams=0")]
    lines = [capgen.header("Abel", hero_b, "B8 lane: air pass views (beams, haze, mist, headlights).")]
    # features_ab gives the two hero views sim clocks 8600 and 9200 (SIM_T0 + i * SIM_DT).
    lines += block(hero_b, "airbeams_pinewood_morn", 8600, extra=split + [("debug", "remastered,debug=6")])
    lines += block(hero_h, "airhaze_valley_0515", 9200, extra=split + [("nomist", "remastered,valleyMist=0")])
    lines += block(pine_eve, "b8_pine_eve", 9800)
    lines += block(low, "b8_lowsun_fp", 10400, extra=split + [("nomist", "remastered,valleyMist=0")])
    lines += block(ridge, "b8_sunrise_ridge", 11000, extra=split)
    lines += block(noon, "b8_noon_fields", 11600)
    lines += block(town, "b8_town_night", 12200, settle=300)
    lines += block(hl, "b8_hl_front", 12800, settle=300)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests"
    capgen.write(out / "b8lane.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "b8lane.test.toml", capgen.toml("Abel", "B8 lane captures", views.WORLDS, timeout=3600))
    (out / "capture_lib.sqf").write_bytes((CAPTURES / "capture_lib.sqf").read_bytes())
    print(out / "b8lane.test.sqf")


if __name__ == "__main__":
    abel()
