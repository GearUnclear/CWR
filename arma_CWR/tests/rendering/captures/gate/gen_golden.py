"""Golden still tests for vgate (PERF-PLAN 3.3 "Visual gate", P0-8; S6 A1): T1 (8 views) or T2
(25 staged views; the cockpit, optics and combat stills come from their own tests, see
golden_views.EXTRA_TESTS). Writes one .test.sqf/.toml per island into a caller folder.

    python -I gen_golden.py --tier T1|T2 --out <dir> [--views v1,v2] [--has-reset] [--has-seed]
                            [--has-deterministic] [--fixed-step]
                            [--fixed-settle N [--settle-wait-ms MS]] [--list]

Determinism recipe per shot (3.3), with the verbs this dist has:
  pin camera, date, weather, view distance, props (capBegin), a pinned sim time per view;
  triPinObjectDetail 1, triSetLighting remastered, triSettleScene [300,30,2] (capMode; the
  comparator refuses UNSETTLED, the runner reshoots);
  [--fixed-settle N] (GATE-2, needs --has-deterministic) instead exactly N settle frames: N - 30,
  `triWait MS` (default 1000; no frame runs under the capture hold) and 30 that must be idle
  (capModeTail), so the frame count before each shot does not follow the draw count;
  [--fixed-step] triSetFixedStep 1/60; triPinCloudDrift (capgen.begin);
  [--has-reset] triResetTemporal 1 (P0-6; M2's scratch build has it);
  [--has-seed] triSeedRandom 1; [--has-deterministic] triDeterministic true;
  triSimFrames 32, triCaptureAtPresent true (header), triScreenshot "<view>__gold".
Order: dry views first, rain views last in each island session (P0-17); the comparator also
refuses a dry shot taken after rain. Shot names: <view>__gold.png (vgate keys).
run-vgate.ps1 detects the verbs (byte scan of PoseidonGame.exe) and passes the flags.
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

LIB = r'''
capBurn = { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }
'''


def shot_block(v, label, o, mode="remastered"):
    out = gatelib.mode_lines(mode, o.fixed_settle, o.settle_wait_ms) + ["triSimFrames 60", "[] call capPin"]
    if v.get("refresh"):
        out.append(v["refresh"])
    # only verbs this dist has: an unknown verb is a script error under --autotest
    if o.has_reset:
        out.append('call { capRt = triResetTemporal 1; format ["view=%1;event=det;reset=%2", capView, capRt] call capLog }')
    if o.has_seed:
        out.append('call { capSd = triSeedRandom 1; format ["view=%1;event=det;seed=%2", capView, capSd] call capLog }')
    out += ["triSimFrames 32", f'triScreenshot "{label}"', f'"{label}" call capRecord']
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--tier", default="T1", choices=["T1", "T2"])
    ap.add_argument("--views", default="")
    ap.add_argument("--out", default="")
    ap.add_argument("--label", default="gold")
    ap.add_argument("--modes", default="remastered", help="triSetLighting modes, comma separated; remastered is labelled --label, others by their name")
    ap.add_argument("--has-reset", action="store_true")
    ap.add_argument("--has-seed", action="store_true")
    ap.add_argument("--has-deterministic", action="store_true")
    ap.add_argument("--fixed-step", action="store_true")
    ap.add_argument("--fixed-settle", type=int, default=0, help="GATE-2: exactly N settle frames per mode (0: draw-count settle)")
    ap.add_argument("--settle-wait-ms", type=int, default=1000, help="with --fixed-settle: runner-side pause before the last 30 frames")
    ap.add_argument("--list", action="store_true")
    o = ap.parse_args()
    err = gatelib.fixed_settle_error(o.fixed_settle, o.settle_wait_ms, o.has_deterministic)
    if err:
        print(f"gen_golden: {err}")
        return 2
    names = [n for n in (o.views.split(",") if o.views else (gv.T1 if o.tier == "T1" else gv.T2)) if n]
    names = [gv.resolve(n) for n in names]
    by_world = {}
    for n in names:
        by_world.setdefault(gv.world_of(n), []).append(gv.view(n))
    for w in by_world:
        by_world[w].sort(key=lambda v: gv.is_rain(v["name"]))
    plan = dict(tier=o.tier, label=o.label, modes=o.modes.split(","), worlds={w: [v["name"] for v in vs] for w, vs in by_world.items()},
                has_reset=o.has_reset, has_seed=o.has_seed, has_deterministic=o.has_deterministic, fixed_step=o.fixed_step,
                extra_tests=gv.EXTRA_TESTS if o.tier == "T2" else {})
    if o.fixed_settle:
        plan.update(fixed_settle=o.fixed_settle, settle_wait_ms=o.settle_wait_ms)
    if o.list or not o.out:
        print(json.dumps(plan, indent=1))
        return 0
    os.makedirs(o.out, exist_ok=True)
    shutil.copyfile(os.path.join(gatelib.CAPTURES, "capture_lib.sqf"), os.path.join(o.out, "capture_lib.sqf"))
    files = []
    for world, vs in by_world.items():
        title = f"vgate golden stills {o.tier}: {world}"
        lines = [capgen.header(world, vs[0], title), LIB]
        if o.has_deterministic:
            lines.append('call { capDetOn = triDeterministic true; format ["view=setup;event=det;deterministic=%1", capDetOn] call capLog }')
        if o.fixed_step:
            lines.append('call { capStepRes = triSetFixedStep (1/60); format ["view=setup;event=step;res=%1", capStepRes] call capLog }')
        if o.fixed_settle:
            lines.append(gatelib.fixed_settle_setup(o.fixed_settle, o.settle_wait_ms))
        t = 100
        for v in vs:
            lines.append(f"\n// ---- {v['name']}")
            lines += capgen.begin(v, v["name"], sim_time=t)
            lines.append("triSimFrames 150")
            for mode in o.modes.split(","):
                tag = o.label if mode == "remastered" else mode.replace(",", "_").replace("=", "")
                lines += shot_block(v, f"{v['name']}__{tag}", o, mode)
            lines += capgen.end(v)
            t += 200
        lines.append('\ncall { "view=done;event=done" call capLog }')
        lines.append("triEndTest")
        name = f"vg_{o.tier.lower()}_{world.lower()}"
        capgen.write(Path(o.out) / f"{name}.test.sqf", "\n".join(lines) + "\n")
        capgen.write(Path(o.out) / f"{name}.test.toml", capgen.toml(world, title, gv.WORLDS, timeout=5400))
        files.append(str(Path(o.out) / f"{name}.test.sqf"))
    plan["files"] = files
    gatelib.write_json(os.path.join(o.out, f"vg_{o.tier.lower()}.plan.json"), plan)
    print(json.dumps(plan, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
