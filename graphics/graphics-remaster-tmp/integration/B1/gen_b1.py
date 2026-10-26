"""B1 lane capture tests (light gate items 5, 6 and 8 for the shadows step).

    python gen_b1.py

Writes, next to a copy of capture_lib.sqf:

  tests/b1lane.test.sqf   one Abel game, each view shot off ("remastered,
                          treeShadows=0,softShadows=0") then on ("remastered"),
                          then both again under debug=1 (sun visibility):
      shadows_woods_pine2_evening  the features_ab shadows hero block,
                                   unchanged (sim clock 4400, cloud pin)
      b1_forest_edge_golden        18:15, sun about 12 deg: the soldier's
                                   shadow at his boots, long tree shadows
      b1_town_golden               18:15 in Saint Louis: house shadows
      b1_lowsun_fp                 08:00 in March: Jeep, M113, soldier, tent
      b1_woods_mixed_noon          14:00: dappled floor under a high sun
  tests/b1sinai.test.sqf  one Sinai game with @LoBo: sinai_lowsun_fp off and on
                          plus debug=1 (the critic's foreground slab check of the
                          tight near tiers)

On the base build the two keys are registered but inert, so off and on draw the
same frame; that build's shots are the BEFORE images.
"""
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = Path(r"D:\Arma_CWA_worktrees\gr-B1\arma_CWR\tests\rendering\captures")
sys.path.insert(0, str(CAPTURES))
import capgen  # noqa: E402
import views  # noqa: E402

OFF = "remastered,treeShadows=0,softShadows=0"
ON = "remastered"


def block(v, name, sim_time, debug=True, each=False):
    out = [f"\n// ---- {name} (off = {OFF}; on = {ON})"]
    out += capgen.begin(v, name, sim_time=sim_time)
    out.append("triSimFrames 150")
    out += capgen.shot(v, name, OFF, f"{name}__off")
    out += capgen.shot(v, name, ON, f"{name}__on")
    if each:
        # Each switch alone (tree shadows with the stock filter; the soft
        # filter without the leaf lift and the always-cast rule), and the soft
        # filter's path without TAA.
        out += capgen.shot(v, name, "remastered,softShadows=0", f"{name}__treeonly")
        out += capgen.shot(v, name, "remastered,treeShadows=0", f"{name}__softonly")
        out += capgen.shot(v, name, "remastered,taa=0", f"{name}__notaa")
    if debug:
        out += capgen.shot(v, name, OFF + ",debug=1", f"{name}__dbgoff")
        out += capgen.shot(v, name, ON + ",debug=1", f"{name}__dbgon")
    out += capgen.end(v)
    return out


def abel():
    hero = [h for h in views.HERO if h["name"] == "shadows_woods_pine2_evening"][0]
    forest = views._timing("forest_edge_golden")
    low = views._timing("lowsun_fp")
    town = dict(name="town_golden", x=7152, z=8915, eye=1.75, h=345, p=2, date=[1985, 7, 5, 18, 15], vd=1500)
    noon = dict(name="woods_mixed_noon", x=2864, z=6656, eye=1.75, h=300, p=-2, date=[1985, 7, 5, 14, 0], vd=1500)
    for v in (forest, low):
        v.pop("f10", None)
        v.pop("note", None)
    lines = [capgen.header("Abel", hero, "B1 lane: shadows hero, forest edge, town, lowsun_fp, noon woods.")]
    # features_ab gives the shadows hero view sim clock 4400 (SIM_T0 + 4 * SIM_DT).
    lines += block(hero, "shadows_woods_pine2_evening", 4400, each=True)
    lines += block(forest, "b1_forest_edge_golden", 5000)
    lines += block(town, "b1_town_golden", 5600, debug=False)
    lines += block(low, "b1_lowsun_fp", 6200)
    lines += block(noon, "b1_woods_mixed_noon", 6800, debug=False)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests"
    capgen.write(out / "b1lane.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "b1lane.test.toml", capgen.toml("Abel", "B1 lane captures", views.WORLDS, timeout=3600))
    (out / "capture_lib.sqf").write_bytes((CAPTURES / "capture_lib.sqf").read_bytes())
    print(out / "b1lane.test.sqf")


def sinai():
    v = dict([x for x in views.TIMING_SINAI if x["name"] == "sinai_lowsun_fp"][0])
    lines = [capgen.header("Sinai", v, "B1 lane: the Sinai foreground slab check.")]
    lines += block(v, "b1_sinai_lowsun_fp", 2600)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests"
    capgen.write(out / "b1sinai.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "b1sinai.test.toml", capgen.toml("Sinai", "B1 lane Sinai", views.WORLDS, timeout=3600))
    print(out / "b1sinai.test.sqf")


def motion():
    import gen_video_tests as gv
    a = gv.A_WOODS
    clips = [
        # Still camera, consecutive frames: the soft filter turns its taps
        # every TAA frame; what the history leaves of that is frame-to-frame
        # flicker on a still view.
        dict(clip="b1still", feature="shadows", keys=["treeShadows", "softShadows"], world="Abel", mode="S",
             frames=16, date=[1985, 7, 5, 17, 30], vd=1500, stage=gv.stage_of(a), path=gv.line(a, a),
             props=['[["Jeep"], 15, 2, 70, "veh"] call capSpawn;']),
        # A walk into the woods at 0.08 m per frame (2.5 m/s at 30 fps): crawl
        # of the per-pixel tap pattern and of the leaf shadows.
        dict(clip="b1walk", feature="shadows", keys=["treeShadows", "softShadows"], world="Abel", mode="S",
             frames=30, date=[1985, 7, 5, 17, 30], vd=1500, stage=gv.stage_of(a),
             path=gv.line(a, gv.along(a, 0.08 * 29)), props=['[["Jeep"], 15, 2, 70, "veh"] call capSpawn;']),
    ]
    title = "B1 lane motion: still and walking woods, off pass then on pass (S mode)"
    lines = [capgen.header("Abel", clips[0]["stage"], title, include="../capture_lib.sqf",
                           extra_includes=("capture_motion.sqf",))]
    lines.append("capEase = { _this }")
    for c in clips:
        lines += [f"\n// ---- {c['clip']}"]
        lines += gv.stage_block(c, c["frames"])
        lines += gv.s_mode(c, c["frames"])
        lines += capgen.end(c)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests" / "video"
    out.mkdir(parents=True, exist_ok=True)
    capgen.write(out / "b1motion.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "b1motion.test.toml", capgen.toml("Abel", title, views.WORLDS, timeout=3600, assert_timeout=180))
    (out / "capture_motion.sqf").write_bytes((CAPTURES / "video" / "capture_motion.sqf").read_bytes())
    print(out / "b1motion.test.sqf")


sys.path.insert(0, str(CAPTURES / "video"))
abel()
sinai()
motion()
