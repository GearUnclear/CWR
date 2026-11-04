"""Round-1 perf-audit measurement tests (moved from tmp/perf-audit/reports/measure/gen_perf.py,
2026-10-31; paths are now relative to this script and the output folder is an argument).

    python -I gen_perf.py --out <dir>                # observer A/B + corrected baseline sets
    python -I gen_perf.py --out <dir> --extra        # TAA bias and LOD pin set
    python -I gen_perf.py --out <dir> --scratch      # scratch-build ring set (needs triPerfRing)

Each measurement block: optional variant (render scale, vsync), capMode (lighting setting
+ settle + timer reset), 60 warm frames, timer reset (triGpuTimes false then true, triPerfReset),
then `windows` windows of 256 frames, each logged as one event=meas line with triPerfStats
(avg/p95/max per phase + counters) and triGpuTimes. No screenshot inside a timing window.

This is the historical generator, kept so round 1 can be re-run byte for byte. New gate
work uses gen_perfgate.py (same protocol, the fixed configuration of PERF-PLAN 3.3, sets).
The generated tests #include capture_lib.sqf, so it is copied next to them.
"""
import argparse
import os
import shutil
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gatelib  # noqa: E402,F401  (puts the capture rig on sys.path)
from pathlib import Path  # noqa: E402

import capgen  # noqa: E402
import views  # noqa: E402

OUT = None  # set by main from --out

LIB = r'''
capTimerReset = { capGpuOff = triGpuTimes false; capGpuOn = triGpuTimes true; capPerfRes = triPerfReset 0 }
capMeas = {
    capGpu = triGpuTimes true;
    capPerf = triPerfStats 0;
    format ["view=%1;event=meas;label=%2;mode=%3;lit=%4;fps=%5;%6;perf=%7;gpu=%8", capView, _this, capModeName, capLit, triFps, triSceneStats, capPerf, capGpu] call capLog;
}
capBurn = { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }
capShellAt = {
    private ["_p"];
    _p = [_this select 0, _this select 1] call capRel;
    capShellObj = createShell [_this select 4, [_p select 0, _p select 1, _this select 2], _this select 3, player, objNull];
    format ["view=%1;event=shell;ammo=%2;pos=%3;object=%4", capView, _this select 4, _p, capShellObj] call capLog;
}
'''


def V(name):
    for v in views.TIMING_ABEL + views.TIMING_EDEN:
        if v["name"] == name:
            return dict(v)
    raise KeyError(name)


COMBAT = dict(name="combat_fires", x=7600, z=10850, eye=5, h=0, p=-12, date=[1985, 7, 12, 23, 0], vd=1000,
              props=views.FFH_PROPS, refresh="[] call capBurn", feature="night fire + smoke")


def meas(v, label, setting, scale=None, vsync=None, windows=1, frames=256):
    out = [f"// meas {label}"]
    if scale is not None:
        out.append(f"call {{ capScRes = triSetRenderScale {scale} }}")
    if vsync is not None:
        out.append(f"call {{ capVsRes = triSetVsync {vsync} }}")
    out.append(f'"{setting}" call capMode')
    if v.get("refresh"):
        out.append(v["refresh"])
    out.append("triSimFrames 60")
    for k in range(1, windows + 1):
        out.append("[] call capTimerReset")
        out.append(f"triSimFrames {frames}")
        lab = label if windows == 1 else f"{label}_w{k}"
        out.append(f'"{lab}" call capMeas')
    if scale is not None:
        out.append("call { capScRes = triSetRenderScale 1.0 }")
    if vsync is not None:
        out.append("call { capVsRes = triSetVsync 1 }")
    return out


def view(v, rnd, blocks):
    lab = f"{v['name']}_r{rnd}"
    out = [f"\n// ---- {lab}"]
    out += capgen.begin(v, lab)
    out.append("triSimFrames 150")
    for b in blocks:
        out += b(v, lab)
    out += capgen.end(v)
    return out


def std(v, lab):
    return meas(v, f"{lab}__remastered", "remastered") + meas(v, f"{lab}__classic", "classic")


def write(name, world, v0, body, title):
    lines = [capgen.header(world, v0, title), LIB] + body
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    capgen.write(OUT / f"{name}.test.sqf", "\n".join(lines) + "\n")
    capgen.write(OUT / f"{name}.test.toml", capgen.toml(world, title, views.WORLDS, timeout=7200))
    print("wrote", name)


