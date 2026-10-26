"""Probe test generator for the horizon fixer (not part of the patch): the
firefight of abel_lighting view 7, remastered only, shot in P mode (paused,
same frame) with several variants."""
import sys
from pathlib import Path

HERE = Path(__file__).parent
tag = sys.argv[1]
variants = [v.split("=", 1) for v in sys.argv[2].split(";") if v.strip()]
stills = sys.argv[3].split(",") if len(sys.argv) > 3 else ["s1", "s2", "s3"]
night = len(sys.argv) > 4 and sys.argv[4] == "night"

src = (HERE / "abel_lighting.test.sqf").read_text()
head = src[: src.index("// ----------------------------------------------------------------------------\n//  1.")]
head = head.replace("triSimUntil { alive player }",
                    "triSimUntil { alive player }\ncapPause = { capPauseRes = triPauseGame }\n"
                    "capUnpause = { capPauseRes = triUnpauseGame }\ncapFixStep = { capFixRes = triSetFixedStep _this }\n"
                    "capModeQuick = { capModeName = _this; capLit = triSetLighting capModeName }")
ff = src[src.index("//  7. firefight"):]
ff = ff[: ff.index('[] call capFFStage\n"classic" call capMode')]
date = "[1985, 7, 5, 19, 35]"
if night:
    ff = ff.replace(date, "[1985, 7, 5, 23, 0]")
out = [head, "// ----", ff, "[] call capFFStage", '"remastered" call capMode', "0.0333333 call capFixStep",
       "[] call capEngage", "triSimFrames 200",
       "call { [] call capPinEnv; [] call capPinView; [] call capEngage; [] call capBurn }"]


def shots(still):
    o = ["[] call capPause"]
    for label, setting in variants:
        name = f"{tag}_{still}__{label}"
        o += [f'"{setting}" call capModeQuick', "triSimFrames 8", f'triScreenshot "{name}"', f'"{name}" call capRecord']
    o += ['"remastered" call capModeQuick', "[] call capUnpause"]
    return o


if "s1" in stills:
    out += ["[62, -2] call capBoom", "triSimFrames 5"] + shots("s1")
if "s2" in stills:
    out += ["[] call capEngage", "[58, -32] call capBoom", "triSimFrames 6"] + shots("s2")
if "s3" in stills:
    out += ["[] call capEngage", "triSimFrames 20"] + shots("s3")
if "s4" in stills:
    out += ["[] call capEngage", "triSimFrames 13"] + shots("s4")
out += ["[] call capFFClear", 'call { "view=done;event=done" call capLog }', "triEndTest"]
name = f"look_probe_{tag}"
(HERE / f"{name}.test.sqf").write_text("\n".join(out) + "\n", newline="\n")
toml = (HERE / "abel_lighting.test.toml").read_text().replace("timeout = 900", "timeout = 900")
(HERE / f"{name}.test.toml").write_text(toml, newline="\n")
print(f"tests/rendering/captures/{name}.test.sqf")
