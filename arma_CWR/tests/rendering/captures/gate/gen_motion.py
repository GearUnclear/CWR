"""Motion clip tests for the bench's `motion` set (PERF-PLAN P0-10, 3.3 T3, 3.6): one camera path
per clip, recorded at real speed by the P0-10 recorder (triRecord, every presented frame), once
per lighting mode. Writes one .test.sqf/.toml per island into a caller folder plus
motion.plan.json. Both sides of a job run the same files.

    python -I gen_motion.py --out <dir> [--clips c1,c2] [--modes remastered,classic]
                            [--has-reset] [--has-seed] [--has-deterministic]
                            [--fixed-settle N [--settle-wait-ms MS]]
                            [--timing-pass off|after|before] [--list]

Recipe per clip and mode (3.3's determinism recipe, adapted to a moving camera): stage the
view (capgen.begin: camera, date, weather, view distance, props, the sim clock pinned per clip,
the clouds held), settle in the mode (capMode: detail pinned, triSettleScene), re-pin
(capPin), [--has-deterministic] triDeterministic true, [--has-reset] triResetTemporal [0, 0]
(mist held), [--has-seed] triSeedRandom 1, then triCamPath (keys relative to the view's own
camera) and triRecord ["motion/<clip>__<label>.raw", frames, 0]; a top-level triRecordState
waits until the dump is written. The whole run steps the simulation by 1/60 s
(triSetFixedStep) with setAccTime 1, so the main loop renders exactly one frame per tick:
frame k of a clip shows the same simulated instant in every build and boot, and the camera
path advances by one fixed step per frame. Every mode of a clip starts from the same pinned
sim clock (capPin re-pins it), as P0-6's det1/det2 shots do.

[--fixed-settle N] (GATE-2, needs --has-deterministic): triDeterministic is turned on at setup
too, so the capture hold covers every clip's staging and settle, and capMode settles for exactly
N frames (gatelib.mode_lines) instead of until the draw count is steady: each clip starts at the
same simulated frame in both builds whatever their draw counts.

[--timing-pass after|before] (OVERLAY-1, needs triPerfRing, triGpuRing and triRecordState): after
(or before) each recorded clip, the same clip once more WITHOUT recording, for the owner clip's
frame-time graphs (the recorded pass's frame times are the recorder's): the same re-pin, refresh,
det, reset and seed lines, then ONE statement that switches GPU timing off and on, starts the same
triCamPath and runs the clip's frames plus TIMING_TAIL with triSimFrames in chunks of TIMING_CHUNK,
each chunk ending with triRecordState (pathT) and triPerfRing / triGpuRing [n, "tim_<label>_<k>"]
(event=timing lines; one statement, so no main-loop frame runs between chunks, held or not); then
GPU timing off and a re-pin. frametime.py extract aligns the rings with the video frames by path
step. off (the default) writes exactly the files it wrote before OVERLAY-1. "after" leaves every
recording as it was without the pass (each clip still starts from its own pin).

Labels: "remastered" is <clip>__today, any other mode <clip>__<mode> (classic: the
"Classic lighting on today's engine" row of the owner's three-way clip; never call it
vanilla).
"""
import argparse
import json
import os
import shutil
import sys
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gatelib  # noqa: E402
import capgen  # noqa: E402
import golden_views as gv  # noqa: E402

STEP = 1.0 / 60.0
FPS = 60
# Clips (3.6: 8 to 15 s, a move that exercises the item). Keys: [t, d_east, d_up, d_north,
# d_heading, d_pitch, ease] relative to the view's camera (metres, degrees); t in seconds.
CLIPS = {
    # the TAA foliage pan over the dawn valley (W1-1, W1-5i, TAA items): slow, eased
    "valley_foliage_pan": dict(view="valley_0515", keys=[(0, 0, 0, 0, 0, 0, 0), (10, 0, 0, 0, 14, 0.5, 1)], hold=0.5),
    # the dawn valley held camera (fog items)
    "valley_mist_hold": dict(view="valley_0515", keys=[(0, 0, 0, 0, 0, 0, 0)], hold=8.0),
    # a short walk toward the forest edge (LOD, foliage)
    "forest_edge_walk": dict(view="forest_edge_golden", keys=[(0, 0, 0, 0, 0, 0, 0), (12, 0, 0, 18, 0, 0, 1)], hold=0.5),
    # particles (W4-1 decal batching): the night combat staging (burning Ural 36 m ahead and 13 m
    # right, camp fire 21 m ahead and 12 m left, posed infantry), a slow pan left with a 2 degree
    # lift so both smoke columns cross the frame and overlap; at the end the Ural sits about 36
    # degrees right of centre, inside the 16:9 half width of about 43 at fov 0.7
    "combat_smoke_pan": dict(view="combat_fires", keys=[(0, 0, 0, 0, 0, 0, 0), (10, 0, 0, 0, -16, 2.0, 1)], hold=0.5),
    # particles in rain (W4-1): the wet town at night, the fire 9.5 m ahead (18 degrees right at
    # the start) and the lamps' flares; a pan right with a 2 m step east for parallax through the
    # smoke, ending with the fire about 6 degrees left of centre
    "town_rain_smoke_pan": dict(view="town_night_rain", keys=[(0, 0, 0, 0, 0, 0, 0), (9.5, 2.0, 0, 0, 12, 0, 1)], hold=0.5),
}
DEFAULT_CLIPS = ["valley_foliage_pan"]
# OVERLAY-1 timing pass: ring reads every TIMING_CHUNK frames (both rings hold 256), and TIMING_TAIL
# frames past the clip so the GPU ring (read back kGpuFrames = 4 frames late) reaches its last frame
TIMING_CHUNK = 240
TIMING_TAIL = 8
# the burning-vehicle refresh of golden_views.COMBAT (gen_golden.LIB): written into a world file only
# when one of its clips' views calls it
LIB_BURN = ("capBurn = { if (not (isNull capFire)) then { capFire inflame true }; "
            "if (not (isNull capBurnV)) then { capBurnV inflame true } }")


