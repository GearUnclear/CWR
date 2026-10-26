"""B9 lane capture tests (light gate items 1, 5, 6, 7 and 8 for the hfrt step).

    python gen_b9.py

Writes, next to a copy of capture_lib.sqf:

  tests/b9lane.test.sqf   one Abel game, each view shot off ("remastered,
                          canopyShadows=0") then on ("remastered"), then both
                          again under debug=1 (sun visibility):
      hfrt_hill_1730       the features_ab hfrt hero block, unchanged (sim
                           clock 9800, cloud pin)
      b9_slope_tele_1730   the telephoto (fov 0.3) onto the wooded slope SSW of
                           the summit: the research's +13 to 16 ms worst case
      b9_ground_b_1730     eye 2.5 m among the summit trees: the near range,
                           where the shadow maps own the tree shadows
      b9_forest_edge_golden 18:15 in July, sun about 12 deg, VD 1500
      b9_hill_0700         the extra time view: the same vantage on an October
                           morning, sun about 10 deg from the left
      b9_hill_1630         an hour before the hero, sun about 17 deg
      b9_hill_noon         12:00: the gate holds the feature off
  then a short time-lapse on the hero vantage (17:20 to 17:39, one minute per
  step, the sun moves about 0.2 deg a step), so the canopy section reads the
  visibility map's rebuild on every step.

On the base build the key is registered but inert, so off and on draw the same
frame; that build's shots are the BEFORE images.
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


# triCanopyCheck after each on shot (the B9 build only: the verb is new).
CHECK = False


def block(v, name, sim_time, debug=True, fov=None):
    out = [f"\n// ---- {name} (off = {OFF}; on = {ON})"]
    if fov:
        out.append(f"capFov = {fov}")
    out += capgen.begin(v, name, sim_time=sim_time)
    out.append("triSimFrames 150")
    out += capgen.shot(v, name, OFF, f"{name}__off")
    out += capgen.shot(v, name, ON, f"{name}__on")
    if CHECK:
        out.append('call { capCheck = triCanopyCheck 4096; format ["view=%1;event=canopy_check;res=%2", '
                   'capView, capCheck] call capLog }')
    if debug:
        out += capgen.shot(v, name, OFF + ",debug=1", f"{name}__dbgoff")
        out += capgen.shot(v, name, ON + ",debug=1", f"{name}__dbgon")
    out += capgen.end(v)
    if fov:
        out.append("capFov = 0.7")
    return out


def timelapse(v, name, sim_time, minutes):
    """One minute of clock per step at the hero vantage, three frames each:
    every step moves the sun past the visibility map's rebuild threshold."""
    out = [f"\n// ---- {name}: time-lapse, one minute per step ({minutes} steps)"]
    out += capgen.begin(v, name, sim_time=sim_time)
    out.append("triSimFrames 150")
    out.append(f'"{ON}" call capMode')
    out.append("triSimFrames 30")
    out.append("call { capGpuOff = triGpuTimes false; capGpuOn = triGpuTimes true }")
    d = v["date"]
    for k in range(minutes):
        out.append(f"call {{ capDate = [{d[0]}, {d[1]}, {d[2]}, {d[3]}, {d[4] + k}]; setDate capDate }}")
        out.append("triSimFrames 3")
    out.append("[] call capReadTimers")
    out.append("triSimFrames 1")
    out.append(f'triScreenshot "{name}__on"')
    out.append(f'"{name}__on" call capRecord')
    out += capgen.end(v)
    return out


def abel(check=False, stem="b9lane"):
    global CHECK
    CHECK = check
    hero = [h for h in views.HERO if h["name"] == "hfrt_hill_1730"][0]
    base = dict(x=5500, z=6250, eye=15, h=185, p=-14, date=[1985, 9, 20, 17, 30], vd=2000)
    slope = dict(base, name="slope_tele_1730", h=200, p=-12)
    ground_b = dict(base, name="ground_b_1730", x=5440, z=6200, eye=2.5, h=180, p=-8)
    forest = dict(name="forest_edge_golden", x=4674, z=10156, eye=1.75, h=75, p=3, date=[1985, 7, 5, 18, 15],
                  vd=1500)
    morning = dict(base, name="hill_0700", date=[1985, 10, 10, 7, 0])
    earlier = dict(base, name="hill_1630", date=[1985, 9, 20, 16, 30])
    noon = dict(base, name="hill_noon", date=[1985, 9, 20, 12, 0])
    lapse = dict(base, name="hill_lapse", date=[1985, 9, 20, 17, 20])
    lines = [capgen.header("Abel", hero, "B9 lane: hfrt hero, telephoto slope, ground among trees, forest edge, "
                                         "morning, earlier, noon, time-lapse.")]
    # features_ab gives the hfrt hero view sim clock 9800 (SIM_T0 + 13 * SIM_DT).
    lines += block(hero, "hfrt_hill_1730", 9800)
    lines += block(slope, "b9_slope_tele_1730", 10400, fov=0.3)
    lines += block(ground_b, "b9_ground_b_1730", 11000)
    lines += block(forest, "b9_forest_edge_golden", 11600)
    lines += block(morning, "b9_hill_0700", 12200)
    lines += block(earlier, "b9_hill_1630", 12800, debug=False)
    lines += block(noon, "b9_hill_noon", 13400, debug=False)
    lines += timelapse(lapse, "b9_hill_lapse", 14000, 20)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests"
    capgen.write(out / f"{stem}.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / f"{stem}.test.toml", capgen.toml("Abel", "B9 lane captures", views.WORLDS, timeout=3600))
    (out / "capture_lib.sqf").write_bytes((CAPTURES / "capture_lib.sqf").read_bytes())
    print(out / f"{stem}.test.sqf")


abel()
abel(check=True, stem="b9new")
