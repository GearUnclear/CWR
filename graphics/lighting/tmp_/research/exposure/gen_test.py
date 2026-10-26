"""Generate the exposure-track research capture test (Abel views).

    python gen_test.py <round name> <out .test.sqf>

Each view: Classic, Remastered (current, research knobs at their neutral
defaults), then the variants of the round. Labels are <view>__<variant>.
triScreenshot must be a top-level statement with a literal label, so every
shot is spelled out here.
"""
import sys
from pathlib import Path

# Research knobs and their neutral defaults (LightRig.hpp, track exposure).
RESET = ["hdr=0", "ltmShadow=1", "ltmHighlight=1", "ltmDetail=1", "ltmBlur=0.4", "ltmMid=-2.47",
         "ltmMidAdapt=0", "ltmMaxGain=3", "ltmMinGain=0.5", "ltmNight=0", "ltmSigma=1", "adapt=0",
         "adaptKey=-2", "adaptSpeed=1.5", "adaptMin=-1", "adaptMax=1.5", "tmCurve=0", "bloomHdr=0",
         "ltmDebug=0", "hdrWhite=24", "exposure=1"]

L1 = ["ltmShadow=0.65", "ltmHighlight=0.9", "ltmBlur=0.3", "ltmMaxGain=2.5"]
SEQ_KNOBS: dict = {}
L3 = ["ltmShadow=0.6", "ltmHighlight=0.9", "ltmDetail=1.15", "ltmBlur=0.3", "ltmMaxGain=2.5", "ltmMidAdapt=1"]
L3S = L3 + ["bloomHdr=1"]
P1 = ["exposure=1.3", "ltmShadow=0.65", "ltmHighlight=0.7", "ltmDetail=1.2", "ltmBlur=0.3", "ltmMaxGain=2.5",
      "ltmMinGain=0.5", "ltmMidAdapt=1", "bloomHdr=1"]
ADAPT = ["adapt=1", "adaptKey=-4", "hdrWhite=0.75", "adaptMin=0", "adaptMax=0.75", "adaptSpeed=30"]
ROUNDS = {
    "r1": {
        "L1": L1,
        "L2": ["ltmShadow=0.65", "ltmHighlight=0.9", "ltmBlur=0.3", "ltmMaxGain=2.5", "ltmDetail=1.3"],
        "A1": L1 + ["adapt=1", "adaptKey=-2", "adaptSpeed=30"],
        "H1": ["hdr=1"] + L1,
        "H2": ["hdr=1", "tmCurve=2"] + L1,
        "H3": ["hdr=1", "tmCurve=1"] + L1,
        "G1": L1 + ["adapt=1", "adaptKey=-2", "adaptSpeed=30", "ltmDebug=1"],
    },
    # r2 runs with override_r2/psPostComposite.glsl: ltmMidAdapt is the SHADE GATE.
    "r2": {
        "L3": L3,
        "L4": [k for k in L3 if not k.startswith("ltmMidAdapt")],
        "L5": [k for k in L3 if not k.startswith("ltmShadow")] + ["ltmShadow=0.45"],
        "M1": L3 + ["ltmDebug=3"],
        "G2": L3 + ["ltmDebug=1"],
    },
    # r3 runs with override_r3: ltmMidAdapt = shade gate, bloomHdr = sky protect,
    # hdrWhite = adaptation dead band (EV).
    "r3": {
        "L3s": L3S,
        "K1": L3S + ["tmCurve=2"],
        "P1": P1,
        "P2": P1 + ["tmCurve=2"],
        "AD": L3S + ADAPT,
        "PA": P1 + ["tmCurve=2"] + ADAPT,
    },
}
N2 = [k for k in L3S if not k.startswith("ltmMidAdapt")] + ["ltmMidAdapt=2"]
ROUNDS["r4"] = {
    "N2": N2,
    "N3": [k for k in N2 if not k.startswith("ltmShadow")] + ["ltmShadow=0.5"],
    "N2K": N2 + ["tmCurve=2"],
    "NG": N2 + ["ltmDebug=1"],
}
NIGHT_R4 = {
    "NK": ["tmCurve=2", "ltmNight=1"],
    "NM": ["tmCurve=2", "ltmNight=1", "ltmDebug=3", "ltmShadow=0.999"],
    "NH": ["tmCurve=2", "ltmNight=1", "ltmHighlight=0.75", "ltmDetail=1.1", "ltmMid=-4.5", "ltmBlur=0.3", "bloomHdr=1"],
    "NA": ["tmCurve=2", "ltmNight=1", "ltmHighlight=0.75", "ltmDetail=1.1", "ltmMid=-4.5", "ltmBlur=0.3", "bloomHdr=1",
           "adapt=1", "adaptKey=-6", "hdrWhite=0.5", "adaptMin=-0.75", "adaptMax=0", "adaptSpeed=30"],
}
SEQ_KNOBS["r4"] = N2 + ["adaptKey=-4", "hdrWhite=0.75", "adaptMin=0", "adaptMax=1"]
F1 = ["ltmShadow=0.6", "ltmHighlight=0.9", "ltmDetail=1.15", "ltmBlur=0.3", "ltmMaxGain=2.5", "ltmMid=-2.8",
      "ltmMidAdapt=2", "bloomHdr=1"]