def clip_frames(c):
    return int(round((c["keys"][-1][0] + c["hold"]) * FPS))


def offset(v):
    # "+ 4" or "- 16": a binary minus, never "+ -16" (no unary minus right after an operator in the test SQF)
    return f"- {-v:g}" if v < 0 else f"+ {v:g}"


def path_expr(c):
    ks = []
    for k in c["keys"]:
        t, dx, dy, dz, dh, dp, ease = k
        # triCamPath keys: [t, east, upASL, north, heading, pitch, ease]; the view's own camera from capPin
        ks.append(f"[{t:g}, capOX {offset(dx)}, capEyeASL {offset(dy)}, capOZ {offset(dz)}, capH {offset(dh)}, capP {offset(dp)}, {ease}]")
    return "[" + ", ".join(ks) + "]"


def label_of(clip, mode):
    return f"{clip}__{'today' if mode == 'remastered' else mode.replace(',', '_').replace('=', '')}"


def timing_lines(c, lab):
    """The unrecorded timing pass of one clip (see the docstring): one statement."""
    n = clip_frames(c)
    total = n + TIMING_TAIL
    body = ['capTimGpu = triGpuTimes false', 'capTimGpu = triGpuTimes true', f'capPathRes = triCamPath {path_expr(c)}',
            f'format ["view=%1;event=timing_start;clip={lab};frames={n};tail={TIMING_TAIL};chunk={TIMING_CHUNK};'
            f'gpuon=%2;pathres=%3", capView, capTimGpu, capPathRes] call capLog']
    k, done = 0, 0
    while done < total:
        m = min(TIMING_CHUNK, total - done)
        tag = f"tim_{lab}_{k}"
        body += [f'triSimFrames {m}', 'capTimSt = triRecordState', f'capRing = triPerfRing [{m}, "{tag}"]',
                 f'capGRing = triGpuRing [{m}, "{tag}"]',
                 f'format ["view=%1;event=timing;clip={lab};chunk={k};n={m};pring=%2;gring=%3;state=%4", '
                 'capView, capRing, capGRing, capTimSt] call capLog']
        done += m
        k += 1
    body += ['capPathRes = triCamPath []', 'capTimGpu = triGpuTimes false',
             f'format ["view=%1;event=timing_done;clip={lab};frames={n};run={total};chunks={k}", capView] call capLog']
    return [f"// timing pass {lab}: {total} frames, not recorded", "call { " + "; ".join(body) + " }"]


