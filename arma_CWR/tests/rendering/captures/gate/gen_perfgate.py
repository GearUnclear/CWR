"""Perf-gate test generator (PERF-PLAN 3.3 "Perf gate"; P0-3). Writes the .test.sqf/.toml
files of one bench set into a caller folder, plus a copy of capture_lib.sqf they include.

    python -I gen_perfgate.py --set quick|full|abba|abba-scoped|obs|hitch|legacy --out <dir>
                              [--views v1,v2] [--read meas|ring] [--config fixed|legacy]
                              [--rounds N] [--windows N] [--frames 256] [--no-stills]
                              [--has-fpscap] [--list]

Protocol per view and mode (3.3): capBegin, 150 frames, capMode (triPinObjectDetail,
triSetLighting <mode> with every knob at its shipped default, triSettleScene [300,30,2]),
60 warm frames, then per window: triGpuTimes false, triGpuTimes true, triPerfReset (the
working reset on afdb4d60), <frames> frames, then
  read=meas (legacy reading, the only one on afdb4d60): one event=meas line (triPerfStats
            window mean/p95/max per phase + counters, triGpuTimes EMA weight 0.1);
  read=ring (02b67fb1 on: triPerfRing/triGpuRing): first an event=ring label line and the
            two ring verbs in their labelled form [<frames>, "<label>"], which write
            perfring-<label>.csv and gpuring-<label>.csv under TRI_OUTPUT_DIR and log the
            rings; then the same meas line (for counters only). The rings must be read
            BEFORE capMeas: its `triGpuTimes true` returns the EMA summary and then restarts
            the GPU window, which empties the GPU ring (EngineGL33::SetGpuTimingEnabled),
            so a triGpuRing after it answers FAIL:no_frames (BATCH1 abba-scoped, 2026-10-31).
After the windows, one still `<view>__bench` (round 1 only) for vgate.

Sets (views default; --views overrides):
  quick        valley_0515, town_night_rain, sunrise_ridge; 2 rounds, 1 window (M2's quick).
  full         the 8 standard views + pin-0 rows at valley and Eden; 1 round, 3 windows.
  abba         as full (perf-bench.ps1 runs it on 8 boots, ABBA x 2).
  abba-scoped  as full but only --views (the claimed views); perf-bench adds an AB
               regression file over the other standard views with --set abba-scoped --rest.
  obs          valley_0515 and broken_high; 1 round, 3 windows (perf-bench toggles the env).
  hitch        the six round-1 views, 4 windows of 256 = 1,024 steady frames each (H-steady
               on afdb4d60; P0-13 pacing fields join when INFRA-C3 lands them).
  legacy       round 1's corrected baseline exactly: valley, sunrise, wet town, combat_fires,
               noon (Abel) and Eden 250 m, Remastered and Classic, 1 window, 2 rounds, rounds
               outer as round 1 did; config legacy (vsync on; perf-bench seeds fpsCap 60).

Config fixed (every comparison): triSetVsync 0 in the test, fpsCap 0 seeded in graphics.cfg by
perf-bench (triSetFpsCap does not exist on afdb4d60; --has-fpscap adds `triSetFpsCap 0` once
P0-7 lands). Config legacy: triSetVsync 1, fpsCap 60 seeded. 1920x1080 windowed, render scale
1.0, 4x MSAA (capgen.header), detail pin 1 (capDetail), lighting remastered.

Order: rain views run last in every session (P0-17: a dry still after rain is refused), each
view's rounds back to back. legacy keeps round 1's order (rounds outer) and takes no stills.
--list prints the plan as JSON (views, worlds, labels) without writing anything.
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
capTimerReset = { capGpuOff = triGpuTimes false; capGpuOn = triGpuTimes true; capPerfRes = triPerfReset 0 }
capMeas = {
    capGpu = triGpuTimes true;
    capPerf = triPerfStats 0;
    format ["view=%1;event=meas;label=%2;mode=%3;lit=%4;fps=%5;%6;perf=%7;gpu=%8", capView, _this, capModeName, capLit, triFps, triSceneStats, capPerf, capGpu] call capLog;
}
capBurn = { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }
'''

# The ring dump at the end of a window, emitted before capMeas (see the docstring: capMeas's
# `triGpuTimes true` empties the GPU ring). Verbs as landed by INFRA-C1 (P0-4,
# Game/Commands/GameStateExtTestPerf.cpp): `triPerfRing [n, "label"]` writes
# <TRI_OUTPUT_DIR>/perfring-<label>.csv (i,total,<13 phases>,draws), `triGpuRing [n, "label"]`
# writes gpuring-<label>.csv (i,frame,world,pass1,ground,rest,post); both return "OK:<n>" or
# "FAIL:...". The labels here only hold letters, digits and '_', which the engine keeps as is.
RING_CALL = ('call {{ format ["view=%1;event=ring;label={lab}", capView] call capLog; '
             'capRing = triPerfRing [{frames}, "{lab}"]; capGRing = triGpuRing [{frames}, "{lab}"]; '
             'format ["view=%1;event=ringres;label={lab};cpu=%2;gpu=%3", capView, capRing, capGRing] call capLog }}')

SETS = {
    "quick": dict(views=gv.QUICK_VIEWS, rounds=2, windows=1, modes=["remastered"], pin0=False),
    "full": dict(views=gv.BENCH_VIEWS, rounds=1, windows=3, modes=["remastered"], pin0=True),
    "abba": dict(views=gv.BENCH_VIEWS, rounds=1, windows=3, modes=["remastered"], pin0=True),
    "abba-scoped": dict(views=gv.BENCH_VIEWS, rounds=1, windows=3, modes=["remastered"], pin0=True),
    "obs": dict(views=["valley_0515", "broken_high"], rounds=1, windows=3, modes=["remastered"], pin0=False),
    "hitch": dict(views=gv.ROUND1_VIEWS, rounds=1, windows=4, modes=["remastered"], pin0=False),
    "legacy": dict(views=gv.ROUND1_VIEWS, rounds=2, windows=1, modes=["remastered", "classic"], pin0=False),
    # untimed counts: the event=meas counters (draws, objects, X1 / P0-15 counters) and any env-gated
    # stats lines (e.g. event=landfit) over 2 short windows, plus one still per view for objects/draws
    "counts": dict(views=gv.QUICK_VIEWS, rounds=1, windows=2, modes=["remastered"], pin0=False),
    # boot-cache hitch (W1-4): the shortest scene that reaches InitGL and the first world frames
    "bootcache": dict(views=["broken_high"], rounds=1, windows=1, modes=["remastered"], pin0=False),
}


def meas_block(v, label, mode, windows, frames, read, pin=None):
    out = [f"// meas {label}"]
    if pin is not None:
        out.append(f"call {{ capDetail = {pin} }}")
    out.append(f'"{mode}" call capMode')
    if v.get("refresh"):
        out.append(v["refresh"])
    out.append("triSimFrames 60")
    for k in range(1, windows + 1):
        lab = f"{label}_w{k}" if windows > 1 else label
        out.append("[] call capTimerReset")
        out.append(f"triSimFrames {frames}")
        if read == "ring":
            out.append(RING_CALL.format(lab=lab, frames=frames))  # before capMeas, which empties the GPU ring
        out.append(f'"{lab}" call capMeas')
    if pin is not None:
        out.append("call { capDetail = 1 }")
    return out


def still_block(v, label):
    out = ['"remastered" call capMode', "triSimFrames 30", "[] call capPin"]
    if v.get("refresh"):
        out.append(v["refresh"])
    out += ["triSimFrames 3", f'triScreenshot "{label}"', f'"{label}" call capRecord']
    return out


def config_lines(config, has_fpscap):
    vs = 1 if config == "legacy" else 0
    out = [f'call {{ capVsRes = triSetVsync {vs}; format ["view=setup;event=config;config={config};vsync=%1", capVsRes] call capLog }}']
    if has_fpscap:
        cap = 60 if config == "legacy" else 0
        out.append(f'call {{ capCapRes = triSetFpsCap {cap}; format ["view=setup;event=config;fpscap=%1", capCapRes] call capLog }}')
    return out


def plan(o):
    s = SETS[o.set]
    names = [n for n in (o.views.split(",") if o.views else s["views"]) if n]
    names = [gv.resolve(n) for n in names]
    if o.rest:
        names = [n for n in gv.BENCH_VIEWS if n not in names]
    rounds = o.rounds or s["rounds"]
    windows = o.windows or s["windows"]
    modes = o.modes.split(",") if o.modes else s["modes"]
    stills = (not o.no_stills) and o.set not in ("legacy", "bootcache", "hitch")
    read = o.read
    config = o.config or ("legacy" if o.set == "legacy" else "fixed")
    by_world = {}
    for n in names:
        v = gv.view(n)
        by_world.setdefault(gv.world_of(n), []).append(v)
    for w in by_world:
        if o.set != "legacy":
            by_world[w].sort(key=lambda v: gv.is_rain(v["name"]))  # rain last (P0-17), stable otherwise
    return dict(set=o.set, rounds=rounds, windows=windows, frames=o.frames, modes=modes, stills=stills, read=read,
                config=config, pin0=s["pin0"] or o.pin0, scale2=o.scale2,
                worlds={w: [v["name"] for v in vs] for w, vs in by_world.items()}), by_world


def body_for(p, vs):
    out = []
    frames, windows, read = p["frames"], p["windows"], p["read"]
    clock = [0]

    def next_time():
        # Pinned sim time per view and round, always moving forward (capgen.begin); round 1
        # (legacy) left the clock running, so legacy keeps None.
        if p["set"] == "legacy":
            return None
        clock[0] += 1
        return 100 + 200 * clock[0]

    def view_rounds(v, rounds_iter):
        lines = []
        for r in rounds_iter:
            lab = f"{v['name']}_r{r}"
            lines.append(f"\n// ---- {lab}")
            lines += capgen.begin(v, lab, sim_time=next_time())
            lines.append("triSimFrames 150")
            for m in p["modes"]:
                tag = m if m in ("remastered", "classic") else m.replace(",", "_").replace("=", "")
                lines += meas_block(v, f"{lab}__{tag}", m, windows, frames, read)
            if p.get("scale2"):
                lines.append("call { capScRes = triSetRenderScale 2.0 }")
                lines += meas_block(v, f"{lab}__rem_s2", "remastered", windows, frames, read)
                lines.append("call { capScRes = triSetRenderScale 1.0 }")
            if p["pin0"] and v["name"] in gv.PIN0_VIEWS:
                lines += meas_block(v, f"{lab}__rem_pin0", "remastered", windows, frames, read, pin=0)
            if p["stills"] and r == 1:
                lines += still_block(v, f"{v['name']}__bench")
            lines += capgen.end(v)
        return lines

    if p["set"] == "legacy":
        for r in range(1, p["rounds"] + 1):  # round 1's order: rounds outer
            for v in vs:
                out += view_rounds(v, [r])
    else:
        for v in vs:
            out += view_rounds(v, range(1, p["rounds"] + 1))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--set", required=True, choices=sorted(SETS))
    ap.add_argument("--out", default="")
    ap.add_argument("--views", default="")
    ap.add_argument("--rest", action="store_true", help="abba-scoped: the standard views NOT in --views")
    ap.add_argument("--read", default="meas", choices=["meas", "ring"])
    ap.add_argument("--config", default="", choices=["", "fixed", "legacy"])
    ap.add_argument("--modes", default="")
    ap.add_argument("--rounds", type=int, default=0)
    ap.add_argument("--windows", type=int, default=0)
    ap.add_argument("--frames", type=int, default=256)
    ap.add_argument("--pin0", action="store_true", help="add pin-0 rows at valley/Eden even with --views")
    ap.add_argument("--scale2", action="store_true", help="add a render-scale-2 row per view (makes CPU-bound views GPU-bound)")
    ap.add_argument("--no-stills", action="store_true")
    ap.add_argument("--has-fpscap", action="store_true", help="the dist has triSetFpsCap (P0-7)")
    ap.add_argument("--prefix", default="", help="file name prefix (default pg_<set>)")
    ap.add_argument("--list", action="store_true")
    o = ap.parse_args()
    p, by_world = plan(o)
    if o.list or not o.out:
        print(json.dumps(p, indent=1))
        return 0
    os.makedirs(o.out, exist_ok=True)
    shutil.copyfile(os.path.join(gatelib.CAPTURES, "capture_lib.sqf"), os.path.join(o.out, "capture_lib.sqf"))
    prefix = o.prefix or ("pg_" + o.set.replace("-", "_") + ("_rest" if o.rest else ""))
    files = []
    for world, vs in by_world.items():
        title = f"perf gate {o.set} ({p['config']}, read {p['read']}): {world}"
        lines = [capgen.header(world, vs[0], title), LIB] + config_lines(p["config"], o.has_fpscap) + body_for(p, vs)
        lines.append('\ncall { "view=done;event=done" call capLog }')
        lines.append("triEndTest")
        name = f"{prefix}_{world.lower()}"
        sqf = os.path.join(o.out, f"{name}.test.sqf")
        capgen.write(Path(sqf), "\n".join(lines) + "\n")
        capgen.write(Path(o.out) / f"{name}.test.toml",
                     capgen.toml(world, title, gv.WORLDS, timeout=7200))
        files.append(sqf)
    p["files"] = files
    gatelib.write_json(os.path.join(o.out, f"{prefix}.plan.json"), p)
    print(json.dumps(p, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
