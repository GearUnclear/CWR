"""A5 lane capture tests (light gate items 1, 5, 6 and 8 for the TAA step).

    python gen_lane.py

Writes, next to copies of capture_lib.sqf and video/capture_motion.sqf:

  tests/a5lane.test.sqf    one Abel game, every pair shot in that game, off
                           ("remastered,taa=0") first, then on ("remastered"):
      temporal_forest_edge_golden  the features_ab temporal hero block,
                                   unchanged (sim clock 2600, cloud pin)
      a5_lowsun_fp                 the M113 antennas and the camouflage-net
                                   stand-in (thin geometry under TAA + MSAA)
      a5_town_night                lamps and the inflamed Fire (the particle
                                   mask; fire and smoke differ shot to shot)
  tests/video/a5motion.test.sqf   one Abel game, two S-mode clips (off pass,
                                   then on pass, as gen_video_tests.py writes):
      a5strafe   forest_edge_golden, a 1.2 m strafe right over 40 frames with
                 the soldier at 12 m and a Jeep teleported across at 25 m:
                 0.12 m per frame for 20 frames, then 0.5 m per frame (the
                 ghost-trail check of plan 5 A5 and 8)
      a5pan      forest_edge_golden, a yaw pan of 2 px per frame at the centre
                 (atan(2 / 1371.4) deg, 1920 px wide at fov 0.7) over 40
                 frames: the frame-to-frame residual of the crowns after an
                 integer-shift registration ("steadier crowns")

  tests/a5sinai.test.sqf   one Sinai game with @LoBo: sinai_lowsun_fp off and on
                           (the plan's sharpen check view: net and antennas)

On a build before A5 the taa key is registered but inert, so off and on draw the
same frame; that build's shots are the BEFORE images.
"""
import math
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = Path(r"D:\Arma_CWA_worktrees\gr-A5\arma_CWR\tests\rendering\captures")
sys.path.insert(0, str(CAPTURES))
sys.path.insert(0, str(CAPTURES / "video"))
import capgen  # noqa: E402
import views  # noqa: E402
import gen_video_tests as gv  # noqa: E402

OFF = "remastered,taa=0"


def pair(v, name, sim_time):
    out = [f"\n// ---- {name} (off = {OFF}; on = remastered)"]
    out += capgen.begin(v, name, sim_time=sim_time)
    out.append("triSimFrames 150")
    out += capgen.shot(v, name, OFF, f"{name}__off")
    out += capgen.shot(v, name, "remastered", f"{name}__on")
    out += capgen.end(v)
    return out


def stills():
    hero = [h for h in views.HERO if h["name"] == "temporal_forest_edge_golden"][0]
    low = views._timing("lowsun_fp")
    town = views._timing("town_night")
    for v in (low, town):
        v.pop("f10", None)
        v.pop("note", None)
    lines = [capgen.header("Abel", hero, "A5 lane: TAA hero, antennas, town night fire.")]
    # features_ab gives the temporal hero view sim clock 2600 (SIM_T0 + 1 * SIM_DT).
    lines += pair(hero, "temporal_forest_edge_golden", 2600)
    lines += pair(low, "a5_lowsun_fp", 3200)
    lines += pair(town, "a5_town_night", 3800)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests"
    capgen.write(out / "a5lane.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "a5lane.test.toml", capgen.toml("Abel", "A5 lane captures", views.WORLDS, timeout=3600))
    (out / "capture_lib.sqf").write_bytes((CAPTURES / "capture_lib.sqf").read_bytes())
    print(out / "a5lane.test.sqf")


def motion():
    a = gv.A_FOREST
    n = 40
    # 1920 px across at fov 0.7 (capFov): focal length 960 / 0.7 px.
    pan_deg = math.degrees(math.atan(2.0 / (960.0 / 0.7)))
    clips = [
        dict(clip="a5strafe", feature="temporal", keys=["taa"], world="Abel", mode="S", frames=n,
             date=[1985, 7, 5, 18, 15], vd=1500, stage=gv.stage_of(a),
             path=gv.line(a, gv.strafe(a, 0.03 * (n - 1))),
             props=['[["SoldierWB"], 12, 2, 215, "man"] call capSpawn;',
                    'capMover1 = [["Jeep"], 25, -12, 90, "veh"] call capSpawn;'],
             movers=[("capMover1", [[0, 25, -12, 90], [20, 25, -9.6, 90], [39, 25, -0.1, 90]])]),
        dict(clip="a5pan", feature="temporal", keys=["taa"], world="Abel", mode="S", frames=n,
             date=[1985, 7, 5, 18, 15], vd=1500, stage=gv.stage_of(a),
             path=gv.line(a, [a[0], a[1], a[2], round(a[3] + pan_deg * (n - 1), 4), a[4]]),
             props=['[["SoldierWB"], 12, 2, 215, "man"] call capSpawn;']),
        # The particle mask: a still camera on the town's inflamed Fire at 9 m,
        # consecutive frames (the flames and smoke move, the camera does not).
        dict(clip="a5fire", feature="temporal", keys=["taa"], world="Abel", mode="S", frames=24,
             date=[1985, 2, 27, 22, 0], vd=800, stage=gv.stage_of(gv.A_TOWN), path=gv.line(gv.A_TOWN, gv.A_TOWN),
             props=views.TOWN_FIRE, refresh=views.FIRE_REFRESH),
    ]
    first = clips[0]["stage"]
    title = "A5 lane motion: strafe with a crossing Jeep, yaw pan (S mode, off pass then on pass)"
    lines = [capgen.header("Abel", first, title, include="../capture_lib.sqf",
                           extra_includes=("capture_motion.sqf",))]
    # Constant speed along both paths (capture_motion.sqf eases with a
    # smoothstep): 0.03 m and 2 px per frame from the first frame to the last.
    lines.append("capEase = { _this }")
    for c in clips:
        lines += [f"\n// ---- {c['clip']}"]
        lines += gv.stage_block(c, c["frames"])
        lines += gv.s_mode(c, c["frames"])
        lines += capgen.end(c)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests" / "video"
    capgen.write(out / "a5motion.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "a5motion.test.toml",
                 capgen.toml("Abel", title, views.WORLDS, timeout=3600, assert_timeout=180))
    (out / "capture_motion.sqf").write_bytes((CAPTURES / "video" / "capture_motion.sqf").read_bytes())
    print(out / "a5motion.test.sqf", f"pan {pan_deg:.5f} deg per frame")


def sinai():
    # The plan's sharpen check view (5 A5 critic fixes): the camouflage net,
    # the HMMWV and M113 Zelda antennas on @LoBo content.
    v = [x for x in views.TIMING_SINAI if x["name"] == "sinai_lowsun_fp"][0]
    v = dict(v)
    lines = [capgen.header("Sinai", v, "A5 lane: the sharpen check view on Sinai.")]
    lines += pair(v, "a5_sinai_lowsun_fp", 2600)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests"
    capgen.write(out / "a5sinai.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "a5sinai.test.toml", capgen.toml("Sinai", "A5 lane Sinai", views.WORLDS, timeout=3600))
    print(out / "a5sinai.test.sqf")


stills()
motion()
sinai()
