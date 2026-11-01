"""Motion clip tests for the bench's `motion` set (PERF-PLAN P0-10, 3.3 T3, 3.6): one camera path
per clip, recorded at real speed by the P0-10 recorder (triRecord, every presented frame), once
per lighting mode. Writes one .test.sqf/.toml per island into a caller folder plus
motion.plan.json. Both sides of a job run the same files.

    python -I gen_motion.py --out <dir> [--clips c1,c2] [--modes remastered,classic]
                            [--has-reset] [--has-seed] [--has-deterministic] [--list]

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
}
DEFAULT_CLIPS = ["valley_foliage_pan"]


def clip_frames(c):
    return int(round((c["keys"][-1][0] + c["hold"]) * FPS))


def path_expr(c):
    ks = []
    for k in c["keys"]:
        t, dx, dy, dz, dh, dp, ease = k
        # triCamPath keys: [t, east, upASL, north, heading, pitch, ease]; the view's own camera from capPin
        ks.append(f"[{t:g}, capOX + {dx:g}, capEyeASL + {dy:g}, capOZ + {dz:g}, capH + {dh:g}, capP + {dp:g}, {ease}]")
    return "[" + ", ".join(ks) + "]"


def label_of(clip, mode):
    return f"{clip}__{'today' if mode == 'remastered' else mode.replace(',', '_').replace('=', '')}"


def clip_block(name, c, v, mode, o):
    lab = label_of(name, mode)
    n = clip_frames(c)
    out = [f"\n// ---- {lab}: {n} frames", f'"{mode}" call capMode', "triSimFrames 60", "[] call capPin"]
    if v.get("refresh"):
        out.append(v["refresh"])
    if o.has_deterministic:
        out.append('call { capDetRes = triDeterministic true; format ["view=%1;event=det;det=%2", capView, capDetRes] call capLog }')
    if o.has_reset:
        out.append('call { capRstRes = triResetTemporal [0, 0]; format ["view=%1;event=det;reset=%2", capView, capRstRes] call capLog }')
    if o.has_seed:
        out.append('call { capSdRes = triSeedRandom 1; format ["view=%1;event=det;seed=%2", capView, capSdRes] call capLog }')
    out.append(f'call {{ capPathRes = triCamPath {path_expr(c)}; capRecRes = triRecord ["motion/{lab}.raw", {n}, 0]; '
               f'format ["view=%1;event=motion_start;clip={lab};frames={n};path=%2;rec=%3", capView, capPathRes, capRecRes] call capLog }}')
    out.append("triRecordState")
    out.append(f'call {{ capRecSt = triRecordState; capPathRes = triCamPath []; '
               f'format ["view=%1;event=motion_done;clip={lab};state=%2", capView, capRecSt] call capLog }}')
    out.append("[] call capPin")
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default="")
    ap.add_argument("--clips", default="")
    ap.add_argument("--modes", default="remastered")
    ap.add_argument("--has-reset", action="store_true")
    ap.add_argument("--has-seed", action="store_true")
    ap.add_argument("--has-deterministic", action="store_true")
    ap.add_argument("--list", action="store_true")
    o = ap.parse_args()
    names = [n for n in (o.clips.split(",") if o.clips else DEFAULT_CLIPS) if n]
    unknown = [n for n in names if n not in CLIPS]
    if unknown:
        print(f"gen_motion: unknown clip(s) {', '.join(unknown)}; known: {', '.join(CLIPS)}")
        return 2
    modes = [m for m in o.modes.split(",") if m]
    by_world = {}
    for n in names:
        by_world.setdefault(gv.world_of(CLIPS[n]["view"]), []).append(n)
    plan = dict(set="motion", clips={n: dict(CLIPS[n], frames=clip_frames(CLIPS[n]), world=gv.world_of(CLIPS[n]["view"]))
                                     for n in names},
                modes=modes, labels={n: {m: label_of(n, m) for m in modes} for n in names}, fps=FPS, step=STEP,
                has_reset=o.has_reset, has_seed=o.has_seed, has_deterministic=o.has_deterministic, worlds=by_world)
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