def clip_block(name, c, v, mode, o):
    lab = label_of(name, mode)
    n = clip_frames(c)
    out = [f"\n// ---- {lab}: {n} frames"] + gatelib.mode_lines(mode, o.fixed_settle, o.settle_wait_ms) + ["triSimFrames 60", "[] call capPin"]
    prep = []
    if v.get("refresh"):
        prep.append(v["refresh"])
    if o.has_deterministic:
        prep.append('call { capDetRes = triDeterministic true; format ["view=%1;event=det;det=%2", capView, capDetRes] call capLog }')
    if o.has_reset:
        prep.append('call { capRstRes = triResetTemporal [0, 0]; format ["view=%1;event=det;reset=%2", capView, capRstRes] call capLog }')
    if o.has_seed:
        prep.append('call { capSdRes = triSeedRandom 1; format ["view=%1;event=det;seed=%2", capView, capSdRes] call capLog }')
    if o.timing_pass == "before":
        out += prep + timing_lines(c, lab) + ["[] call capPin"]
    out += prep
    out.append(f'call {{ capPathRes = triCamPath {path_expr(c)}; capRecRes = triRecord ["motion/{lab}.raw", {n}, 0]; '
               f'format ["view=%1;event=motion_start;clip={lab};frames={n};path=%2;rec=%3", capView, capPathRes, capRecRes] call capLog }}')
    out.append("triRecordState")
    out.append(f'call {{ capRecSt = triRecordState; capPathRes = triCamPath []; '
               f'format ["view=%1;event=motion_done;clip={lab};state=%2", capView, capRecSt] call capLog }}')
    out.append("[] call capPin")
    if o.timing_pass == "after":
        out += prep + timing_lines(c, lab) + ["[] call capPin"]
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default="")
    ap.add_argument("--clips", default="")
    ap.add_argument("--modes", default="remastered")
    ap.add_argument("--has-reset", action="store_true")
    ap.add_argument("--has-seed", action="store_true")
    ap.add_argument("--has-deterministic", action="store_true")
    ap.add_argument("--fixed-settle", type=int, default=0, help="GATE-2: exactly N settle frames per mode (0: draw-count settle)")
    ap.add_argument("--settle-wait-ms", type=int, default=1000, help="with --fixed-settle: runner-side pause before the last 30 frames")
    ap.add_argument("--timing-pass", default="off", choices=["off", "after", "before"],
                    help="OVERLAY-1: replay each clip unrecorded for its frame-time graph (default off)")
    ap.add_argument("--list", action="store_true")
    o = ap.parse_args()
    err = gatelib.fixed_settle_error(o.fixed_settle, o.settle_wait_ms, o.has_deterministic)
    if err:
        print(f"gen_motion: {err}")
        return 2
    names = [n for n in (o.clips.split(",") if o.clips else DEFAULT_CLIPS) if n]
    unknown = [n for n in names if n not in CLIPS]
    if unknown:
        print(f"gen_motion: unknown clip(s) {', '.join(unknown)}; known: {', '.join(CLIPS)}")
        return 2
    modes = [m for m in o.modes.split(",") if m]
    by_world = {}
    for n in names:
        by_world.setdefault(gv.world_of(CLIPS[n]["view"]), []).append(n)
    # dry clips first, rain clips last in each island session, as gen_golden orders its views (P0-17)
    for w in by_world:
        by_world[w].sort(key=lambda n: gv.is_rain(CLIPS[n]["view"]))
    plan = dict(set="motion", clips={n: dict(CLIPS[n], frames=clip_frames(CLIPS[n]), world=gv.world_of(CLIPS[n]["view"]))
                                     for n in names},
                modes=modes, labels={n: {m: label_of(n, m) for m in modes} for n in names}, fps=FPS, step=STEP,
                has_reset=o.has_reset, has_seed=o.has_seed, has_deterministic=o.has_deterministic, worlds=by_world)
    if o.fixed_settle:
        plan.update(fixed_settle=o.fixed_settle, settle_wait_ms=o.settle_wait_ms)
    if o.timing_pass != "off":
        plan.update(timing_pass=o.timing_pass, timing_chunk=TIMING_CHUNK, timing_tail=TIMING_TAIL)
    if o.list or not o.out:
        print(json.dumps(plan, indent=1))
        return 0
    os.makedirs(o.out, exist_ok=True)
    shutil.copyfile(os.path.join(gatelib.CAPTURES, "capture_lib.sqf"), os.path.join(o.out, "capture_lib.sqf"))
    files = []
    t = 100
    for world, clips in by_world.items():
        v0 = gv.view(CLIPS[clips[0]]["view"])
        title = f"motion clips (P0-10 recorder): {world}"
        lines = [capgen.header(world, v0, title),
                 f'call {{ capStepRes = triSetFixedStep {STEP:.7f}; setAccTime 1; '
                 'format ["view=setup;event=step;res=%1", capStepRes] call capLog }']
        if any("capBurn" in (gv.view(CLIPS[n]["view"]).get("refresh") or "") for n in clips):
            lines.append(LIB_BURN)
        if o.fixed_settle:
            lines.append('call { capDetOn = triDeterministic true; format ["view=setup;event=det;deterministic=%1", capDetOn] call capLog }')
            lines.append(gatelib.fixed_settle_setup(o.fixed_settle, o.settle_wait_ms))
        frames = 0
        for name in clips:
            c = CLIPS[name]
            v = gv.view(c["view"])
            lines.append(f"\n// ==== {name} ({v['name']})")
            lines += capgen.begin(v, name, sim_time=t)
            lines.append("triSimFrames 150")
            for mode in modes:
                lines += clip_block(name, c, v, mode, o)
                frames += clip_frames(c)
            lines += capgen.end(v)
            t += 200
        lines.append('\ncall { capStepRes = triSetFixedStep 0; "view=done;event=done" call capLog }')
        lines.append("triEndTest")
        stem = f"motion_{world.lower()}"
        capgen.write(Path(o.out) / f"{stem}.test.sqf", "\n".join(lines) + "\n")
        # the top-level triRecordState waits for a whole clip plus its write: give the wait room
        capgen.write(Path(o.out) / f"{stem}.test.toml", capgen.toml(world, title, gv.WORLDS, timeout=5400, assert_timeout=900))
        files.append(str(Path(o.out) / f"{stem}.test.sqf"))
    plan["files"] = files
    gatelib.write_json(os.path.join(o.out, "motion.plan.json"), plan)
    print(json.dumps(plan, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