ROUNDS["r5"] = {
    "F1": F1,
    "F2": [k for k in F1 if not k.startswith("ltmShadow")] + ["ltmShadow=0.5"],
    "F1K": F1 + ["tmCurve=2"],
    "F1A": F1 + ["tmCurve=2", "adapt=1", "adaptKey=-4", "hdrWhite=0.75", "adaptMin=-0.75", "adaptMax=0.5", "adaptSpeed=30"],
    "FG": F1 + ["ltmDebug=1"],
}
SEQ_KNOBS["r5"] = F1 + ["adaptKey=-4", "hdrWhite=0.75", "adaptMin=0", "adaptMax=1", "ltmDebug=3"]
SEQ_SPEED = {"r5": "0.6", "r6": "0.4"}
SEQ_KNOBS["r6"] = SEQ_KNOBS["r5"]
ROUNDS["r6"] = {}
SEQ_FRAMES = {"r6": ((1, "f001"), (10, "f011"), (20, "f031"), (40, "f071"), (80, "f151"), (160, "f311")), "r5": ((1, "f001"), (10, "f011"), (20, "f031"), (40, "f071"), (80, "f151"), (160, "f311"))}
SEQ_KNOBS["r3"] = P1 + ["tmCurve=2", "adaptKey=-4", "hdrWhite=0.75", "adaptMin=0", "adaptMax=0.75"]


VIEWS = {
    "lowsun_fp": {
        "begin": '["lowsun_fp", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin',
        "props": ['[["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;',
                  '[["M113"], 16, 5, 72.5, "veh"] call capSpawn;',
                  '[["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;',
                  '[["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;'],
    },
    "sunrise_ridge": {
        "begin": '["sunrise_ridge", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin',
        "props": [],
    },
    "coastline_sunset": {
        "begin": '["coastline_sunset", 1550, 5150, 3, 278, -4, [1985, 7, 5, 18, 35], 0, 0, 4000] call capBegin',
        "props": [],
    },
    "noon_fields": {
        "begin": '["noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin',
        "props": ['[["Jeep"], 13, 3, 60, "veh"] call capSpawn;',
                  '[["SoldierWB"], 8, -2, 160, "man"] call capSpawn;'],
    },
    "forest_edge_golden": {
        "begin": '["forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin',
        "props": ['[["SoldierWB"], 12, 2, 215, "man"] call capSpawn;'],
    },
    "town_night": {
        "begin": '["town_night", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0, 800] call capBegin',
        "props": ['capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;',
                  'if (not (isNull capFire)) then { capFire inflame true };'],
        "night": True,
    },
}

HEADER = r'''// ============================================================================
//  RESEARCH (track exposure): local exposure, eye adaptation, HDR frame and
//  tonemap curves. Generated by tmp/research/exposure/gen_test.py; not committed.
// ============================================================================

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
capFire = objNull

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

capResearchReset = [%RESET%]
// [knob strings] call capSetKnobs: reset every research knob, then apply these.
capSetKnobs = {
    private ["_r", "_bad"];
    _bad = "";
    { _r = triSetLighting _x; if (_r != "OK") then { _bad = _bad + _x + "=>" + _r + " " } } forEach (capResearchReset + _this);
    capLit = "OK";
    if (_bad != "") then { capLit = _bad };
}
// [name, [knobs]] call capVariant: Remastered with these research knobs.
capVariant = {
    capModeName = _this select 0;
    (_this select 1) call capSetKnobs;
    capGpuOff = triGpuTimes false;
    capGpuOn = triGpuTimes true;
    capPerfRes = triPerfReset 0;
    format ["view=%1;event=mode;mode=%2;lit=%3;knobs=%4", capView, capModeName, capLit, _this select 1] call capLog;
}

triSimFrames 240
call { "view=setup;event=setup;world=Abel;msaa=4;scale=1.5;window=960x640;capture=present" call capLog }
'''


def sqf_list(items):
    return "[" + ", ".join('"%s"' % s for s in items) + "]"


def shot(view, label, fire):
    out = ["triSimFrames 90", "[] call capReadTimers", "[] call capPin"]
    if fire:
        out.append("call { if (not (isNull capFire)) then { capFire inflame true } }")
    out += ["triSimFrames 3", f'triScreenshot "{view}__{label}"', f'"{view}__{label}" call capRecord']
    return out


