"""Agent 2 probe 3: side effects of the setAccTime step recipe (agent 3) on fire,
a burning wreck, a flare, a shell burst and a teleported UAZ with its driver,
plus three posed men (control / setDir+setPos per frame / capPinProps style).
gen_fx.py <ud|vanilla> <base|acc> [frames]
  base: the clips' loop (triSimFrames 2 per shot, real deltaT)
  acc:  setAccTime 0 for the clip; per shot one block
        call { triWaitFrames 12; setAccTime (1/18); triSimFrames 1; setAccTime 0 }
Every shot logs the sim time the shot's tick(s) advanced (time after - time before)."""
import sys
from pathlib import Path
side, variant = sys.argv[1], sys.argv[2]
n = int(sys.argv[3]) if len(sys.argv) > 3 else 120
VW = Path("D:/Arma_CWA_worktrees/video-work/arma_CWR/tests/rendering/captures/video")
VD = Path("D:/Arma_CWA_worktrees/vanilla-upstream/tests/rendering/captures/video")
src = ((VW / "ud_outpost.test.sqf") if side == "ud" else (VD / "van_outpost.test.sqf")).read_text().split("\n")
end = next(i for i, l in enumerate(src) if l.startswith("// ---- outpost"))
out = list(src[:end])
out[0] = f"// agent 2 probe 3 (fx {variant}, {side}): setAccTime step recipe side effects, {n} frames"
lab = f"h2fx{variant}"
tag = "ud" if side == "ud" else "vanilla"
out += [
 "// ---- probe",
 "call { capRain = 0; capEyeAbs = false }",
 '["h2fx", 7300, 10900, 1.75, 152.5, 0, [1985, 7, 5, 19, 0], 0, 0, 1200] call capBegin',
 "call { capSimT = 2000; capSimRes = triSetSimTime capSimT }",
 "capH2 = []",
 "capShellObj = objNull",
 "call {",
 '  { private ["_g", "_u"]; _g = createGroup east; _u = [["SoldierEB"], 5, _x, 180, "man", _g] call capSpawn; capH2 = capH2 + [_u] } forEach [-3.2, -2.4, -1.6];',
 "}",
 "[] call capStageSave",
 "call { capMovers = [] }",
 'capMover1 = [["UAZ", "Jeep"], 11, 0.5, 90, "veh"] call capSpawn',
 'if (not (isNull capMover1)) then { capMover1 lock false; "SoldierEB" createUnit [[capStageX, capStageZ, 0], capGrpE, "capNewUnit = this", 0.6, "PRIVATE"]; capNewUnit setCaptive true; capNewUnit allowDamage false; capNewUnit moveInDriver capMover1; capNewUnit disableAI "MOVE"; capNewUnit setBehaviour "SAFE"; capDrv = capNewUnit }',
 f"[capMover1, [[0, 11, 0.5, 90], [{n - 1}, 11, 3.5, 90]]] call capMoverAdd",
 'capFire = [["Fire"], 16, 5, 0, "veh"] call capSpawn',
 'if (not (isNull capFire)) then { capFire inflame true }',
 'capBurnV = [["Ural", "Truck5t"], 32, 9, 30, "veh"] call capSpawn',
 'if (not (isNull capBurnV)) then { capBurnV allowDamage true; capBurnV setDammage 1; capBurnV inflame true }',
 f"call {{ capPathKind = \"line\"; capPathA = [7300, 10900, 1.75, 152.5, 0]; capPathB = [7300, 10900, 1.75, 152.5, 0]; capN = {n}; capLapse = 0 }}",
]
if side == "ud":
    out += ["call { capSettleArgs = [150, 30, 2] }"] + ["triSimFrames 90"] * 4 + ['"remastered" call capMode', "[] call capPin"]
else:
    out += ["call { capSettleFrames = 90 }"] + ["triSimFrames 90"] * 6 + ['"vanilla" call capMode', "[] call capPin"]
out += [
 "call { capH2S = []; { capH2S = capH2S + [[getPos _x, getDir _x]] } forEach capH2 }",
 "capH2Pin = {",
 '  private ["_s", "_u"];',
 "  _u = capH2 select 1; _s = capH2S select 1; _u setDir (_s select 1); _u setPos [(_s select 0) select 0, (_s select 0) select 1, 0];",
 "  _u = capH2 select 2; _s = capH2S select 2; _u setVelocity [0, 0, 0]; _u setDir (_s select 1); _u setPos [(_s select 0) select 0, (_s select 0) select 1, 0];",
 "}",
]
if variant == "udfix":
    out.append('call { capStepRes = triSetFixedStep (1/60); setAccTime 0; format ["view=h2fx;event=udfix;step=%1;acc=%2", capStepRes, accTime] call capLog }')
if variant in ("acc", "acc60"):
    out.append('call { setAccTime 0; format ["view=h2fx;event=acc0;acc=%1", accTime] call capLog }')
events = {0: '[30, -18, 45, [0, 0, 12], "Flare"] call capShellAt', 30: '[38, 12, 40, [0, 0, -250], "Shell120"] call capShellAt'}
for k in range(n):
    if k in events:
        out.append(events[k])
    out += [f"[{k}] call capClockAt", f"[{k}, {n}] call capPathAt", f"[{k}] call capPropsAt",
            "call { if (not (isNull capFire)) then { capFire inflame true }; if (not (isNull capBurnV)) then { capBurnV inflame true } }",
            "[] call capH2Pin", "call { capT0 = time }"]
    if variant == "acc":
        out.append("call { triWaitFrames 12; setAccTime (1/18); triSimFrames 1; setAccTime 0 }")
    elif variant == "acc60":
        out.append("call { triWaitFrames 60; setAccTime (1/18); triSimFrames 1; setAccTime 0 }")
    elif variant == "udfix":
        out.append("call { setAccTime 1; triSimFrames 1; setAccTime 0 }")
    else:
        out.append("triSimFrames 2")
    out += ["call { capT1 = time }", f'triScreenshot "{lab}__{tag}__f{k + 1:04d}"',
            f'call {{ format ["view=h2fx;event=step;k={k};dt=%1;time=%2;acc=%3;shell=%4;shellpos=%5;uaz=%6", capT1 - capT0, time, accTime, capShellObj, getPos capShellObj, getPos capMover1] call capLog }}']
if variant in ("acc", "acc60"):
    out.append("call { setAccTime 1 }")
if variant == "udfix":
    out.append("call { setAccTime 1; capStepRes = triSetFixedStep 0 }")
out += ["[] call capClear", 'call { "view=done;event=done" call capLog }', "triEndTest"]
text = "\n".join(out) + "\n"
toml = ((VW / "ud_outpost.test.toml") if side == "ud" else (VD / "van_outpost.test.toml")).read_text().replace("timeout = 3600", "timeout = 900")
d, fn = (VW, f"look_probe_head2_fx{variant}") if side == "ud" else (VD, f"van_probe_head2_fx{variant}")
(d / f"{fn}.test.sqf").write_text(text, newline="\n")
(d / f"{fn}.test.toml").write_text(toml, newline="\n")
print(d / f"{fn}.test.sqf", len(out))
