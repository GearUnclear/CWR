"""look-fog lane capture tests: Original (Classic) | Overhaul (volumetric fog).

    python gen_fog.py [--variants "name=setting;name=setting"] [--only view,view] [--tag <suffix>]

Writes tests/foglane_abel<tag>.test.sqf and tests/foglane_eden<tag>.test.sqf
(+ .toml) next to a copy of capture_lib.sqf. Every view is shot as
  <view>__classic   triSetLighting "classic" (the original game look)
  <view>__base      "remastered,volFog=0" (the branch before this lane)
  <view>__fog       "remastered" (every feature on: the overhaul)
plus one shot per --variants entry (<view>__<name>, its triSetLighting setting).
1920x1080, scale 1.0, 4x MSAA, the presented frame (capgen.header).
"""
import argparse
import sys
from pathlib import Path

CAP = Path(r"D:\Arma_CWA_worktrees\look-fog\arma_CWR\tests\rendering\captures")
sys.path.insert(0, str(CAP))
import capgen  # noqa: E402
import views  # noqa: E402

HERE = Path(__file__).resolve().parent
OUT = HERE / "tests"


def timing(name):
    for v in views.TIMING_ABEL + views.TIMING_EDEN + views.TIMING_SINAI:
        if v["name"] == name:
            d = dict(v)
            d.pop("note", None)
            d.pop("f10", None)
            return d
    raise KeyError(name)


# A Jeep facing the camera 22 m up the street, headlights on (a SAFE AI
# driver held in place), for the foggy night street.
FOG_JEEP = ['capJeep = [["Jeep"], 22, 1.5, 180, "veh"] call capSpawn;',
            'capDrv = objNull;',
            'if (not (isNull capJeep)) then { capJeep lock false; "SoldierWB" createUnit [[capOX, capOZ, 0], capGrpW, '
            '"capDrv = this", 0.6, "PRIVATE"]; capDrv setCaptive true; capDrv allowDamage false; capDrv moveInDriver '
            'capJeep; capDrv disableAI "MOVE"; capDrv setBehaviour "SAFE" };',
            'format ["view=%1;event=jeep;jeep=%2;driver=%3", capView, capJeep, driver capJeep] call capLog;']

ABEL = [
    dict(timing("sunrise_ridge"), name="sunrise_ridge"),
    dict(timing("forest_edge_golden"), name="forest_edge_golden"),
    dict(timing("coastline_sunset"), name="coastline_sunset"),
    dict(timing("town_night"), name="town_night"),
    # New: a misty dawn valley (the valley view of the A0 set at 05:45).
    dict(timing("valley_0515"), name="dawn_valley", date=[1985, 7, 1, 5, 45]),
    # New: sun rays through a pine wood (the volumetric track's pinewood_morn).
    dict(timing("pinewood_morn"), name="pine_rays"),
    # New: a lit town street in fog at night, a Jeep's headlights toward the eye.
    dict(timing("town_night"), name="town_fog_night", fog=0.35,
         props=views.TOWN_FIRE + FOG_JEEP, cleanup=views.NIGHT_JEEP_CLEANUP),
    dict(timing("noon_fields"), name="noon_fields"),
]
SINAI = [
    dict(timing("sinai_lowsun_fp"), name="sinai_lowsun_fp"),
    dict(timing("sinai_desert_noon"), name="sinai_desert_noon"),
]
EDEN = [
    dict(timing("overcast_broken"), name="overcast_broken"),
    dict(timing("overcast_broken"), name="overcast_heavy", overcast=0.9),
    # New (round 2): a clear-noon long vista, terrain from 1 to 5 km (the
    # game's view distance stops at 5 km), from 150 m over the clouds view's
    # spot.
    dict(timing("broken_high"), name="noon_vista", eye=150, p=-5, date=[1985, 7, 5, 12, 30], vd=5000,
         overcast=0.05),
]


MODES = [("classic", "classic"), ("base", "remastered,volFog=0"), ("fog", "remastered")]
# base: the landed graphics-remaster with B8's air pass on (volFog off).


def view_block(v, variants, index, modes):
    out = [f"\n// ---- {v['name']}"]
    out += capgen.begin(v, v["name"], False, sim_time=2000 + 600 * index)
    out.append("triSimFrames 150")
    shots = [m for m in MODES if m[0] in modes] + variants
    for label, setting in shots:
        out += capgen.shot(v, v["name"], setting, f"{v['name']}__{label}")
    out += capgen.end(v, False)
    return out


def write(world, vs, variants, tag, modes):
    if not vs:
        return
    name = f"foglane_{world.lower()}{tag}"
    title = f"look-fog lane: {world} views, classic | base | fog (gen_fog.py)"
    lines = [capgen.header(world, vs[0], title)]
    for i, v in enumerate(vs):
        lines += view_block(v, variants, i, modes)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    capgen.write(OUT / f"{name}.test.sqf", "\n".join(lines) + "\n")
    capgen.write(OUT / f"{name}.test.toml", capgen.toml(world, title, views.WORLDS, timeout=3600))
    print(name, len(vs), "views")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--variants", default="")
    ap.add_argument("--only", default="")
    ap.add_argument("--tag", default="")
    ap.add_argument("--modes", default="classic,base,fog")
    a = ap.parse_args()
    variants = []
    for item in [s for s in a.variants.split(";") if s.strip()]:
        k, val = item.split("=", 1)
        variants.append((k.strip(), val.strip()))
    want = {s.strip() for s in a.only.split(",") if s.strip()}
    pick = (lambda vs: [v for v in vs if not want or v["name"] in want])
    modes = a.modes.split(",")
    write("Abel", pick(ABEL), variants, a.tag, modes)
    write("Eden", pick(EDEN), variants, a.tag, modes)
    write("Sinai", pick(SINAI), variants, a.tag, modes)


if __name__ == "__main__":
    main()
