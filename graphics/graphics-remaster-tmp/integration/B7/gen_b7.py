"""B7 lane capture test (light gate items 1, 5, 6, 7 and 8 for the materials step).

    python gen_b7.py [--name b7lane] [--only view,...] [--extra tag=setting;...]

Writes, next to a copy of capture_lib.sqf (tests/), one Abel game:

  b7_town_night_rain  the features_ab materials hero block (Saint Louis 22:00,
                      overcast 0.9, rain 1, the Fire at 9 m), sim clock 8000
  b7_lowsun_wet       the lowsun_fp set (08:00 March, clear) after the rain:
                      off forces B6's ground wetness (ssrWet=1), on forces the
                      wetness state (wetness=1), so the ground under both is
                      equally wet for B6's pass
  b7_town_rain_day    the plan 7.3 wet street at 14:30 (overcast 0.85, rain 1),
                      a Jeep at 12 m: wet paint, wet walls, puddles
  b7_rain_fields      noon_fields staging in rain (14:30, overcast 0.9): grass
                      keeps no film, the Jeep and the soldier wet
  b7_noon_fields      dry noon: the gate holds the wet path off (off = on)

Per view: off ("remastered,wetWeather=0,lampStreaks=0"), on ("remastered").
On a base build the switch keys are inert (A1 registry, landed false) and the
`wetness` knob is refused, so its two shots of a view are a same-config repeat.
"""
import argparse
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = Path(r"D:\Arma_CWA_worktrees\gr-B7\arma_CWR\tests\rendering\captures")
sys.path.insert(0, str(CAPTURES))
import capgen  # noqa: E402
import views  # noqa: E402

OFF = "remastered,wetWeather=0,lampStreaks=0"
ON = "remastered"


def block(v, name, sim_time, shots):
    out = [f"\n// ---- {name}"]
    out += capgen.begin(v, name, sim_time=sim_time)
    out.append("triSimFrames 150")
    for tag, setting in shots:
        out += capgen.shot(v, name, setting, f"{name}__{tag}")
    out += capgen.end(v)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--name", default="b7lane")
    ap.add_argument("--only", default="", help="comma list of view names")
    ap.add_argument("--extra", default="", help="tag=setting;... extra shots per view (settings in full)")
    a = ap.parse_args()

    hero = [h for h in views.HERO if h["name"] == "materials_town_night_rain"][0]
    night = dict(hero, name="b7_town_night_rain")
    lowsun = views._timing("lowsun_fp")
    lowsun.pop("note", None)
    lowsun.update(name="b7_lowsun_wet")
    jeep12 = ['[["Jeep"], 12, 1, 20, "veh"] call capSpawn;']
    town_day = dict(name="b7_town_rain_day", x=7162, z=8931, eye=1.75, h=320, p=-4, date=[1985, 7, 5, 14, 30],
                    overcast=0.85, rain=1, vd=800, props=jeep12)
    fields = views._timing("noon_fields")
    fields.pop("note", None)
    rain_fields = dict(fields, name="b7_rain_fields", date=[1985, 7, 5, 14, 30], overcast=0.9, rain=1)
    noon = dict(fields, name="b7_noon_fields")

    extra = []
    for item in filter(None, a.extra.split(";")):
        tag, setting = item.split("=", 1)
        extra.append((tag, setting))

    def shots(off=OFF, on=ON):
        return [("off", off), ("on", on)] + extra

    stills = [
        (night, "b7_town_night_rain", 8000, shots()),
        (lowsun, "b7_lowsun_wet", 8600, shots(OFF + ",ssrWet=1", ON + ",wetness=1")),
        (town_day, "b7_town_rain_day", 9200, shots()),
        (rain_fields, "b7_rain_fields", 9800, shots()),
        (noon, "b7_noon_fields", 10400, [("off", OFF), ("on", ON)]),
    ]
    only = {s for s in a.only.split(",") if s}
    lines = [capgen.header("Abel", night, "B7 lane: wet night town, after-rain low sun, wet town by day, "
                                          "rain on the fields, dry noon.")]
    for v, n, t, sh in stills:
        if only and n not in only:
            continue
        lines += block(v, n, t, sh)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests"
    out.mkdir(exist_ok=True)
    (out / "capture_lib.sqf").write_bytes((CAPTURES / "capture_lib.sqf").read_bytes())
    capgen.write(out / f"{a.name}.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / f"{a.name}.test.toml", capgen.toml("Abel", "B7 lane captures", views.WORLDS, timeout=3600))
    print(f"{a.name}: written to {out}")


if __name__ == "__main__":
    main()
