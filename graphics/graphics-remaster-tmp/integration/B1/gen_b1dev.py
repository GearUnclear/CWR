"""B1 tuning run: leaf bias and sun size sweeps, TAA and no-TAA soft paths.

    python gen_b1dev.py   ->  tests/b1dev.test.sqf (one Abel game)
"""
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = Path(r"D:\Arma_CWA_worktrees\gr-B1\arma_CWR\tests\rendering\captures")
sys.path.insert(0, str(CAPTURES))
import capgen  # noqa: E402
import views  # noqa: E402


def sweep(v, name, sim_time, shots):
    out = [f"\n// ---- {name}"]
    out += capgen.begin(v, name, sim_time=sim_time)
    out.append("triSimFrames 150")
    for label, setting in shots:
        out += capgen.shot(v, name, setting, f"{name}__{label}")
    out += capgen.end(v)
    return out


forest = views._timing("forest_edge_golden")
forest.pop("note", None)
hero = [h for h in views.HERO if h["name"] == "shadows_woods_pine2_evening"][0]
lines = [capgen.header("Abel", forest, "B1 tuning: leaf bias, sun size, TAA and no-TAA soft paths.")]
import os
if os.environ.get("B1DEV") == "2":
    lines += sweep(forest, "dev_forest", 4000, [
        ("off", "remastered,treeShadows=0,softShadows=0"),
        ("r40", "remastered,shadowReach=40"),
        ("r20", "remastered,shadowReach=20"),
        ("r12", "remastered,shadowReach=12"),
        ("s075r20", "remastered,sunSize=0.75,shadowReach=20"),
        ("dbg_lb4p5", "remastered,debug=1,leafBias=4.5"),
        ("dbg_lb6", "remastered,debug=1,leafBias=6"),
        ("lb4p5", "remastered,leafBias=4.5"),
    ])
    lines += sweep(hero, "dev_woods", 4400, [
        ("off", "remastered,treeShadows=0,softShadows=0"),
        ("r40", "remastered,shadowReach=40"),
        ("r20", "remastered,shadowReach=20"),
        ("r12", "remastered,shadowReach=12"),
        ("s075r20", "remastered,sunSize=0.75,shadowReach=20"),
        ("s053", "remastered,sunSize=0.53"),
        ("dbg_lb4p5", "remastered,debug=1,leafBias=4.5"),
    ])
else:
    lines += sweep(forest, "dev_forest", 4000, [
        ("dbg_lb0", "remastered,debug=1,leafBias=0"),
        ("dbg_lb1p5", "remastered,debug=1,leafBias=1.5"),
        ("dbg_lb3", "remastered,debug=1"),
        ("dbg_lb6", "remastered,debug=1,leafBias=6"),
        ("dbg_off", "remastered,debug=1,treeShadows=0,softShadows=0"),
        ("lb0", "remastered,leafBias=0"),
        ("on", "remastered"),
        ("lb6", "remastered,leafBias=6"),
        ("notaa_on", "remastered,taa=0"),
        ("notaa_off", "remastered,taa=0,treeShadows=0,softShadows=0"),
        ("sun053", "remastered,sunSize=0.53"),
        ("sun2", "remastered,sunSize=2"),
    ])
    lines += sweep(hero, "dev_woods", 4400, [
        ("dbg_lb0", "remastered,debug=1,leafBias=0"),
        ("dbg_lb3", "remastered,debug=1"),
        ("dbg_lb6", "remastered,debug=1,leafBias=6"),
        ("off", "remastered,treeShadows=0,softShadows=0"),
        ("on", "remastered"),
        ("notaa_on", "remastered,taa=0"),
    ])
lines.append('\ncall { "view=done;event=done" call capLog }')
lines.append("triEndTest")
out = HERE / "tests"
name = "b1dev2" if os.environ.get("B1DEV") == "2" else "b1dev"
capgen.write(out / f"{name}.test.sqf", "\n".join(lines) + "\n")
capgen.write(out / f"{name}.test.toml", capgen.toml("Abel", "B1 tuning", views.WORLDS, timeout=3600))
print(out / f"{name}.test.sqf")
