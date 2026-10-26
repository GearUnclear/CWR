"""B11 lane capture tests (light gate items 1, 5, 6, 7 and 8 for the texsharp step).

    python gen_lane.py

Writes, next to copies of capture_lib.sqf and video/capture_motion.sqf:

  tests/b11lane.test.sqf   one Abel game, every pair shot in that game, off first,
                           then on:
      texsharp_forest_edge   the features_ab texsharp hero block (sim clock 11000,
                             cloud pin), off = remastered,terrainDetail=0,sharpen=0
      b11_sharpen_forest_edge the same view with TAA off, where the sharpen
                             switch runs: off = remastered,taa=0,sharpen=0,
                             on = remastered,taa=0
      b11_noon_fields        the noon hillside with the Jeep and the soldier
      b11_coastline_sunset   the sea and the beach (the sea is drawn by the terrain
                             detail shader until B5: it must not change)
      b11_lowsun_fp          near gravel and the M113 set in the morning
      cost rounds            3 interleaved rounds of off / on at the hero view and
                             noon_fields (capReadTimers per shot)
  tests/b11eden.test.sqf   one Eden game: the overcast meadow (overcast_broken,
                           Jeep at 12 m), off and on
  tests/video/b11motion.test.sqf  one Abel game, S mode: the texsharp walk of
                           plan 7.3 (0.25 m per frame along heading 75 from the
                           hero view), 40 frames off, then 40 frames on

On a build before B11 the terrainDetail and sharpen keys are registered but
inert, so off and on draw the same frame; that build's shots are the BEFORE
images.
"""
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
CAPTURES = Path(r"D:\Arma_CWA_worktrees\gr-B11\arma_CWR\tests\rendering\captures")
sys.path.insert(0, str(CAPTURES))
sys.path.insert(0, str(CAPTURES / "video"))
import capgen  # noqa: E402
import views  # noqa: E402
import gen_video_tests as gv  # noqa: E402

OFF = "remastered,terrainDetail=0,sharpen=0"


def pair(v, name, sim_time, off=OFF, on="remastered"):
    out = [f"\n// ---- {name} (off = {off}; on = {on})"]
    out += capgen.begin(v, name, sim_time=sim_time)
    out.append("triSimFrames 150")
    out += capgen.shot(v, name, off, f"{name}__off")
    out += capgen.shot(v, name, on, f"{name}__on")
    out += capgen.end(v)
    return out


def cost(v, name, sim_time, rounds=3):
    out = [f"\n// ---- {name}: {rounds} interleaved rounds off / on"]
    out += capgen.begin(v, name, sim_time=sim_time)
    out.append("triSimFrames 150")
    for r in range(rounds):
        out += capgen.shot(v, name, OFF, f"{name}__r{r}_off")
        out += capgen.shot(v, name, "remastered", f"{name}__r{r}_on")
    out += capgen.end(v)
    return out


def timing_view(name):
    v = views._timing(name)
    v.pop("f10", None)
    v.pop("note", None)
    return v


