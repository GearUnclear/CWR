"""Shared munition clip tests for the explosion swarm (orchestrator, not part of any patch).
Staging: abel_lighting view 7 (open plain, riflemen for scale, no engagement), remastered only,
fixed 1/30 s step, every frame shot. Writes look_probe_mun_dusk / look_probe_mun_night
(.test.sqf + .test.toml) into <engine root>/tests/rendering/captures.
Clips (frames after the spawn; PRE calm frames before it):
  he      120 mm class HE shell dropped into [50,-10]   (candidates Shell120, Shell125, Shell105)
  grenade hand grenade dropped at [22,-2]               (GrenadeHand, Grenade)
  rocket  AT rocket driven into the ground at [45,5]    (LAW, RPG, AT4, CarlGustav)
  fuel    a Jeep at [45,12] destroyed: its fuel fireball (Jeep, UAZ)
usage: gen_munition_clips.py <engine root>"""
import sys
from pathlib import Path
eng = Path(sys.argv[1]); cap = eng / "tests/rendering/captures"
PRE = 8
src = (cap / "abel_lighting.test.sqf").read_text()
head = src[: src.index("// ----------------------------------------------------------------------------\n//  1.")]
ff = src[src.index("//  7. firefight"):]
ff = ff[: ff.index('[] call capFFStage\n"classic" call capMode')]
CLIPS = {
    "he": (90, '[["Shell120", "Shell125", "Shell105"], [50, -10], 40, [0, 0, -250]] call capMunition'),
    "grenade": (150, '[["GrenadeHand", "Grenade"], [22, -2], 1, [0, 0, -2]] call capMunition'),
    "rocket": (90, '[["LAW", "RPG", "AT4", "CarlGustav"], [45, 5], 6, [0, 0, -120]] call capMunition'),
    "fuel": (150, 'capFuelV = [["Jeep", "UAZ"], 45, 12, 60, "veh"] call capSpawn; if (not (isNull capFuelV)) then { capFuelV allowDamage true; capFuelV setDammage 1 }; format ["view=munition;event=fuel;veh=%1", capFuelV] call capLog'),
}
LIB = '''// [[classes], [forward, right], height, velocity] call capMunition: the first class that spawns.
capMunition = {
    private ["_cls", "_p", "_h", "_v", "_s"];
    _cls = _this select 0; _p = (_this select 1) call capRel; _h = _this select 2; _v = _this select 3;
    _s = objNull;
    { if (isNull _s) then { _s = createShell [_x, [_p select 0, _p select 1, _h], _v, player, objNull]; if (not (isNull _s)) then { capMunClass = _x } } } forEach _cls;
    format ["view=munition;event=spawn;class=%1;obj=%2", capMunClass, _s] call capLog;
}
capMunClass = "none"
'''
def test(name, clips, night):
    body = ff.replace("[1985, 7, 5, 19, 35]", "[1985, 7, 5, 23, 0]") if night else ff
    out = [head, body, LIB, "[] call capFFStage", '"remastered" call capMode', "triSimFrames 200",
           "call { [] call capPinEnv; [] call capPinView; [] call capBurn }", "0.0333333 call capFixStep"]
    for clip in clips:
        post, spawn = CLIPS[clip]
        n = 0
        for i in range(PRE + post):
            if i == PRE:
                out.append(f"call {{ {spawn} }}")
            n += 1
            out += ["triSimFrames 1", f'triScreenshot "mun_{name}_{clip}_f{n:03d}"']
        out += ["triSimFrames 90"]
    out += ["0 call capFixStep", "[] call capFFClear", 'call { "view=done;event=done" call capLog }', "triEndTest"]
    (cap / f"look_probe_mun_{name}.test.sqf").write_text("\n".join(out) + "\n", newline="\n")
    (cap / f"look_probe_mun_{name}.test.toml").write_text((cap / "abel_lighting.test.toml").read_text(), newline="\n")
    print(f"tests/rendering/captures/look_probe_mun_{name}.test.sqf")
import os
if os.environ.get("ROCKET_PROBE"):
    CLIPS["rocketB"] = (90, '[["RPG", "AT4", "CarlGustav", "Heat120"], [40, 0], 30, [0, 0, -250]] call capMunition')
    CLIPS["rocket"] = (90, '[["LAW"], [45, 5], 30, [0, 0, -250]] call capMunition')
    test("rprobe", ["rocket", "rocketB"], False)
else:
    test("dusk", ["he", "grenade", "rocket", "fuel"], False)
    test("night", ["he", "fuel"], True)
