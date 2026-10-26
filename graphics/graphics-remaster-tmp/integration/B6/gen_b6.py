"""B6 lane capture tests (light gate items 1, 5, 6, 7 and 8 for the ssr step).

    python gen_b6.py [--frames 16] [--name b6lane] [--base]

Writes, next to a copy of capture_lib.sqf (tests/), one Abel game:

  ssr_bay_offshore     the features_ab ssr hero block (eye 3 m ASL, h 272,
                       p -3, 06:30, BoatW at 45 m), sim clock 7400, clouds pinned
  ssr_coastline_sunset the water hero framing (eye 3 m ASL, h 278, p -4,
                       18:35): the pale lilac sunset sea check
  ssr_town_rain        plan 7.3 wet-street row: 7162, 8931, eye 1.75, h 320,
                       p -4, 19:30, overcast 0.9, rain 1, VD 800, Jeep at 12 m
  ssr_town_rain_day    the same street at the research's 14:30 (overcast 0.85)
  ssr_rain_fields      noon_fields staging in rain (overcast 0.9, rain 1, 14:30):
                       grass stays dry, no boxes under the Jeep, soldier, bushes
  ssr_wet_lowsun_fp    the lowsun_fp set, clear, wetness forced (ssrWet=0.8;
                       the critic's boxes under the Jeep and the soldier)
  ssr_noon_fields      dry noon without water: the gate holds SSR off

Per view: off ("remastered,ssr=0"), on ("remastered"), and on the B6 build the
tag view (debug=7) and the reflection-only view (debug=8). The hero view also
gets classic. Then the motion series at the hero view (the sea clock steps
1/30 s per frame, N frames off then N on) and three interleaved timing rounds
(off, on) at the hero view and the wet street.

--base writes the copy for the base build (b6base): only the keys the base
build knows (no ssrWet, no debug views 7 and 8).
"""
import argparse
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = Path(r"D:\Arma_CWA_worktrees\gr-B6\arma_CWR\tests\rendering\captures")
sys.path.insert(0, str(CAPTURES))
import capgen  # noqa: E402
import views  # noqa: E402

OFF = "remastered,ssr=0"
ON = "remastered"
SIM_T0, SIM_DT = 2000, 600  # gen_features_ab.py


def block(v, name, sim_time, shots):
    out = [f"\n// ---- {name}"]
    out += capgen.begin(v, name, sim_time=sim_time)
    out.append("triSimFrames 150")
    for tag, setting in shots:
        out += capgen.shot(v, name, setting, f"{name}__{tag}")
    out += capgen.end(v)
    return out


def motion(v, frames, sim_time, settings):
    name = "ssr_motion"
    out = [f"\n// ---- {name}: the sea clock steps 1/30 s per frame, {frames} frames off then on"]
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


def timing(vs, rounds, t0):
    """Interleaved rounds; every view of every round gets its own, later sim
    clock (time only jumps forward between views)."""
    out = ["\n// ---- timing: interleaved rounds, off then on, per view"]
    t = t0
    for r in range(rounds):
        for v, name, off, on in vs:
            lab = f"{name}_t{r}"
            out += capgen.begin(v, lab, sim_time=t)
            t += 300
            out.append("triSimFrames 120")
            out += capgen.shot(v, lab, off, f"{lab}__off")
            out += capgen.shot(v, lab, on, f"{lab}__on")
            out += capgen.end(v)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--frames", type=int, default=16)
    ap.add_argument("--rounds", type=int, default=3)
    ap.add_argument("--name", default="b6lane")
    ap.add_argument("--base", action="store_true")
    ap.add_argument("--only", default="", help="comma list of view names (stills only)")
    ap.add_argument("--no-motion", action="store_true")
    ap.add_argument("--no-timing", action="store_true")
    ap.add_argument("--extra", default="", help="tag=setting;... extra on-build shots per view (after 'remastered,')")
    a = ap.parse_args()
    name = "b6base" if a.base else a.name

    def H(n):
        return [h for h in views.HERO if h["name"] == n][0]

    bay = H("ssr_bay_offshore")
    bay_t = SIM_T0 + SIM_DT * views.HERO.index(bay)
    water = dict(H("water_coastline_sunset"))
    water_t = SIM_T0 + SIM_DT * views.HERO.index(water)
    jeep12 = ['[["Jeep"], 12, 1, 20, "veh"] call capSpawn;']
    town = dict(name="ssr_town_rain", x=7162, z=8931, eye=1.75, h=320, p=-4, date=[1985, 7, 5, 19, 30],
                overcast=0.9, rain=1, vd=800, props=jeep12)
    town_day = dict(town, name="ssr_town_rain_day", date=[1985, 7, 5, 14, 30], overcast=0.85)
    fields = views._timing("noon_fields")
    fields.pop("note", None)
    rain_fields = dict(fields, name="ssr_rain_fields", date=[1985, 7, 5, 14, 30], overcast=0.9, rain=1)
    noon = dict(fields, name="ssr_noon_fields")
    lowsun = views._timing("lowsun_fp")
    lowsun.pop("note", None)
    lowsun.update(name="ssr_wet_lowsun_fp")

    extra = []
    for item in filter(None, a.extra.split(";")):
        tag, setting = item.split("=", 1)
        extra.append((tag, "remastered," + setting))

    def shots(wet_force=False, classic=False):
        off, on = OFF, ON
        if wet_force and not a.base:
            off, on = OFF + ",ssrWet=0.8", ON + ",ssrWet=0.8"
        s = [("off", off), ("on", on)]
        if not a.base:
            s += [("tags", on + ",debug=7"), ("refl", on + ",debug=8")]
            s += [(t, st + (",ssrWet=0.8" if wet_force else "")) for t, st in extra]
        if classic:
            s.append(("classic", "classic"))
        return s

    stills = [
        (bay, "ssr_bay_offshore", bay_t, shots(classic=True)),
        (water, "ssr_coastline_sunset", water_t, shots(classic=True)),
        (town, "ssr_town_rain", bay_t + 2400, shots()),
        (town_day, "ssr_town_rain_day", bay_t + 3000, shots()),
        (rain_fields, "ssr_rain_fields", bay_t + 3600, shots()),
        (lowsun, "ssr_wet_lowsun_fp", bay_t + 4200, shots(wet_force=True)),
        (noon, "ssr_noon_fields", bay_t + 4800, [("off", OFF), ("on", ON)]),
    ]
    only = {s for s in a.only.split(",") if s}
    lines = [capgen.header("Abel", bay, "B6 lane: ssr hero, sunset sea, wet streets and fields, dry noon, "
                                        "motion, timing.")]
    # Views in sim-clock order (time only jumps forward between views).
    stills.sort(key=lambda s: s[2])
    for v, n, t, sh in stills:
        if only and n not in only:
            continue
        lines += block(v, n, t, sh)
    t_next = bay_t + 5400
    if not a.no_motion:
        lines += motion(bay, a.frames, t_next, [("off", OFF), ("on", ON)])
    t_next += 600
    if not a.no_timing:
        lines += timing([(bay, "tim_bay", OFF, ON), (town, "tim_town", OFF, ON)], a.rounds, t_next)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests"
    out.mkdir(exist_ok=True)
    (out / "capture_lib.sqf").write_bytes((CAPTURES / "capture_lib.sqf").read_bytes())
    capgen.write(out / f"{name}.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / f"{name}.test.toml", capgen.toml("Abel", "B6 lane captures", views.WORLDS, timeout=3600))
    print(f"{name}: written to {out}")


if __name__ == "__main__":
    main()
