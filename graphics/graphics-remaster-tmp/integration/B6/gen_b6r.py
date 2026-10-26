"""B6 lane capture tests, review round (fog fade, covered panels, dot lattice).

    python gen_b6r.py            -> tests/b6r.test.sqf (Abel, B6 build)
    python gen_b6r.py --eden     -> tests/b6reden.test.sqf (Eden, B6 build)

Abel: the pinned prefix of gen_b6v2.py first (the same place in the sequence
as the base build's b6pinbase run: off, off2, on per view), then the stills
(the bay hero and the sunset sea for the water path, the two town rain views
and the rain field for the review items, the dry noon gate): off, on, tags,
refl; then 16 frames off and 16 on at the wet street by day (the sim clock
1/30 s per frame, TAA on: the dot lattice in motion), then one timing round
(bay, town).

Eden: the research's clutter bush in the overcast rain (ssr NOTES.md,
detail_overcast_bush.png): off, on, tags, refl, the box under a bush that the
covered panels must not bring back.
"""
import argparse
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import gen_b6  # noqa: E402
from gen_b6 import CAPTURES, OFF, ON, SIM_T0, SIM_DT, block, timing  # noqa: E402
from gen_b6v2 import pinned  # noqa: E402

sys.path.insert(0, str(CAPTURES))
import capgen  # noqa: E402
import views  # noqa: E402


def motion(v, name, frames, sim_time, settings):
    out = [f"\n// ---- {name}: the sim clock steps 1/30 s per frame, {frames} frames off then on"]
    out += capgen.begin(v, name, sim_time=sim_time)
    out.append("triSimFrames 150")
    for label, setting in settings:
        out.append(f'"{setting}" call capMode')
        out.append("triSimFrames 30")
        out.append("[] call capPin")
        out.append('call { capFixRes = triSetFixedStep 0.0005; format ["view=%1;event=fixedstep;res=%2", capView, '
                   'capFixRes] call capLog }')
        for k in range(frames):
            out.append(f"call {{ capMotRes = triSetSimTime ({sim_time} + {k} / 30) }}")
            out.append("triSimFrames 2")
            lab = f"{name}__{label}__f{k:03d}"
            out.append(f'triScreenshot "{lab}"')
            out.append(f'call {{ format ["view=%1;event=frame;label={lab};k={k};mode=%2;sim=%3;feat=%4", capView, '
                       f'capModeName, capMotRes, triFeatureState] call capLog }}')
        out.append('call { capFixRes = triSetFixedStep 0 }')
    out += capgen.end(v)
    return out


def shots(wet_force=False):
    sfx = ",ssrWet=0.8" if wet_force else ""
    return [("off", OFF + sfx), ("on", ON + sfx), ("tags", ON + sfx + ",debug=7"), ("refl", ON + sfx + ",debug=8")]


def abel(frames, rounds):
    def H(n):
        return [h for h in views.HERO if h["name"] == n][0]

    bay = H("ssr_bay_offshore")
    bay_t = SIM_T0 + SIM_DT * views.HERO.index(bay)
    water = dict(H("water_coastline_sunset"))
    water_t = SIM_T0 + SIM_DT * views.HERO.index(water)
    lowsun = views._timing("lowsun_fp")
    lowsun.pop("note", None)
    noon = views._timing("noon_fields")
    noon.pop("note", None)

    pin_views = [(dict(bay, name="pin_bay"), "pin_bay", 900), (dict(water, name="pin_sunset"), "pin_sunset", 1200),
                 (dict(lowsun, name="pin_lowsun"), "pin_lowsun", 1500), (dict(noon, name="pin_noon"), "pin_noon", 1800)]
    lines = [capgen.header("Abel", bay, "B6 review round: pinned prefix, water and wet stills, motion on the wet "
                                        "street, one timing round.")]
    for v, n, t in pin_views:
        lines.append(f"\n// ---- {n} (pinned)")
        lines += capgen.begin(v, n, sim_time=t)
        lines.append("triSimFrames 150")
        lines += pinned(OFF, f"{n}__off")
        lines += pinned(OFF, f"{n}__off2")
        lines += pinned(ON, f"{n}__on")
        lines += capgen.end(v)

    jeep12 = ['[["Jeep"], 12, 1, 20, "veh"] call capSpawn;']
    town = dict(name="ssr_town_rain", x=7162, z=8931, eye=1.75, h=320, p=-4, date=[1985, 7, 5, 19, 30],
                overcast=0.9, rain=1, vd=800, props=jeep12)
    town_day = dict(town, name="ssr_town_rain_day", date=[1985, 7, 5, 14, 30], overcast=0.85)
    rain_fields = dict(noon, name="ssr_rain_fields", date=[1985, 7, 5, 14, 30], overcast=0.9, rain=1)
    dry_noon = dict(noon, name="ssr_noon_fields")
    stills = [
        (bay, "ssr_bay_offshore", bay_t, [("off", OFF), ("on", ON), ("refl", ON + ",debug=8")]),
        (water, "ssr_coastline_sunset", water_t, [("off", OFF), ("on", ON)]),
        (town, "ssr_town_rain", bay_t + 2400, shots()),
        (town_day, "ssr_town_rain_day", bay_t + 3000, shots()),
        (rain_fields, "ssr_rain_fields", bay_t + 3600, shots()),
        (dry_noon, "ssr_noon_fields", bay_t + 4800, [("off", OFF), ("on", ON)]),
    ]
    stills.sort(key=lambda s: s[2])
    for v, n, t, sh in stills:
        lines += block(v, n, t, sh)
    t_next = bay_t + 5400
    lines += motion(town_day, "ssr_motion_town", frames, t_next, [("off", OFF), ("on", ON)])
    t_next += 600
    if rounds:
        lines += timing([(bay, "tim_bay", OFF, ON), (town, "tim_town", OFF, ON)], rounds, t_next)
    return lines


def eden():
    bush = dict(name="ssr_eden_bush", x=9200, z=3700, eye=1.75, h=288, p=-4, date=[1985, 7, 5, 14, 30],
                overcast=0.9, rain=1, vd=3500, props=['[["Jeep"], 12, -5, 70, "veh"] call capSpawn;'])
    lines = [capgen.header("Eden", bush, "B6 review round: the research's clutter bush in the overcast rain.")]
    lines += block(bush, "ssr_eden_bush", 3000, shots())
    return lines


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--eden", action="store_true")
    ap.add_argument("--frames", type=int, default=16)
    ap.add_argument("--rounds", type=int, default=1)
    a = ap.parse_args()
    name = "b6reden" if a.eden else "b6r"
    world = "Eden" if a.eden else "Abel"
    lines = eden() if a.eden else abel(a.frames, a.rounds)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests"
    out.mkdir(exist_ok=True)
    (out / "capture_lib.sqf").write_bytes((CAPTURES / "capture_lib.sqf").read_bytes())
    capgen.write(out / f"{name}.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / f"{name}.test.toml", capgen.toml(world, "B6 review captures", views.WORLDS, timeout=3600))
    print(f"{name}: written to {out}")


if __name__ == "__main__":
    main()
