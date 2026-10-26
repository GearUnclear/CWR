"""FINAL check scratch rig (not in the repo): what each step costs at the long
views, one step's switches off at a time, everything else on, in one game.

Modes per view: all on, then each step's keys off, then baseline (all off);
2 interleaved rounds (the order reverses on round 2). Writes
cost_abel.test.sqf/.toml next to this file with a copy of capture_lib.sqf.
"""
import shutil
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = HERE.parents[3] / "arma_CWR" / "tests" / "rendering" / "captures"
sys.path.insert(0, str(CAPTURES))

import capgen  # noqa: E402
import views  # noqa: E402

ROUNDS = 2
MODES = [
    ("on", "remastered"),
    ("A4", "remastered,drawGrouping=0,hillCull=0"),
    ("A5", "remastered,taa=0"),
    ("A7", "remastered,atmosphere=0"),
    ("B1", "remastered,treeShadows=0,softShadows=0"),
    ("B2", "remastered,lowSunRig=0,crownCanopy=0"),
    ("B5", "remastered,seaWaves=0"),
    ("B6", "remastered,ssr=0"),
    ("B8", "remastered,airBeams=0,airHaze=0,valleyMist=0"),
    ("B9", "remastered,canopyShadows=0"),
    ("B11", "remastered,terrainDetail=0,sharpen=0"),
    ("base", "baseline"),
]
VIEWS = ["valley_0515", "sunrise_ridge"]


def main():
    global MODES, ROUNDS
    name = 'cost_abel'
    if '--valley' in sys.argv:
        global VIEWS
        MODES = [m for m in MODES if m[0] in ('on', 'base')]
        ROUNDS = 2
        VIEWS = ['valley_0515']
        name = 'valley_abel'
    elif '--short' in sys.argv:
        MODES = [m for m in MODES if m[0] in ('on', 'base')]
        ROUNDS = 3
        name = 'short_abel'
    vs = [views._timing(n) for n in VIEWS]
    title = f"FINAL per-step cost at the long views, {ROUNDS} interleaved rounds."
    lines = [capgen.header("Abel", vs[0], title)]
    t = 1000
    for rnd in range(1, ROUNDS + 1):
        for v in vs:
            label = f"{v['name']}_r{rnd}"
            lines.append(f"\n// ---- {v['name']} round {rnd}")
            lines += capgen.begin(v, label, sim_time=t)
            t += 100
            lines.append("triSimFrames 150")
            modes = list(MODES)
            if rnd % 2 == 0:
                modes.reverse()
            for tag, mode in modes:
                lines += capgen.shot(v, label, mode, f"{label}__{tag}")
            lines += capgen.end(v)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    capgen.write(HERE / f"{name}.test.sqf", "\n".join(lines) + "\n")
    capgen.write(HERE / f"{name}.test.toml", capgen.toml("Abel", title, views.WORLDS, timeout=7200))
    shutil.copy(CAPTURES / "capture_lib.sqf", HERE / "capture_lib.sqf")
    print(f"{name}: {len(vs)} views x {len(MODES)} modes x {ROUNDS} rounds")


if __name__ == "__main__":
    main()