def stills():
    hero = [h for h in views.HERO if h["name"] == "texsharp_forest_edge"][0]
    noon = timing_view("noon_fields")
    coast = timing_view("coastline_sunset")
    low = timing_view("lowsun_fp")
    lines = [capgen.header("Abel", hero, "B11 lane: terrain detail hero, sharpen, noon, sea, near gravel, cost.")]
    # features_ab gives the texsharp hero view sim clock 11000.
    lines += pair(hero, "texsharp_forest_edge", 11000)
    lines += pair(hero, "b11_sharpen_forest_edge", 11600, off="remastered,taa=0,sharpen=0", on="remastered,taa=0")
    lines += pair(noon, "b11_noon_fields", 12200)
    lines += pair(coast, "b11_coastline_sunset", 12800)
    lines += pair(low, "b11_lowsun_fp", 13400)
    lines += cost(hero, "b11cost_forest", 14000)
    lines += cost(noon, "b11cost_noon", 14600)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests"
    capgen.write(out / "b11lane.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "b11lane.test.toml", capgen.toml("Abel", "B11 lane captures", views.WORLDS, timeout=3600))
    (out / "capture_lib.sqf").write_bytes((CAPTURES / "capture_lib.sqf").read_bytes())
    print(out / "b11lane.test.sqf")


def eden():
    v = dict([x for x in views.TIMING_EDEN if x["name"] == "overcast_broken"][0])
    lines = [capgen.header("Eden", v, "B11 lane: the overcast meadow on Eden.")]
    lines += pair(v, "b11_overcast_broken", 2600)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests"
    capgen.write(out / "b11eden.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "b11eden.test.toml", capgen.toml("Eden", "B11 lane Eden", views.WORLDS, timeout=3600))
    print(out / "b11eden.test.sqf")


def motion():
    n = 40
    a = [4674, 10156, 1.75, 75, 0]
    clip = dict(clip="b11walk", feature="texsharp", keys=["terrainDetail", "sharpen"], world="Abel", mode="S",
                frames=n, date=[1985, 7, 5, 18, 15], vd=1500, stage=gv.stage_of(a),
                path=gv.line(a, gv.along(a, 0.25 * (n - 1))))
    title = "B11 lane motion: the texsharp walk, 0.25 m per frame (S mode, off pass then on pass)"
    lines = [capgen.header("Abel", clip["stage"], title, include="../capture_lib.sqf",
                           extra_includes=("capture_motion.sqf",))]
    # Constant speed (capture_motion.sqf eases with a smoothstep by default).
    lines.append("capEase = { _this }")
    lines += [f"\n// ---- {clip['clip']}"]
    lines += gv.stage_block(clip, n)
    lines += gv.s_mode(clip, n)
    lines += capgen.end(clip)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests" / "video"
    capgen.write(out / "b11motion.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "b11motion.test.toml",
                 capgen.toml("Abel", title, views.WORLDS, timeout=3600, assert_timeout=180))
    (out / "capture_motion.sqf").write_bytes((CAPTURES / "video" / "capture_motion.sqf").read_bytes())
    print(out / "b11motion.test.sqf")


def offcheck():
    """The deterministic off-equals-before check: with TAA off a shot repeats to
    the pixel (the before run's b11_sharpen_forest_edge pair: 0 pixels apart),
    while with TAA on the jitter phase at the shot differs shot to shot. Same
    view, label and sim clock as b11_sharpen_forest_edge, three shots:
    __base (taa=0, terrainDetail=0, sharpen=0: against the before run's __on),
    __detail (taa=0, sharpen=0: the terrain detail alone), __on (taa=0: the
    terrain detail and the luma sharpen)."""
    hero = [h for h in views.HERO if h["name"] == "texsharp_forest_edge"][0]
    name = "b11_sharpen_forest_edge"
    lines = [capgen.header("Abel", hero, "B11 lane: deterministic off check with TAA off.")]
    lines.append(f"\n// ---- {name}: base, detail only, detail and sharpen (TAA off)")
    lines += capgen.begin(hero, name, sim_time=11600)
    lines.append("triSimFrames 150")
    lines += capgen.shot(hero, name, "remastered,taa=0,terrainDetail=0,sharpen=0", f"{name}__base")
    lines += capgen.shot(hero, name, "remastered,taa=0,sharpen=0", f"{name}__detail")
    lines += capgen.shot(hero, name, "remastered,taa=0", f"{name}__on")
    lines += capgen.end(hero)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = HERE / "tests"
    capgen.write(out / "b11offcheck.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "b11offcheck.test.toml", capgen.toml("Abel", "B11 off check", views.WORLDS, timeout=3600))
    print(out / "b11offcheck.test.sqf")


stills()
eden()
motion()
offcheck()
