"""Shared generator for the perf research tests (research only, not committed).

A test is a list of SQF lines. Each config block:
    call { [label, mode, saturation(10 + shader mask), scale, msaa, vd, post, debug(100 + C++ perf flags)] call pfCfg }
    triSimFrames N
    call { "label" call pfRead }
and optionally a top-level screenshot with a literal label.
"""
from pathlib import Path

HEADER = r'''// PERF RESEARCH (not committed): where the remastered frame time goes.
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
pfDbg = "none"

// [label, mode, sat, scale, msaa, vd, post, debug] call pfCfg
pfCfg = {
    pfLabel = _this select 0;
    pfLit = triSetLighting (_this select 1);
    pfSat = triSetLighting (format ["saturation=%1", _this select 2]);
    pfPost = triSetLighting (format ["post=%1", _this select 6]);
    pfDbg = triSetLighting (format ["debug=%1", _this select 7]);
    pfLL = triSetLighting (format ["localLights=%1", _this select 8]);
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

TOML = '''# PERF RESEARCH (not committed): remastered cost diagnosis.
binary = "PoseidonGame"
mission = "tests/rendering/captures/missions/udcap_abel.Abel"
tags = ["lighting_capture", "headful", "exclusive"]
timeout = 900
assert_timeout = 180
extra_args = ["--no-strict", "--display-mode", "windowed", "--width", "960", "--height", "640"]
'''

VIEWS = {
    "sunrise_ridge": ('["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000]', []),
    "noon_fields": ('["noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500]',
                    ['[["Jeep"], 13, 3, 60, "veh"]', '[["SoldierWB"], 8, -2, 160, "man"]']),
    "lowsun_fp": ('["lowsun_fp", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500]',
                  ['[["Jeep"], 10, -5, -107.5, "veh"]', '[["M113"], 16, 5, 72.5, "veh"]',
                   '[["SoldierWB"], 7, 1, -112.5, "man"]',
                   '[["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"]']),
    "town_night": ('["town_night", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0, 800]', []),
    "forest_edge_golden": ('["forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500]',
                           ['[["SoldierWB"], 12, 0, 180, "man"]']),
}


class Test:
    def __init__(self):
        self.lines = [HEADER]

    def view(self, name, settle=150):
        begin, spawns = VIEWS[name]
        self.lines.append(f"\n{begin} call capBegin\n")
        if spawns:
            self.lines.append("call {\n" + "".join(f"    {s} call capSpawn;\n" for s in spawns) + "}\n")
        self.lines.append(f"triSimFrames {settle}\n")

    def clear(self):
        self.lines.append("[] call capClear\n")

    def cfg(self, label, mode="remastered", mask=0, scale=1.5, msaa=4, vd=4000, post=1, frames=120, shot=False,
            debug=0, sat=None, amp=0):
        s = sat if sat is not None else 10 + mask
        ll = 100 + amp if amp > 0 else 1
        self.lines.append(
            f'call {{ ["{label}", "{mode}", {s}, {scale}, {msaa}, {vd}, {post}, {debug}, {ll}] call pfCfg }}\n')
        self.lines.append(f"triSimFrames {frames}\n")
        self.lines.append(f'call {{ "{label}" call pfRead }}\n')
        if shot:
            self.lines.append(f'triScreenshot "{label}"\n')

    def raw(self, text):
        self.lines.append(text)

    def write(self, out):
        out = Path(out)
        self.lines.append('\ncall { logInfo "UDPERF;event=done" }\ntriEndTest\n')
        out.with_suffix(".test.sqf").write_text("".join(self.lines))
        out.with_suffix(".test.toml").write_text(TOML)
        print("wrote", out.with_suffix(".test.sqf"))
