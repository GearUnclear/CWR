"""B9 timing reading (light gate item 7): three interleaved rounds in one Abel
game, each round every view off then on, then a time-lapse step block that
rebuilds the sun-visibility map on every step (the canopy section).

    python gen_b9timing.py

Labels <view>_r<k>__off / __on, as summarize_timing.py groups them. Run with
run-timing.ps1 (GPU clock sampler) from the scratch copy.
"""
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = Path(r"D:\Arma_CWA_worktrees\gr-B9\arma_CWR\tests\rendering\captures")
sys.path.insert(0, str(CAPTURES))
import capgen  # noqa: E402
import views  # noqa: E402

OFF = "remastered,canopyShadows=0"
ON = "remastered"
ROUNDS = 3

base = dict(x=5500, z=6250, eye=15, h=185, p=-14, date=[1985, 9, 20, 17, 30], vd=2000)
VIEWS = [
    (dict(base, name="b9t_hill_1730"), None),
    (dict(base, name="b9t_slope_tele_1730", h=200, p=-12), 0.3),
    (dict(name="b9t_forest_edge_golden", x=4674, z=10156, eye=1.75, h=75, p=3, date=[1985, 7, 5, 18, 15],
          vd=1500), None),
    (dict(base, name="b9t_hill_noon", date=[1985, 9, 20, 12, 0]), None),
]


def view_block(v, fov, k, sim_time):
    label = f"{v['name']}_r{k}"
    out = [f"\n// ---- {label}"]
    if fov:
        out.append(f"capFov = {fov}")
    out += capgen.begin(v, label, sim_time=sim_time)
    out.append("triSimFrames 120")
    out += capgen.shot(v, label, OFF, f"{label}__off")
    out += capgen.shot(v, label, ON, f"{label}__on")
    out += capgen.end(v)
    if fov:
        out.append("capFov = 0.7")
    return out


def lapse_block(k, sim_time):
    v = dict(base, name="b9t_lapse", date=[1985, 9, 20, 17, 20])
    label = f"b9t_lapse_r{k}"
    out = [f"\n// ---- {label}: 20 one-minute steps, a map rebuild each"]
    out += capgen.begin(v, label, sim_time=sim_time)
    out.append("triSimFrames 120")
    out.append(f'"{ON}" call capMode')
    out.append("triSimFrames 30")
    out.append("call { capGpuOff = triGpuTimes false; capGpuOn = triGpuTimes true }")
    for m in range(20):
        out.append(f"call {{ capDate = [1985, 9, 20, 17, {20 + m}]; setDate capDate }}")
        out.append("triSimFrames 3")
    out.append("[] call capReadTimers")
    out.append("triSimFrames 1")
    out.append(f'triScreenshot "{label}__on"')
    out.append(f'"{label}__on" call capRecord')
    out += capgen.end(v)
    return out


def main():
    lines = [capgen.header("Abel", VIEWS[0][0], "B9 timing: hero, telephoto, forest edge, noon off/on, 3 rounds; "
                                               "time-lapse rebuilds.")]
    t = 20000
    for k in range(1, ROUNDS + 1):
        for v, fov in VIEWS:
            lines += view_block(v, fov, k, t)
            t += 600
        lines += lapse_block(k, t)
        t += 600
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests"
    capgen.write(out / "b9timing.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "b9timing.test.toml", capgen.toml("Abel", "B9 timing", views.WORLDS, timeout=3600))
    print(out / "b9timing.test.sqf")


main()