TANK = r'''// ---- tank_gunner_interior (player in the M60 gunner seat, INTERNAL camera)
["tank_gunner_interior", 7300, 10900, 1.75, 152.5, 0, [1985, 7, 5, 10, 0], 0, 0, 1500] call capBegin
call {
    [["M113"], 70, 4, 90, "veh"] call capSpawn;
    [["Jeep"], 55, -10, -60, "veh"] call capSpawn;
    [["SoldierWB"], 50, 0, 180, "man"] call capSpawn;
}
call {
    capUseView = false;
    capCam cameraEffect ["terminate", "back"];
    capClearRes = triClearView;
    capTankCls = ["M60", "M1Abrams", "T72"] call capPick;
    capTank = objNull;
    if (capTankCls != "") then { capTank = capTankCls createVehicle [7300, 10900, 0] };
    if (not (isNull capTank)) then {
        capTank setDir 152.5;
        capTank setPos [7300, 10900, 0];
        capTank allowDamage false;
        player moveInGunner capTank;
    };
    format ["view=tank_gunner_interior;event=tank;class=%1;inTank=%2", capTankCls, not ((vehicle player) == player)] call capLog;
}
triSimFrames 60
call { (vehicle player) switchCamera "INTERNAL" }
triSimFrames 60
'''


def adapt_seq(knobs, speed="1.5", frames=((1, "f001"), (8, "f009"), (16, "f025"), (32, "f057"), (64, "f121"), (160, "f281"))):
    """Eye adaptation over time: settle on the sunrise view (sun and bright
    sky in frame), then tip the camera down at the dark foreground and shoot
    the frames after the cut. Remastered (current) first for reference."""
    out = ['// ---- adapt_seq: sunrise_ridge, look at the sun, then down at the ground',
           '["adapt_seq", 4400, 3300, 1.75, 60, 2, [1985, 7, 5, 5, 8], 0, 0, 4000] call capBegin',
           'triSimFrames 150',
           '"remastered" call capMode',
           'call { [] call capSetKnobs }',
           'call { capP = -12 }']
    out += shot("adapt_seq", "remastered_down", False)
    out.append('call { capP = 20 }')
    out.append('[] call capPin')
    out.append(f'["seq", {sqf_list(knobs + ["adapt=1", "adaptSpeed=" + speed])}] call capVariant')
    out += ["triSimFrames 150", "[] call capPin", "triSimFrames 2",
            'triScreenshot "adapt_seq__up_settled"', '"adapt_seq__up_settled" call capRecord',
            'call { capP = -12 }', '[] call capPin']
    for n, lab in frames:
        out += [f"triSimFrames {n}", f'triScreenshot "adapt_seq__{lab}"', f'"adapt_seq__{lab}" call capRecord']
    out.append("call { [] call capSetKnobs }")
    return out


def main():
    rnd, dst = sys.argv[1], Path(sys.argv[2])
    views = sys.argv[3].split(",") if len(sys.argv) > 3 else list(VIEWS)
    variants = ROUNDS[rnd]
    lines = [HEADER.replace("%RESET%", ", ".join('"%s"' % s for s in RESET))]
    for v in views:
        if v.startswith("adapt_seq"):
            lines += adapt_seq(SEQ_KNOBS.get(rnd, []), SEQ_SPEED.get(rnd, "1.5"), *([SEQ_FRAMES[rnd]] if rnd in SEQ_FRAMES else []))
            continue
        if v == "tank_gunner_interior":
            lines.append(TANK)
            lines.append('"classic" call capMode')
            lines += shot(v, "classic", False)
            lines.append('"remastered" call capMode')
            lines.append("call { [] call capSetKnobs }")
            lines += shot(v, "remastered", False)
            for name, knobs in variants.items():
                lines.append(f'["{name}", {sqf_list(knobs)}] call capVariant')
                lines += shot(v, name, False)
            lines.append("call { [] call capSetKnobs }")
            continue
        spec = VIEWS[v]
        fire = bool(spec.get("night"))
        lines.append(f"// ---- {v}")
        lines.append(spec["begin"])
        if spec["props"]:
            lines.append("call {\n    " + "\n    ".join(spec["props"]) + "\n}")
        lines.append("triSimFrames 150")
        lines.append('"classic" call capMode')
        lines += shot(v, "classic", fire)
        lines.append('"remastered" call capMode')
        lines.append("call { [] call capSetKnobs }")
        lines += shot(v, "remastered", fire)
        vv = variants
        if fire and rnd == "r4":
            vv = NIGHT_R4
        for name, knobs in vv.items():
            k = list(knobs)
            if fire and "ltmNight=1" not in k and rnd != "r5":
                k = k + ["ltmNight=1"]
            lines.append(f'["{name}", {sqf_list(k)}] call capVariant')
            lines += shot(v, name, fire)
        lines.append("call { [] call capSetKnobs }")
        lines.append("[] call capClear")
        lines.append("")
    lines.append('call { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    dst.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
    print(f"wrote {dst} ({len(views)} views x {len(variants) + 2} shots)")


if __name__ == "__main__":
    main()
