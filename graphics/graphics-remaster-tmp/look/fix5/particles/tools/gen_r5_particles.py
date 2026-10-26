"""Round 5 particles fixer's capture tests (not part of the patch).
usage: gen_r5_particles.py <engine root> <tag> <setting>
Writes look_r5_<tag>_ff.test.sqf (the abel_lighting firefight, view 7, one mode, timing-pinned as the
patched abel_lighting stages it) and look_r5_<tag>_views.test.sqf (town_night, ffh by night and ffh_day,
a burning Ural and a camp fire at 15:00) next to the capture tools."""
import sys
from pathlib import Path

eng = Path(sys.argv[1])
cap = eng / "tests/rendering/captures"
tag = sys.argv[2]
setting = sys.argv[3]
sys.path.insert(0, str(cap))
import capgen  # noqa: E402
import gen_look_director as gld  # noqa: E402

src = (cap / "abel_lighting.test.sqf").read_text()
head = src[: src.index("// ----------------------------------------------------------------------------\n//  1.")]
ff = src[src.index("//  7. firefight"):]
ff_def = ff[: ff.index('[] call capFFStage\n"classic" call capMode')]
half = ff[ff.index('[] call capFFStage\n"remastered" call capMode'):]
half = half[: half.index("[] call capFFClear") + len("[] call capFFClear")]
half = half.replace('"remastered" call capMode', f'"{setting}" call capMode')
out = [head, "// ----", ff_def, half, 'call { "view=done;event=done" call capLog }', "triEndTest"]
name = f"look_r5_{tag}_ff"
(cap / f"{name}.test.sqf").write_text("\n".join(out) + "\n", newline="\n")
(cap / f"{name}.test.toml").write_text((cap / "abel_lighting.test.toml").read_text(), newline="\n")
tests = [f"tests/rendering/captures/{name}.test.sqf"]

vs = [gld.t("town_night"), gld.t("ffh"), gld.t("ffh", name="ffh_day", date=[1985, 7, 12, 15, 0])]
lines = [capgen.header("Abel", vs[0], "round 5 particles fixer views")]
SIMT = {"town_night": 7400}
for i, v in enumerate(vs):
    lines.append(f"\n// ---- {v['name']}")
    lines += capgen.begin(v, v["name"], False, sim_time=SIMT.get(v["name"], gld.SIM_T0 + gld.SIM_DT * (20 + i)))
    lines.append("call { capFixRes = triSetFixedStep 0.0333333 }")
    lines.append("triSimFrames 150")
    lines += gld.shot(v, setting, f"{v['name']}__{tag}", 90)
    lines.append("call { capFixRes = triSetFixedStep 0 }")
    lines += capgen.end(v, False)
lines.append('\ncall { "view=done;event=done" call capLog }')
lines.append("triEndTest")
name = f"look_r5_{tag}_views"
capgen.write(cap / f"{name}.test.sqf", "\n".join(lines) + "\n")
capgen.write(cap / f"{name}.test.toml", capgen.toml("Abel", "round 5 particles views", gld.views.WORLDS, timeout=5400))
tests.append(f"tests/rendering/captures/{name}.test.sqf")
print(",".join(tests))
