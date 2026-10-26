"""Round-3/4 sheets for the normals track (research only).

    python analyze_r3.py <run_dir> [<run_dir> ...]

For every view in each run (after compare.py has collected shots/):
  <view>__grid.png     classic, as shipped, + late VB, V1, V2, V5, V7, debug (2 x 4)
  <view>__four.png     Classic | Remastered as shipped | + late VB | + normal map + micro shadows
  <view>__nm_crop.png  2x crops of the normal-map variants against the late-VB base
and a diffstat table (mean / p99 abs difference against the late-VB base).
"""
import subprocess
import sys
from pathlib import Path

import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
COMPARE = HERE / "compare.py"
RENAME = [
    "--rename", "remastered=Remastered + late-VB fix (no bump)",
    "--rename", "cur=Remastered (as shipped)",
    "--rename", "v7shad=+ normal map + micro shadows",
    "--rename", "v8cav=+ normal map + micro shadows + cavities",
    "--rename", "v9bold=+ all, bump and height x2",
    "--rename", "dbgview1=debug: sun visibility (V8)",
]
R4 = '--r4' in sys.argv
if R4:
    sys.argv.remove('--r4')
# A crop box per view that holds the most relief (960x640 frame).
CROPS = {
    "lowsun_fp": (0, 300, 480, 620),
    "hull_close": (380, 120, 860, 440),
    "noon_fields": (240, 320, 720, 640),
    "forest_edge_golden": (240, 180, 720, 500),
    "tank_gunner_interior": (240, 60, 720, 380),
    "sinai_lowsun_fp": (0, 300, 480, 620),
    "sinai_hull_close": (380, 120, 860, 440),
}


def run_compare(run, panels, name, extra=()):
    cmd = [sys.executable, str(COMPARE), str(run), "--panels", ",".join(panels), "--name", name, *RENAME, *extra]
    subprocess.run(cmd, check=True)


def diffstat(run):
    shots = run / "shots"
    lines = ["| view | label | mean abs diff vs late-VB base | p99 |", "|---|---|---|---|"]
    for base in sorted(shots.glob("*__remastered.png")):
        view = base.name.split("__")[0]
        b = np.asarray(Image.open(base).convert("RGB")).astype(np.float32)
        for p in sorted(shots.glob(f"{view}__*.png")):
            lab = p.stem.split("__")[1]
            if lab == "remastered":
                continue
            a = np.asarray(Image.open(p).convert("RGB")).astype(np.float32)
            d = np.abs(a - b).mean(axis=2)
            lines.append(f"| {view} | {lab} | {d.mean():.2f} | {np.percentile(d, 99):.1f} |")
    (run / "diffstat.md").write_text("\n".join(lines) + "\n")
    print("\n".join(lines))


def main():
    for r in sys.argv[1:]:
        run = Path(r)
        if R4:
            run_compare(run, ["classic", "cur", "remastered", "v2tex", "v5nmap", "v7shad", "v8cav", "v9bold", "dbgview1"], "grid", ["--cols", "3"])
            run_compare(run, ["classic", "cur", "v8cav", "v9bold"], "four", ["--cols", "2"])
        else:
            run_compare(run, ["classic", "cur", "remastered", "v1ss", "v2tex", "v5nmap", "v7shad", "dbgnormal"], "grid", ["--cols", "4"])
            run_compare(run, ["classic", "cur", "remastered", "v7shad"], "four", ["--cols", "2"])
        views = sorted({p.name.split("__")[0] for p in (run / "shots").glob("*.png")})
        for v in views:
            if v in CROPS:
                box = ",".join(str(x) for x in CROPS[v])
                crop_panels = ["remastered", "v5nmap", "v7shad", "v8cav", "v9bold", "dbgview1"] if R4 else ["remastered", "v2tex", "v5nmap", "v6nmap2x", "v7shad", "v8shad2x"]
                run_compare(run, crop_panels, "nm_crop",
                            ["--views", v, "--crop", box, "--cols", "3", "--scale", "1.5"])
        diffstat(run)


if __name__ == "__main__":
    main()
