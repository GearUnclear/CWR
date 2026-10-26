"""A5 lane: a sweep of the TAA reconstruction width and sharpen on the hero view
(the temporary taaKernel knob), one Abel game, the off shot first.

    python gen_sweep.py
"""
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = Path(r"D:\Arma_CWA_worktrees\gr-A5\arma_CWR\tests\rendering\captures")
sys.path.insert(0, str(CAPTURES))
import capgen  # noqa: E402
import views  # noqa: E402

VARIANTS = [
    ("off", "remastered,taa=0"),
    ("k229s050", "remastered"),
    ("k4s050", "remastered,taaKernel=4"),
    ("k8s050", "remastered,taaKernel=8"),
    ("k4s075", "remastered,taaKernel=4,taaSharpen=0.75"),
    ("k8s075", "remastered,taaKernel=8,taaSharpen=0.75"),
    ("k229s100", "remastered,taaSharpen=1"),
    ("k4s050m10", "remastered,taaKernel=4,taaMipBias=-1"),
]


def main():
    hero = [h for h in views.HERO if h["name"] == "temporal_forest_edge_golden"][0]
    lines = [capgen.header("Abel", hero, "A5 lane: TAA reconstruction and sharpen sweep on the hero view.")]
    name = "a5sweep_forest"
    lines += capgen.begin(hero, name, sim_time=2600)
    lines.append("triSimFrames 150")
    for label, setting in VARIANTS:
        lines += capgen.shot(hero, name, setting, f"{name}__{label}")
    lines += capgen.end(hero)
    low = views._timing("lowsun_fp")
    low.pop("f10", None)
    low.pop("note", None)
    name = "a5sweep_lowsun"
    lines += capgen.begin(low, name, sim_time=3200)
    lines.append("triSimFrames 150")
    for label, setting in VARIANTS:
        lines += capgen.shot(low, name, setting, f"{name}__{label}")
    lines += capgen.end(low)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests"
    capgen.write(out / "a5sweep.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "a5sweep.test.toml", capgen.toml("Abel", "A5 lane sweep", views.WORLDS, timeout=3600))
    print(out / "a5sweep.test.sqf")


main()
