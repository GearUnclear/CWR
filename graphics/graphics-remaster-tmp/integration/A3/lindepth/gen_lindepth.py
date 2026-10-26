"""A3 scratch rig (not in the repo): the cost and the picture of the LinearDepth
stage. PostLinearDepth runs only for the linear depth debug view (debug=5) in
A3, so this shoots five A0 timing views on Abel in 3 interleaved rounds, each
as `remastered` (label __off) and `remastered,debug=5` (label __on, the debug
view over the frame), with the GPU sections of each shot. The `lindepth`
section of the __on shots is the stage's cost; the __on images show what it
computes. Writes lindepth_abel.test.sqf/.toml next to this file (with a copy of
capture_lib.sqf, which #include resolves beside the test).
"""
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = HERE.parents[3] / "arma_CWR" / "tests" / "rendering" / "captures"
sys.path.insert(0, str(CAPTURES))

import capgen  # noqa: E402
import views  # noqa: E402

NAMES = ["sunrise_ridge", "forest_edge_golden", "coastline_sunset", "woods_pine2_evening", "noon_fields"]
ROUNDS = 3


def main():
    vs = [v for v in views.TIMING_ABEL if v["name"] in NAMES]
    assert len(vs) == len(NAMES)
    title = "A3 linear depth cost on Abel: remastered and debug=5, 3 interleaved rounds."
    lines = [capgen.header("Abel", vs[0], title)]
    for rnd in range(1, ROUNDS + 1):
        for i, v in enumerate(vs):
            label = f"{v['name']}_r{rnd}"
            lines.append(f"\n// ---- {v['name']} round {rnd}")
            lines += capgen.begin(v, label, sim_time=100 * (rnd * 10 + i + 1))
            lines.append("triSimFrames 150")
            lines += capgen.shot(v, label, "remastered", f"{label}__off")
            lines += capgen.shot(v, label, "remastered,debug=5", f"{label}__on")
            lines += capgen.end(v)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    capgen.write(HERE / "lindepth_abel.test.sqf", "\n".join(lines) + "\n")
    capgen.write(HERE / "lindepth_abel.test.toml", capgen.toml("Abel", title, views.WORLDS))
    print(f"{len(vs)} views x {ROUNDS} rounds")


if __name__ == "__main__":
    main()