def main():
    valley, broken = V("valley_0515"), V("broken_high")
    sunrise, rain = V("sunrise_ridge"), V("town_night_rain")
    noon = V("noon_fields")
    # 1. observer A/B set: same file run with and without POSEIDON_OBSERVER_SAMPLE
    body = []
    for r in (1, 2):
        body += view(valley, r, [std])
    write("pa_obs_abel", "Abel", valley, body, "perf audit: observer A/B, Abel valley")
    body = []
    for r in (1, 2):
        body += view(broken, r, [std])
    write("pa_obs_eden", "Eden", broken, body, "perf audit: observer A/B, Eden broken_high")

    # 2-3. corrected baseline, GPU-bound check, vsync, TAA bias, distribution
    body = []
    for r in (1, 2):
        for v in (valley, sunrise, rain, COMBAT, noon):
            blocks = [std]
            if r == 1:
                blocks.append(lambda v, lab: meas(v, f"{lab}__baseline", "baseline"))
                blocks.append(lambda v, lab: meas(v, f"{lab}__rem_s2", "remastered", scale=2.0))
            if r == 1 and v is valley:
                blocks.append(lambda v, lab: meas(v, f"{lab}__rem_vs0", "remastered", vsync=0))
                blocks.append(lambda v, lab: meas(v, f"{lab}__rem_dist", "remastered", windows=3))
            if r == 1 and v is sunrise:
                for b in ("-0.01", "0"):
                    blocks.append(lambda v, lab, b=b: meas(v, f"{lab}__rem_bias{b}", f"remastered,taaMipBias={b}"))
                blocks.append(lambda v, lab: meas(v, f"{lab}__rem_taa0", "remastered,taa=0"))
                blocks.append(lambda v, lab: meas(v, f"{lab}__rem_taa0_s2", "remastered,taa=0", scale=2.0))
                blocks.append(lambda v, lab: meas(v, f"{lab}__rem_dist", "remastered", windows=3))
            body += view(v, r, blocks)
    # 4. combat mask first use: a shell within 60 m at night, then a second one (warm)
    lab = "combat_shell_r1"
    body.append("\n// ---- combat shells")
    body += capgen.begin(COMBAT, lab)
    body.append("triSimFrames 150")
    body.append('"remastered" call capMode')
    body.append("[] call capBurn")
    body.append("triSimFrames 60")
    for k in (1, 2, 3):
        body.append("[] call capTimerReset")
        body.append('[30, -6, 40, [0, 0, -250], "Shell120"] call capShellAt')
        body.append("triSimFrames 120")
        body.append(f'"{lab}__shell{k}" call capMeas')
    body += capgen.end(COMBAT)
    write("pa_full_abel", "Abel", valley, body, "perf audit: corrected baseline, Abel")
    body = []
    for r in (1, 2):
        blocks = [std]
        if r == 1:
            blocks.append(lambda v, lab: meas(v, f"{lab}__baseline", "baseline"))
            blocks.append(lambda v, lab: meas(v, f"{lab}__rem_s2", "remastered", scale=2.0))
            blocks.append(lambda v, lab: meas(v, f"{lab}__rem_dist", "remastered", windows=3))
        body += view(broken, r, blocks)
    write("pa_full_eden", "Eden", broken, body, "perf audit: corrected baseline, Eden")


def detail(v, label, setting, pin, **kw):
    out = [f"call {{ capDetail = {pin} }}"] + meas(v, label, setting, **kw) + ["call { capDetail = 1 }"]
    return out


def extra():
    valley, sunrise, rain = V("valley_0515"), V("sunrise_ridge"), V("town_night_rain")
    body = []
    for r in (1, 2):
        b = [lambda v, lab: meas(v, f"{lab}__remastered", "remastered"),
             lambda v, lab: meas(v, f"{lab}__rem_bias0", "remastered,taaMipBias=0"),
             lambda v, lab: meas(v, f"{lab}__rem_taa0", "remastered,taa=0")]
        if r == 1:
            for pin in (2, 4, 0):
                b.append(lambda v, lab, pin=pin: detail(v, f"{lab}__rem_lod{pin}", "remastered", pin))
            b.append(lambda v, lab: detail(v, f"{lab}__classic_lod0", "classic", 0))
            b.append(lambda v, lab: detail(v, f"{lab}__rem_lod0_dist", "remastered", 0, windows=3))
        body += view(valley, r, b)
        b = [lambda v, lab: meas(v, f"{lab}__remastered", "remastered"),
             lambda v, lab: meas(v, f"{lab}__rem_bias0", "remastered,taaMipBias=0"),
             lambda v, lab: meas(v, f"{lab}__rem_taa0", "remastered,taa=0")]
        body += view(rain, r, b)
        if r == 1:
            b = [lambda v, lab: meas(v, f"{lab}__rem_s2", "remastered", scale=2.0),
                 lambda v, lab: meas(v, f"{lab}__rem_bias0_s2", "remastered,taaMipBias=0", scale=2.0),
                 lambda v, lab: detail(v, f"{lab}__rem_lod0", "remastered", 0)]
            body += view(sunrise, r, b)
    write("pa_extra_abel", "Abel", valley, body, "perf audit: TAA bias and LOD pin, Abel")


def ringmeas(v, label, setting):
    out = meas(v, label, setting)
    out.append(f'call {{ format ["view=%1;event=ring;label={label}", capView] call capLog; capRing = triPerfRing 256 }}')
    return out


def scratch():
    valley, sunrise, rain = V("valley_0515"), V("sunrise_ridge"), V("town_night_rain")
    body = []
    for r in (1, 2):
        body += view(valley, r, [lambda v, lab: ringmeas(v, f"{lab}__remastered", "remastered"),
                                 lambda v, lab: meas(v, f"{lab}__classic", "classic"),
                                 lambda v, lab: meas(v, f"{lab}__baseline", "baseline"),
                                 lambda v, lab: meas(v, f"{lab}__rem_bias-0.01", "remastered,taaMipBias=-0.01")])
        body += view(sunrise, r, [lambda v, lab: ringmeas(v, f"{lab}__remastered", "remastered"),
                                  lambda v, lab: meas(v, f"{lab}__classic", "classic"),
                                  lambda v, lab: meas(v, f"{lab}__baseline", "baseline")])
        body += view(rain, r, [lambda v, lab: ringmeas(v, f"{lab}__remastered", "remastered")])
    write("pa_scr_abel", "Abel", valley, body, "perf audit: scratch experiments, Abel")


def cli():
    global OUT
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", required=True, help="output folder for the .test.sqf/.toml files")
    ap.add_argument("--extra", action="store_true")
    ap.add_argument("--scratch", action="store_true")
    a = ap.parse_args()
    OUT = Path(a.out)
    OUT.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(os.path.join(gatelib.CAPTURES, "capture_lib.sqf"), OUT / "capture_lib.sqf")
    if a.extra:
        extra()
    elif a.scratch:
        scratch()
    else:
        main()


if __name__ == "__main__":
    cli()
