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


# ---------------------------------------------------------------- sunrise_ridge, repeated rounds
view('["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000]', settle=600)
import itertools
A = [("classic", "classic", 0), ("m0", "remastered", 0), ("unlit", "remastered", 128),
     ("allfeatoff", "remastered", 127), ("shadowoff", "remastered", 1), ("terrainoff", "remastered", 2)]
for r in range(4):
    for tag, mode, mask in A:
        cfg(f"A{r}_{tag}", mode, mask=mask, scale=1.5, msaa=4, vd=4000, frames=120)
B = [("classic", "classic", 0), ("m0", "remastered", 0), ("unlit", "remastered", 128), ("allfeatoff", "remastered", 127)]
for r in range(3):
    for tag, mode, mask in B:
        cfg(f"B{r}_{tag}", mode, mask=mask, scale=2.0, msaa=8, vd=4000, frames=120)
C = A
for r in range(3):
    for tag, mode, mask in C:
        cfg(f"C{r}_{tag}", mode, mask=mask, scale=2.0, msaa=8, vd=1000, frames=120)
lines.append('\ncall { logInfo "UDPERF;event=done" }\ntriEndTest\n')
out.with_suffix(".test.sqf").write_text("".join(lines))
toml = """# PERF RESEARCH (not committed): remastered GPU cost diagnosis, repeated rounds.
binary = "PoseidonGame"
mission = "tests/rendering/captures/missions/udcap_abel.Abel"
tags = ["lighting_capture", "headful", "exclusive"]
timeout = 900
assert_timeout = 180
extra_args = ["--no-strict", "--display-mode", "windowed", "--width", "960", "--height", "640"]
"""
out.with_suffix(".test.toml").write_text(toml)
print("wrote", out)
