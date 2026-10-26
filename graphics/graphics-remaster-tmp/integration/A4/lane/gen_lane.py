"""A4 lane capture test (light gate items 1, 5, 6, 7 and the plan 5 A4 shadow check).

    python gen_lane.py

Writes tests/a4lane.test.sqf + .toml (next to a copy of capture_lib.sqf). One
Abel game; every pair is shot in that game, off first, then on:

  perf_sunrise_ridge   the features_ab perf hero block, unchanged (same sim
                       clock 2000 and cloud pin): off = both switches 0, on =
                       remastered
  a4hill_sunrise_ridge off = both 0, on = hillCull only (drawGrouping=0)
  a4group_sunrise_ridge off = both 0, on = drawGrouping only (hillCull=0)
  a4sunvis_sunrise_ridge the same with debug=1 (sun visibility): shadows cast
                       by objects the cull hides must still fall on ground in view
  a4walk_sunrise_end   the end view of the perf clip (4426, 3315, h 75)
  a4valley_0515        valley 05:15, a second long view (11,404 draws in A0)

On a build before A4 the switch keys are registered but inert, so off and on
are the same frame there; that build's shots are the BEFORE images.
"""
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = Path(r"D:\Arma_CWA_worktrees\gr-A4\arma_CWR\tests\rendering\captures")
sys.path.insert(0, str(CAPTURES))
import capgen  # noqa: E402
import views  # noqa: E402

BOTH0 = "remastered,drawGrouping=0,hillCull=0"


def pair(v, name, off, on, sim_time):
    out = [f"\n// ---- {name} (off = {off}; on = {on})"]
    out += capgen.begin(v, name, sim_time=sim_time)
    out.append("triSimFrames 150")
    out += capgen.shot(v, name, off, f"{name}__off")
    out += capgen.shot(v, name, on, f"{name}__on")
    out += capgen.end(v)
    return out


def main():
    hero = [h for h in views.HERO if h["name"] == "perf_sunrise_ridge"][0]
    walk = dict(hero, name="a4walk_sunrise_end", x=4426, z=3315, h=75)
    valley = views._timing("valley_0515")
    valley.pop("f10", None)
    lines = [capgen.header("Abel", hero, "A4 lane: perf hero, attribution, sun visibility, walk end, valley.")]
    # features_ab gives the perf hero view sim clock 2000 (SIM_T0 + 0 * SIM_DT).
    lines += pair(hero, "perf_sunrise_ridge", BOTH0, "remastered", 2000)
    lines += pair(hero, "a4hill_sunrise_ridge", BOTH0, "remastered,drawGrouping=0", 2600)
    lines += pair(hero, "a4group_sunrise_ridge", BOTH0, "remastered,hillCull=0", 3200)
    lines += pair(hero, "a4sunvis_sunrise_ridge", BOTH0 + ",debug=1", "remastered,debug=1", 3800)
    lines += pair(walk, "a4walk_sunrise_end", BOTH0, "remastered", 4400)
    lines += pair(valley, "a4valley_0515", BOTH0, "remastered", 5000)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests"
    capgen.write(out / "a4lane.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "a4lane.test.toml", capgen.toml("Abel", "A4 lane captures", views.WORLDS, timeout=3600))
    print(out / "a4lane.test.sqf")


main()
