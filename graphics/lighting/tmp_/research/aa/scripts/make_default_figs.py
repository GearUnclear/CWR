"""What a player sees: the shipped defaults (no MSAA, 1.0x) against the
proposed Remastered default (4x MSAA + alpha-to-coverage, 1.5x, cutout
coverage j3, Lanczos2 + RCAS 0.5 present). Built from run_v3 captures.

    python make_default_figs.py <run_dir> <out_dir>
"""
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).parent
run = Path(sys.argv[1])
out = Path(sys.argv[2])
T = str(run / "timings.csv")
CROPS = {
    "forest_edge_golden": "560,20,960,320",
    "overcast_broken": "560,240,960,400",
    "town_night": "300,40,700,300",
    "sinai_lowsun_fp": "100,180,600,420",
}
for view, crop in CROPS.items():
    best = "H-best2" if view == "overcast_broken" else "H-best3"
    items = [(f"{view}__D-classic.png", "Classic, shipped default (no AA)"),
             (f"{view}__D-rem.png", "Remastered, shipped default (no AA)"),
             (f"{view}__{best}.png", "Remastered, proposed default (4x MSAA 1.5x + aa prototype)")]
    for name, extra in ((f"PLAYER_DEFAULT_{view}.png", []), (f"PLAYER_DEFAULT_{view}_crop.png", ["--crop", crop, "--zoom", "2"])):
        args = [sys.executable, str(HERE / "compare.py"), str(out / name), "--title",
                f"{view}: what a player sees at the shipped default vs the proposed Remastered default", "--timings", T,
                "--note", "proposed = triSetMsaa 4 + triSetRenderScale 1.5 (A2C on) + j3 coverage + Lanczos2/RCAS 0.5 present (scratch build)"] + extra
        args += [f"{run / p}={lab}" for p, lab in items]
        subprocess.run(args, check=True)
print("done")
