"""B5 single timing reading: the sea's cost, off against on in the same view and
round (light gate item 7).

    python gen_b5_timing.py [--rounds 3]

Writes tests/b5timing.test.sqf / .toml next to the lane's copy of
capture_lib.sqf. Same rig as gen_timing.py (1920x1080, scale 1.0, 4x MSAA,
presented frame). Views: the water hero (coastline_sunset, eye 3 m ASL), the
noon sea (eye 12 m, p -12: the frame is mostly sea) and the mid-sun shore (eye
15 m, p -22: all sea). Each round shoots "remastered,seaWaves=0" then
"remastered" (labels <view>_r<k>__off / __on, which make_pairs.py and
summarize_timing.py read). The sea's draws are in the world pass's ground
section (WorldBegin to GroundEnd: sky, terrain, water, horizon).
"""
import argparse
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = Path(r"D:\Arma_CWA_worktrees\gr-B5\arma_CWR\tests\rendering\captures")
sys.path.insert(0, str(CAPTURES))
import capgen  # noqa: E402
import views  # noqa: E402

ap = argparse.ArgumentParser()
ap.add_argument("--rounds", type=int, default=3)
a = ap.parse_args()

hero = dict([h for h in views.HERO if h["name"] == "water_coastline_sunset"][0])
hero["name"] = "coastline_sunset_sea"
noon = views._timing("sea_noon")
noon.pop("note", None)
shore = dict(name="shore_midsun", x=1550, z=5150, eye=15, h=278, p=-22, date=[1985, 7, 5, 16, 30], vd=4000)
VIEWS = [hero, noon, shore]

lines = [capgen.header("Abel", VIEWS[0], "B5 sea timing: seaWaves off then on, interleaved rounds.")]
for rnd in range(1, a.rounds + 1):
    for i, v in enumerate(VIEWS):
        label = f"{v['name']}_r{rnd}"
        lines.append(f"\n// ---- {label}")
        lines += capgen.begin(v, label, sim_time=2000 + 600 * (3 * (rnd - 1) + i))
        lines.append("triSimFrames 150")
        lines += capgen.shot(v, label, "remastered,seaWaves=0", f"{label}__off")
        lines += capgen.shot(v, label, "remastered", f"{label}__on")
        lines += capgen.end(v)
lines.append('\ncall { "view=done;event=done" call capLog }')
lines.append("triEndTest")
out = HERE / "tests"
out.mkdir(exist_ok=True)
(out / "capture_lib.sqf").write_bytes((CAPTURES / "capture_lib.sqf").read_bytes())
capgen.write(out / "b5timing.test.sqf", "\n".join(lines) + "\n")
capgen.write(out / "b5timing.test.toml", capgen.toml("Abel", "B5 sea timing", views.WORLDS, timeout=3600))
print(f"b5timing: {len(VIEWS)} views x {a.rounds} rounds")
