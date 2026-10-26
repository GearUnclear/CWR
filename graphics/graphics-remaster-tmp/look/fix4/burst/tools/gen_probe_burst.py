"""Burst probe generator (round 4 burst fixer, not part of the patch).
The firefight of abel_lighting view 7, remastered only, fixed step 1/30 s,
one shell dropped, shot paused at several frame counts after the drop with
each knob variant.
usage: gen_probe_burst.py <engine root> <tag> "<label>=<setting>;..." <frames comma list> [night] [spot fwd,right]"""
import sys
from pathlib import Path

eng = Path(sys.argv[1])
cap = eng / "tests/rendering/captures"
tag = sys.argv[2]
variants = [v.split("=", 1) for v in sys.argv[3].split(";") if v.strip()]
frames = [int(x) for x in sys.argv[4].split(",")]
night = len(sys.argv) > 5 and sys.argv[5] == "night"
spot = sys.argv[6] if len(sys.argv) > 6 else "58, -32"

src = (cap / "abel_lighting.test.sqf").read_text()
head = src[: src.index("// ----------------------------------------------------------------------------\n//  1.")]
head = head.replace("triSimUntil { alive player }",
                    "triSimUntil { alive player }\ncapPause = { capPauseRes = triPauseGame }\n"
                    "capUnpause = { capPauseRes = triUnpauseGame }\ncapFixStep = { capFixRes = triSetFixedStep _this }\n"
                    "capModeQuick = { capModeName = _this; capLit = triSetLighting capModeName }")
ff = src[src.index("//  7. firefight"):]
ff = ff[: ff.index('[] call capFFStage\n"classic" call capMode')]
if night:
    ff = ff.replace("[1985, 7, 5, 19, 35]", "[1985, 7, 5, 23, 0]")
out = [head, "// ----", ff, "[] call capFFStage", '"remastered" call capMode', "0.0333333 call capFixStep",
       "[] call capEngage", "triSimFrames 200",
       "call { [] call capPinEnv; [] call capPinView; [] call capEngage; [] call capBurn }",
       f"[{spot}] call capBoom"]
done = 0
for f in frames:
    out += [f"triSimFrames {f - done}", "[] call capPause"]
    done = f
    for label, setting in variants:
        name = f"{tag}_f{f:02d}__{label}"
        out += [f'"{setting}" call capModeQuick', "triSimFrames 6", f'triScreenshot "{name}"', f'"{name}" call capRecord']
    out += ['"remastered" call capModeQuick', "[] call capUnpause"]
out += ["[] call capFFClear", 'call { "view=done;event=done" call capLog }', "triEndTest"]
name = f"look_probe_{tag}"
(cap / f"{name}.test.sqf").write_text("\n".join(out) + "\n", newline="\n")
(cap / f"{name}.test.toml").write_text((cap / "abel_lighting.test.toml").read_text(), newline="\n")
print(f"tests/rendering/captures/{name}.test.sqf")
