"""Agent 2 head-wiggle probe: six posed men side by side, each with a different
per-frame pinning regime, fixed camera; frames shot exactly as the v2 clips
(capClockAt, capPathAt, triSimFrames 2, triScreenshot)."""
import sys
from pathlib import Path
VW = Path("D:/Arma_CWA_worktrees/video-work/arma_CWR/tests/rendering/captures/video")
side = sys.argv[1] if len(sys.argv) > 1 else "ud"
VD = Path("D:/Arma_CWA_worktrees/vanilla-upstream/tests/rendering/captures/video")
src = ((VW / "ud_outpost.test.sqf") if side == "ud" else (VD / "van_outpost.test.sqf")).read_text().split("\n")
name = sys.argv[2] if len(sys.argv) > 2 else "pin"
n = int(sys.argv[3]) if len(sys.argv) > 3 else 120
extra_mode = sys.argv[4] if len(sys.argv) > 4 else ""
SIM = 2
# header: everything up to and including the capPose helper
end = next(i for i, l in enumerate(src) if l.startswith("// ---- outpost"))
head = src[:end]
out = list(head)
out[0] = f"// agent 2 head-wiggle probe ({name}, {side}): six men, per-frame pinning variants, {n} frames"
lab = f"h2{name}"
out += [
 "// ---- probe",
 "call { capRain = 0; capEyeAbs = false }",
 '["h2probe", 7300, 10900, 1.75, 152.5, 0, [1985, 7, 5, 12, 0], 0, 0, 800] call capBegin',
 "call { capSimT = 2000; capSimRes = triSetSimTime capSimT }",
 "call { capH2 = []; capGs = [] }",
 "call {",
 "  { private [\"_g\", \"_u\", \"_p\"]; _g = createGroup east; capGs = capGs + [_g];",
 "    _u = [[\"SoldierEB\"], 4, _x, 180, \"man\", _g] call capSpawn;",
 "    capH2 = capH2 + [_u] } forEach [-2.6, -1.9, -1.2, -0.5];",
 "}",
 "[] call capStageSave",
 "call { capMovers = [] }",
 'capMover1 = [["UAZ", "Jeep"], 9, 1.5, 90, "veh"] call capSpawn',
 'if (not (isNull capMover1)) then { capMover1 lock false; "SoldierEB" createUnit [[capStageX, capStageZ, 0], capGrpE, "capNewUnit = this", 0.6, "PRIVATE"]; capNewUnit setCaptive true; capNewUnit allowDamage false; capNewUnit moveInDriver capMover1; capNewUnit disableAI "MOVE"; capNewUnit setBehaviour "SAFE" }',
 "[capMover1, [[0, 9, 1.5, 90], [%d, 9, 4.0, 90]]] call capMoverAdd" % (n - 1),
 "call { capPathKind = \"line\"; capPathA = [7300, 10900, 1.75, 152.5, 0]; capPathB = [7300, 10900, 1.75, 152.5, 0]; capN = %d; capLapse = 0 }" % n,
]
if side == "ud":
    out += ["call { capSettleArgs = [150, 30, 2] }"] + ["triSimFrames 90"] * 4 + ['"remastered" call capMode', "[] call capPin"]
else:
    out += ["call { capSettleFrames = 90 }"] + ["triSimFrames 90"] * 6 + ['"vanilla" call capMode', "[] call capPin"]
out += [
]
if extra_mode == "fixed" and side == "ud":
    out += ['call { capFx = triSetFixedStep (1/120); format ["view=h2probe;event=fixedstep;res=%1", capFx] call capLog }']
out += [
 # record the pinned spots once, after capPin
 "call { capH2S = []; { capH2S = capH2S + [[getPos _x, getDir _x, getPosASL _x]] } forEach capH2 }",
 "call { format [\"view=h2probe;event=spots;men=%1;spots=%2\", capH2, capH2S] call capLog }",
 # variant pinning: 0 control, 1 setDir+setPos, 2 vel0+setDir+setPos, 3 setPos only, 4 setDir only, 5 setPosASL only
 "capH2Pin = {",
 "  private [\"_s\", \"_u\"];",
 "  _u = capH2 select 1; _s = capH2S select 1; _u setDir (_s select 1); _u setPos [(_s select 0) select 0, (_s select 0) select 1, 0];",
 "  _u = capH2 select 2; _s = capH2S select 2; _u setVelocity [0, 0, 0]; _u setDir (_s select 1); _u setPos [(_s select 0) select 0, (_s select 0) select 1, 0];",
 "  _u = capH2 select 3; _s = capH2S select 3; _u setPosASL (_s select 2);",
 "}",
]
for k in range(n):
    out += [f"[{k}] call capClockAt", f"[{k}, {n}] call capPathAt", f"[{k}] call capPropsAt", "[] call capH2Pin", f"triSimFrames {SIM}",
            f'triScreenshot "{lab}__{side}__f{k+1:04d}"']
    if k % 30 == 0:
        out.append(f'call {{ format ["view=h2probe;event=frame;k={k};pos=%1", capH2 apply {{ 0 }}] call capLog }}' if False else
                   f'call {{ private ["_a"]; _a = []; {{ _a = _a + [[getPosASL _x, getDir _x]] }} forEach capH2; format ["view=h2probe;event=frame;k={k};men=%1", _a] call capLog }}')
if extra_mode == "fixed" and side == "ud":
    out.append("call { capFx = triSetFixedStep 0 }")
out += ["[] call capClear", 'call { "view=done;event=done" call capLog }', "triEndTest"]
text = "\n".join(out) + "\n"
if side != "ud":
    import re
    text = text.replace("triCaptureAtPresent true", "// (vanilla: no triCaptureAtPresent)")
    text = text.replace("\ntriSimFrames 240\n", "\ntriSimFrames 120\ntriSimFrames 120\n")
toml = (VW / "ud_outpost.test.toml").read_text().replace("Video v2 clip outpost", "agent 2 head probe").replace("timeout = 3600", "timeout = 900")
if side == "ud":
    d = VW; fn = f"look_probe_head2_{name}"
else:
    d = Path("D:/Arma_CWA_worktrees/vanilla-upstream/tests/rendering/captures/video"); fn = f"van_probe_head2_{name}"
    vt = (d / "van_outpost.test.toml").read_text()
    toml = vt.replace("timeout = 3600", "timeout = 900")
(d / f"{fn}.test.sqf").write_text(text, newline="\n")
(d / f"{fn}.test.toml").write_text(toml, newline="\n")
print(d / f"{fn}.test.sqf", len(out))
