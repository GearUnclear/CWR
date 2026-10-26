"""A5 lane: where TAA's cost goes, one Abel game, 3 interleaved rounds per view.

    python gen_cost.py

Variants per round (each a settled, timed shot of capgen.shot): off (taa=0),
on, on without the texture LOD bias (taaMipBias=0), on without the opaque copy
of the particle mask (the temporary taaMaskDbg=0 knob of the cost build).
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
    ("on", "remastered"),
    ("mip0", "remastered,taaMipBias=0"),
    ("nomask", "remastered,taaMaskDbg=0"),
]


def main():
    hero = [h for h in views.HERO if h["name"] == "temporal_forest_edge_golden"][0]
    low = views._timing("lowsun_fp")
    town = views._timing("town_night")
    for v in (low, town):
        v.pop("f10", None)
        v.pop("note", None)
    lines = [capgen.header("Abel", hero, "A5 lane: TAA cost split, 3 interleaved rounds.")]
    for name, v, simt in (("a5cost_forest", hero, 2600), ("a5cost_lowsun", low, 3200), ("a5cost_town", town, 3800)):
        lines += capgen.begin(v, name, sim_time=simt)
        lines.append("triSimFrames 150")
        for r in range(3):
            for label, setting in VARIANTS:
                lines += capgen.shot(v, name, setting, f"{name}__r{r}_{label}")
        lines += capgen.end(v)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests"
    capgen.write(out / "a5cost.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "a5cost.test.toml", capgen.toml("Abel", "A5 lane cost split", views.WORLDS, timeout=3600))
    print(out / "a5cost.test.sqf")


main()
