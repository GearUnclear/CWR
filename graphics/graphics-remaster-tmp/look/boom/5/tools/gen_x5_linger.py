"""Extra probe for approach 5 (time and lifetimes): the HE shell and the Jeep's fuel explosion together at dusk,
one shot every 10 frames (1/3 s) for 24 s, so the smoke column's whole life is seen. Built from the shared
generator's pieces (LOOK/boom/gen_munition_clips.py). usage: gen_x5_linger.py <engine root>"""
import sys, importlib.util
from pathlib import Path
eng = Path(sys.argv[1]); cap = eng / "tests/rendering/captures"
spec = importlib.util.spec_from_file_location("g", "D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/boom/gen_munition_clips.py")
src = Path("D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/boom/gen_munition_clips.py").read_text()
src = src[: src.index('test("dusk"')]
sys.argv = ["x", str(eng)]
ns = {}
exec(compile(src, "gen", "exec"), ns)
out = [ns["head"], ns["ff"], ns["LIB"], "[] call capFFStage", '"remastered" call capMode', "triSimFrames 200",
       "call { [] call capPinEnv; [] call capPinView; [] call capBurn }", "0.0333333 call capFixStep"]
n = 0
for i in range(4):
    out += ["triSimFrames 1", f'triScreenshot "x5_linger_f{n:03d}"']; n += 1
out.append("call { " + ns["CLIPS"]["he"][1] + " }")
out.append("call { " + ns["CLIPS"]["fuel"][1] + " }")
for i in range(72):
    out += ["triSimFrames 10", f'triScreenshot "x5_linger_f{n:03d}"']; n += 1
out += ["0 call capFixStep", "[] call capFFClear", 'call { "view=done;event=done" call capLog }', "triEndTest"]
(cap / "look_probe_x5_linger.test.sqf").write_text("\n".join(out) + "\n", newline="\n")
(cap / "look_probe_x5_linger.test.toml").write_text((cap / "abel_lighting.test.toml").read_text(), newline="\n")
print("frames", n)
