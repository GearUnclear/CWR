"""B6 lane capture tests, second round (after the rough blur).

    python gen_b6v2.py --base            -> tests/b6pinbase.test.sqf (base build: the pinned prefix only)
    python gen_b6v2.py [--extra ...]      -> tests/b6v2.test.sqf (B6 build)

Both start with the same pinned prefix: per view (the ssr hero, the sunset sea,
the low-sun set dry, the dry noon field) two shots of "remastered,ssr=0" (off
and off2) with the sea clock pinned and a fixed step, so the B6 build's off can
be held against the base build's at the same place in the sequence, and off2
gives the repeat noise inside one game. The B6 build then adds, per prefix
view, a pinned "remastered" shot (on), and after the prefix the gen_b6.py
stills (off, on, tags, refl and any extras), the motion series and the timing
rounds.
"""
import argparse
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import gen_b6  # noqa: E402
from gen_b6 import CAPTURES, OFF, ON, SIM_T0, SIM_DT, block, motion, timing  # noqa: E402,F401

sys.path.insert(0, str(CAPTURES))
import capgen  # noqa: E402
import views  # noqa: E402


def pinned(setting, label):
    """One shot with the sim clock pinned to the view's capSimT and a fixed
    0.5 ms step for the 40 frames before it (the sea and the TAA history the
    same in every run)."""
    return [f'"{setting}" call capMode',
            "triSimFrames 60",
            "[] call capReadTimers",
            "[] call capPin",
            "call { capFixRes = triSetFixedStep 0.0005 }",
            "call { capMotRes = triSetSimTime capSimT }",
            "triSimFrames 40",
            f'triScreenshot "{label}"',
            f'"{label}" call capRecord',
            "call { capFixRes = triSetFixedStep 0 }"]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--base", action="store_true")
    ap.add_argument("--extra", default="", help="tag=setting;... extra on-build shots per still view")
    ap.add_argument("--only", default="", help="comma list of still views")
    ap.add_argument("--no-motion", action="store_true")
    ap.add_argument("--no-timing", action="store_true")
    ap.add_argument("--frames", type=int, default=16)
    ap.add_argument("--rounds", type=int, default=3)
    ap.add_argument("--name", default="b6v2")
    a = ap.parse_args()
    name = "b6pinbase" if a.base else a.name

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

    # The pinned prefix, in sim-clock order; its clocks sit below the stills'.
    pin_views = [(dict(bay, name="pin_bay"), "pin_bay", 900), (dict(water, name="pin_sunset"), "pin_sunset", 1200),
                 (dict(lowsun, name="pin_lowsun"), "pin_lowsun", 1500), (dict(noon, name="pin_noon"), "pin_noon", 1800)]
    lines = [capgen.header("Abel", bay, "B6 lane round 2: pinned off/off2 prefix" +
                           ("" if a.base else ", on, stills, motion, timing."))]
    for v, n, t in pin_views:
        lines.append(f"\n// ---- {n} (pinned)")
        lines += capgen.begin(v, n, sim_time=t)
        lines.append("triSimFrames 150")
        lines += pinned(OFF, f"{n}__off")
        lines += pinned(OFF, f"{n}__off2")
        if not a.base:
            lines += pinned(ON, f"{n}__on")
        lines += capgen.end(v)

    if not a.base:
        extra = []
        for item in filter(None, a.extra.split(";")):
            tag, setting = item.split("=", 1)
            extra.append((tag, "remastered," + setting))
        jeep12 = ['[["Jeep"], 12, 1, 20, "veh"] call capSpawn;']
        town = dict(name="ssr_town_rain", x=7162, z=8931, eye=1.75, h=320, p=-4, date=[1985, 7, 5, 19, 30],
                    overcast=0.9, rain=1, vd=800, props=jeep12)
        town_day = dict(town, name="ssr_town_rain_day", date=[1985, 7, 5, 14, 30], overcast=0.85)
        rain_fields = dict(noon, name="ssr_rain_fields", date=[1985, 7, 5, 14, 30], overcast=0.9, rain=1)
        dry_noon = dict(noon, name="ssr_noon_fields")
        wet_lowsun = dict(lowsun, name="ssr_wet_lowsun_fp")

        def shots(wet_force=False, classic=False):
            sfx = ",ssrWet=0.8" if wet_force else ""
            s = [("off", OFF + sfx), ("on", ON + sfx), ("tags", ON + sfx + ",debug=7"), ("refl", ON + sfx + ",debug=8")]
            s += [(t, st + sfx) for t, st in extra]
            if classic:
                s.append(("classic", "classic"))
            return s

        stills = [
            (bay, "ssr_bay_offshore", bay_t, shots(classic=True)),
            (water, "ssr_coastline_sunset", water_t, shots(classic=True)),
            (town, "ssr_town_rain", bay_t + 2400, shots()),
            (town_day, "ssr_town_rain_day", bay_t + 3000, shots()),
            (rain_fields, "ssr_rain_fields", bay_t + 3600, shots()),
            (wet_lowsun, "ssr_wet_lowsun_fp", bay_t + 4200, shots(wet_force=True)),
            (dry_noon, "ssr_noon_fields", bay_t + 4800, [("off", OFF), ("on", ON)]),
        ]
        only = {s for s in a.only.split(",") if s}
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
