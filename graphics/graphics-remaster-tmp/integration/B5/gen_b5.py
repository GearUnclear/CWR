"""B5 lane capture tests (light gate items 1, 5, 6 and 8 for the water step).

    python gen_b5.py [--frames 24]

Writes, next to a copy of capture_lib.sqf (tests/):

  tests/b5lane.test.sqf   one Abel game. Per view three shots with the sim
                          clock pinned: off ("remastered,seaWaves=0"), on
                          ("remastered") and the original look ("classic"):
      water_coastline_sunset  the features_ab water hero block, unchanged
                              (eye 3 m ASL, h 278, p -4, 18:35, sim clock 6800,
                              clouds pinned)
      water_sea_noon          plan 7.3 noon clip start: eye 12 m, h 278, p -12,
                              12:30 (the noon colour check against Classic)
      water_shore_midsun      the water research view: eye 15 m, h 278, p -22,
                              16:30, sun ahead (glint patch, time dependent)
      water_night             the noon framing (eye 12 m, p -8) under the
                              town_night date (1985-02-27 22:00, moon high);
                              the hero framing (eye 3 m ASL) showed a dark band,
                              not open sea, at that date in the first base run
                          then the motion series at the hero view: the sea clock
                          steps 1/30 s per frame (triSetFixedStep 0.0005 keeps
                          the two rendered frames per shot from moving it), N
                          frames off, then N frames on (labels
                          water_motion__off__fNNN / __on__fNNN): an animation
                          frame of the sea texture lasts 1/7 s, so a hard frame
                          switch shows as a spike in the frame-to-frame change
                          every 4 to 5 frames.

On the base build the switch is registered but inert, so off and on draw the
same frame; that build's shots are the BEFORE images.
"""
import argparse
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = Path(r"D:\Arma_CWA_worktrees\gr-B5\arma_CWR\tests\rendering\captures")
sys.path.insert(0, str(CAPTURES))
import capgen  # noqa: E402
import views  # noqa: E402

OFF = "remastered,seaWaves=0"
ON = "remastered"
CLASSIC = "classic"
SIM_T0, SIM_DT = 2000, 600  # gen_features_ab.py
# Extra shots of the high-sun views (tuning the noon colour; the switch is
# inert on the base build, so there they repeat the on shot): --sweep a,b:...
SWEEP = []
HERO_SWEEP = []


def three(v, name, sim_time, extra=()):
    out = [f"\n// ---- {name} (off = {OFF}; on = {ON}; classic)"]
    out += capgen.begin(v, name, sim_time=sim_time)
    out.append("triSimFrames 150")
    out += capgen.shot(v, name, OFF, f"{name}__off")
    out += capgen.shot(v, name, ON, f"{name}__on")
    out += capgen.shot(v, name, CLASSIC, f"{name}__classic")
    for tag, setting in extra:
        out += capgen.shot(v, name, setting, f"{name}__{tag}")
    out += capgen.end(v)
    return out


def motion(v, frames, sim_time):
    name = "water_motion"
    out = [f"\n// ---- {name}: the sea clock steps 1/30 s per frame, {frames} frames off then on"]
    out += capgen.begin(v, name, sim_time=sim_time)
    out.append("triSimFrames 150")
    for label, setting in (("off", OFF), ("on", ON)):
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


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--frames", type=int, default=24)
    ap.add_argument("--sweep", default="", help="tag=setting;tag=setting (settings after 'remastered,')")
    ap.add_argument("--hero-sweep", default="", help="the same for the water hero view")
    ap.add_argument("--name", default="b5lane")
    a = ap.parse_args()
    for item in filter(None, a.sweep.split(";")):
        tag, setting = item.split("=", 1)
        SWEEP.append((tag, "remastered," + setting))
    for item in filter(None, a.hero_sweep.split(";")):
        tag, setting = item.split("=", 1)
        HERO_SWEEP.append((tag, "remastered," + setting))
    hero = [h for h in views.HERO if h["name"] == "water_coastline_sunset"][0]
    hero_index = views.HERO.index(hero)
    noon = views._timing("sea_noon")
    noon.update(name="water_sea_noon")
    noon.pop("note", None)
    shore = dict(name="water_shore_midsun", x=1550, z=5150, eye=15, h=278, p=-22, date=[1985, 7, 5, 16, 30], vd=4000)
    night = dict(noon)
    night.update(name="water_night", p=-8, date=[1985, 2, 27, 22, 0])
    lines = [capgen.header("Abel", hero, "B5 lane: water hero, sea noon, mid-sun shore, night, motion.")]
    # features_ab gives the water hero view sim clock SIM_T0 + index * SIM_DT.
    t = SIM_T0 + SIM_DT * hero_index
    lines += three(hero, hero["name"], t, HERO_SWEEP)
    lines += three(noon, noon["name"], t + 600, SWEEP)
    lines += three(shore, shore["name"], t + 1200, SWEEP)
    lines += three(night, night["name"], t + 1800)
    lines += motion(hero, a.frames, t + 2400)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests"
    out.mkdir(exist_ok=True)
    (out / "capture_lib.sqf").write_bytes((CAPTURES / "capture_lib.sqf").read_bytes())
    capgen.write(out / f"{a.name}.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / f"{a.name}.test.toml", capgen.toml("Abel", "B5 lane captures", views.WORLDS, timeout=3600))
    print(f"{a.name}: 4 views x 3 shots + {len(SWEEP)} x 2 sweep shots + {2 * a.frames} motion frames; "
          f"hero sim clock {t}")


if __name__ == "__main__":
    main()
