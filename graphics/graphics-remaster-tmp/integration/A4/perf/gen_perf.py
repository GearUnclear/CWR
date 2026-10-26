"""A4 scratch rig (not in the repo): draw grouping and hill culling, off and on
in the same game.

Every view is shot with the two perf switches off (`remastered,drawGrouping=0,
hillCull=0`, label __off) and on (`remastered`, label __on), in 3 interleaved
rounds, the order of the two modes alternating by round (streaming warms the
second mode of a pair). What each view is for:

  sunrise_ridge      the hero view: draws, CPU frame, GPU, pixel identity
  sunrise_sunvis     the same view under debug=1 (sun visibility): the long
                     05:08 shadows of hidden objects must not change
  sunrise_walk       the eye walks 0.5 m along the heading every frame for
                     120 frames: the horizon table is rebuilt every frame; the
                     timers cover the walk; the shot is taken at its end
  noon_fields        a flatter, crowded view (the critic's "full set slower")
  noon_walk          the same walk there
  town_night         street lamps and a fire at night: the per-object light
                     gate (draw grouping with light slots and lit objects)
  forest_edge_golden a near forest (hills hide little)
  lowsun_fp          an open view (hills hide almost nothing: the governor
                     should idle)

Writes perf_abel.test.sqf/.toml next to this file (with a copy of
capture_lib.sqf, which #include resolves beside the test).
"""
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = HERE.parents[3] / "arma_CWR" / "tests" / "rendering" / "captures"
sys.path.insert(0, str(CAPTURES))

import capgen  # noqa: E402
import views  # noqa: E402

ROUNDS = 3
OFF = "remastered,drawGrouping=0,hillCull=0"
ON = "remastered"
WALK_FRAMES = 120
WALK_STEP = 0.5


def view(base, **over):
    v = views._timing(base)
    v.update(over)
    return v


VIEWS = [
    (view("sunrise_ridge"), None, False),
    (view("sunrise_ridge", name="sunrise_sunvis"), "debug=1", False),
    (view("sunrise_ridge", name="sunrise_walk"), None, True),
    (view("noon_fields"), None, False),
    (view("noon_fields", name="noon_walk"), None, True),
    (view("town_night"), None, False),
    (view("forest_edge_golden"), None, False),
    (view("lowsun_fp"), None, False),
]


def setting(base, extra):
    return base + ("," + extra if extra else "")


def walk_shot(v, label, mode, shot_label):
    out = [f'"{mode}" call capMode',
           "call { capOX0 = capOX; capOZ0 = capOZ }"]
    for i in range(1, WALK_FRAMES + 1):
        d = i * WALK_STEP
        out.append(f"call {{ capOX = capOX0 + {d:g} * (sin capH); capOZ = capOZ0 + {d:g} * (cos capH); "
                   f"[] call capPinView }}")
        out.append("triSimFrames 1")
    out += ["[] call capReadTimers",
            "[] call capPin",
            "triSimFrames 3",
            f'triScreenshot "{shot_label}"',
            f'"{shot_label}" call capRecord',
            "call { capOX = capOX0; capOZ = capOZ0; [] call capPinView }"]
    return out


def main():
    # --quick: the hero view and its walk, one round (perf_quick.test.sqf), a
    # sanity check before the long runs.
    quick = "--quick" in sys.argv
    name = "perf_quick" if quick else "perf_abel"
    rounds = 1 if quick else ROUNDS
    vs = [x for x in VIEWS if not quick or x[0]["name"] in ("sunrise_ridge", "sunrise_walk")]
    title = f"A4 draw grouping and hill culling on Abel: off and on, {rounds} interleaved rounds."
    lines = [capgen.header("Abel", vs[0][0], title)]
    t = 1000
    for rnd in range(1, rounds + 1):
        for v, extra, walk in vs:
            label = f"{v['name']}_r{rnd}"
            lines.append(f"\n// ---- {v['name']} round {rnd}")
            lines += capgen.begin(v, label, sim_time=t)
            t += 100
            lines.append("triSimFrames 150")
            modes = [("off", setting(OFF, extra)), ("on", setting(ON, extra))]
            if rnd % 2 == 0:
                modes.reverse()
            for tag, mode in modes:
                if walk:
                    lines += walk_shot(v, label, mode, f"{label}__{tag}")
                else:
                    lines += capgen.shot(v, label, mode, f"{label}__{tag}")
            lines += capgen.end(v)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    capgen.write(HERE / f"{name}.test.sqf", "\n".join(lines) + "\n")
    capgen.write(HERE / f"{name}.test.toml", capgen.toml("Abel", title, views.WORLDS, timeout=7200))
    print(f"{name}: {len(vs)} views x {rounds} rounds")


if __name__ == "__main__":
    main()
