"""Generate the perf diagnosis test (research only, not committed).

Writes <out>.test.sqf and <out>.test.toml. Every config block:
    call { [label, mode, saturation(10 + mask), scale, msaa, vd, post] call pfCfg }
    triSimFrames N
    call { "label" call pfRead }
and optional top-level screenshots.
"""
import sys
from pathlib import Path

out = Path(sys.argv[1])
name = out.name

HEADER = r'''// PERF RESEARCH (not committed): where the remastered GPU time goes.
#include "capture_lib.sqf"

triSimUntil { alive player }

capWorld = "Abel"
capFov = 0.7
capSimT = 0
capPinTime = false
capUseView = false
capProps = []
capView = "setup"
capModeName = "none"
capLit = "none"
capGpu = "none"
capPerf = "none"
pfLabel = "none"
pfLit = "none"
pfSat = "none"
pfPost = "none"

// [label, mode, sat, scale, msaa, vd, post] call pfCfg
pfCfg = {
    pfLabel = _this select 0;
    pfLit = triSetLighting (_this select 1);
    pfSat = triSetLighting (format ["saturation=%1", _this select 2]);
    pfPost = triSetLighting (format ["post=%1", _this select 6]);
    pfScale = triSetRenderScale (_this select 3);
    pfMsaa = triSetMsaa (_this select 4);
    capVD = _this select 5;
    pfArgs = _this;
    [] call capPin;
    pfG0 = triGpuTimes false;
    pfG1 = triGpuTimes true;
    pfPR = triPerfReset 0;
}

pfRead = {
    pfGpu = triGpuTimes true;
    pfPerf = triPerfStats 0;
    logInfo (format ["UDPERF;view=%1;cfg=%2;args=%3;lit=%4;sat=%5;post=%6;perf=%7;gpu=%8", capView, pfLabel, pfArgs, pfLit, pfSat, pfPost, pfPerf, pfGpu]);
}

triCaptureAtPresent true
triSetMsaa 4
triSetRenderScale 1.5
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false

createCenter west
createCenter east
capGrpW = createGroup west
capProbe = "HeliHEmpty" createVehicle [7300, 10900, 0]
capCam = "camera" camCreate [7300, 10900, 2]
capCam cameraEffect ["internal", "back"]
triSimFrames 240
'''

lines = [HEADER]


def view(begin, spawns=None, settle=150):
    lines.append(f"\n{begin} call capBegin\n")
    if spawns:
        lines.append("call {\n" + "".join(f"    {s} call capSpawn;\n" for s in spawns) + "}\n")
    lines.append(f"triSimFrames {settle}\n")


def cfg(label, mode="remastered", mask=0, scale=1.5, msaa=4, vd=4000, post=1, frames=90, shot=False, sat=None):
    s = sat if sat is not None else 10 + mask
    lines.append(f'call {{ ["{label}", "{mode}", {s}, {scale}, {msaa}, {vd}, {post}] call pfCfg }}\n')
    lines.append(f"triSimFrames {frames}\n")
    lines.append(f'call {{ "{label}" call pfRead }}\n')
    if shot:
        lines.append(f'triScreenshot "{label}"\n')


# ---------------------------------------------------------------- sunrise_ridge
view('["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000]')
VD = 4000
cfg("sr_classic", "classic", vd=VD, shot=True)
cfg("sr_stock", sat=1.12, vd=VD, shot=True)
cfg("sr_m0", mask=0, vd=VD)
for m, tag in [(1, "shadowoff"), (2, "terrainoff"), (4, "cloudoff"), (8, "fogoff"), (16, "glossyoff"),
               (32, "localoff"), (64, "foliageoff"), (128, "unlit"), (256, "shadow1tap"),
               (512, "shadownoblend"), (1024, "shadow4tap"), (127, "allfeaturesoff"), (3, "shadow+terrainoff")]:
    cfg(f"sr_m{m}_{tag}", mask=m, vd=VD, shot=(m in (128, 2)))
cfg("sr_postoff", mask=0, vd=VD, post=0)
cfg("sr_classic_again", "classic", vd=VD)
cfg("sr_m0_again", mask=0, vd=VD)
# resolution / MSAA scaling
for sc, ms in [(1.0, 4), (1.0, 0), (2.0, 4)]:
    cfg(f"sr_classic_s{sc}_msaa{ms}", "classic", scale=sc, msaa=ms, vd=VD)
    cfg(f"sr_m0_s{sc}_msaa{ms}", mask=0, scale=sc, msaa=ms, vd=VD)
    cfg(f"sr_unlit_s{sc}_msaa{ms}", mask=128, scale=sc, msaa=ms, vd=VD)
# view distance
for vd in (2000, 1000):
    cfg(f"sr_classic_vd{vd}", "classic", vd=vd)
    cfg(f"sr_m0_vd{vd}", mask=0, vd=vd)
    cfg(f"sr_unlit_vd{vd}", mask=128, vd=vd)

# ---------------------------------------------------------------- noon_fields
view('["noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500]',
     ['[["Jeep"], 13, 3, 60, "veh"]', '[["SoldierWB"], 8, -2, 160, "man"]'])
cfg("nf_classic", "classic", vd=2500)
cfg("nf_m0", mask=0, vd=2500)
cfg("nf_unlit", mask=128, vd=2500)
cfg("nf_shadowoff", mask=1, vd=2500)
cfg("nf_terrainoff", mask=2, vd=2500)
cfg("nf_allfeaturesoff", mask=127, vd=2500)
lines.append("[] call capClear\n")

# ---------------------------------------------------------------- lowsun_fp
view('["lowsun_fp", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500]',
     ['[["Jeep"], 10, -5, -107.5, "veh"]', '[["M113"], 16, 5, 72.5, "veh"]',
      '[["SoldierWB"], 7, 1, -112.5, "man"]', '[["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"]'])
cfg("lf_classic", "classic", vd=1500)
cfg("lf_m0", mask=0, vd=1500)
cfg("lf_unlit", mask=128, vd=1500)
cfg("lf_shadowoff", mask=1, vd=1500)
cfg("lf_allfeaturesoff", mask=127, vd=1500)
lines.append("[] call capClear\n")

lines.append('\ncall { logInfo "UDPERF;event=done" }\ntriEndTest\n')
out.with_suffix(".test.sqf").write_text("".join(lines))

toml = f'''# PERF RESEARCH (not committed): remastered GPU cost diagnosis.
binary = "PoseidonGame"
mission = "tests/rendering/captures/missions/udcap_abel.Abel"
tags = ["lighting_capture", "headful", "exclusive"]
timeout = 900
assert_timeout = 180
extra_args = ["--no-strict", "--display-mode", "windowed", "--width", "960", "--height", "640"]
'''
out.with_suffix(".test.toml").write_text(toml)
print("wrote", out.with_suffix(".test.sqf"), out.with_suffix(".test.toml"))
