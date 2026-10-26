"""A4 lane extra views (one round, not in the repo): the cases the a4lane hero
set does not cover, each shot off (both perf switches 0) then on in one game.

    python gen_extra.py

  town_night         street lamps and a fire at night: drawGrouping's per-object
                     light gate (lit objects stay out of runs) must keep the
                     frame exact
  noon_fields        a flatter, crowded noon view (the critic's "on noon_fields
                     the full set was slower")
  noon_walk          the eye walks 0.5 m along the heading every frame for 120
                     frames: the horizon table is rebuilt every frame and the
                     timers cover the walk
  sunrise_walk       the same walk at the hero view
  forest_edge_golden a near forest (hills hide little)
  lowsun_fp          an open view (hills hide almost nothing: the governor idles)

Writes tests/a4extra.test.sqf + .toml next to a copy of capture_lib.sqf.
"""
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = Path(r"D:\Arma_CWA_worktrees\gr-A4\arma_CWR\tests\rendering\captures")
sys.path.insert(0, str(CAPTURES))
import capgen  # noqa: E402
import views  # noqa: E402

OFF = "remastered,drawGrouping=0,hillCull=0"
ON = "remastered"
WALK_FRAMES = 120
WALK_STEP = 0.5


def view(base, **over):
    v = views._timing(base)
    v.update(over)
    return v


NIGHT_ONLY = "--night" in sys.argv

VIEWS = [
    (view("town_night"), False),
    (view("noon_fields"), False),
    (view("noon_fields", name="noon_walk"), True),
    (view("sunrise_ridge", name="sunrise_walk"), True),
    (view("forest_edge_golden"), False),
    (view("lowsun_fp"), False),
]


def walk_shot(v, mode, shot_label):
    out = [f'"{mode}" call capMode', "call { capOX0 = capOX; capOZ0 = capOZ }"]
    for i in range(1, WALK_FRAMES + 1):
        d = i * WALK_STEP
        out.append(f"call {{ capOX = capOX0 + {d:g} * (sin capH); capOZ = capOZ0 + {d:g} * (cos capH); "
                   f"[] call capPinView }}")
        out.append("triSimFrames 1")
    out += ["[] call capReadTimers", "[] call capPin", "triSimFrames 3", f'triScreenshot "{shot_label}"',
            f'"{shot_label}" call capRecord', "call { capOX = capOX0; capOZ = capOZ0; [] call capPinView }"]
    return out


def main():
    title = "A4 lane extra views: light gate at night, noon, walks, forest, open view; off then on."
    vs = VIEWS
    stem = "a4extra"
    if NIGHT_ONLY:
        # town_night without the fire (its flicker hides everything): the
        # street lamps alone, steady, light the houses and the road.
        lamps = view("town_night", name="town_night_lamps", props=[])
        lamps.pop("refresh", None)
        vs = [(lamps, False)]
        stem = "a4night"
        title = "A4 lane: town_night street lamps without the fire, off then on."
    lines = [capgen.header("Abel", vs[0][0], title)]
    t = 1000
    for v, walk in vs:
        name = v["name"]
        lines.append(f"\n// ---- {name}")
        v.pop("f10", None)
        lines += capgen.begin(v, name, sim_time=t)
        t += 100
        lines.append("triSimFrames 150")
        for tag, mode in (("off", OFF), ("on", ON)):
            if walk:
                lines += walk_shot(v, mode, f"{name}__{tag}")
            else:
                lines += capgen.shot(v, name, mode, f"{name}__{tag}")
        lines += capgen.end(v)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests"
    capgen.write(out / f"{stem}.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / f"{stem}.test.toml", capgen.toml("Abel", title, views.WORLDS, timeout=3600))
    print(out / f"{stem}.test.sqf")


main()
