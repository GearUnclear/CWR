"""Mutation check for the B11 contract tests: each mutation breaks one contract
in the scratch copy, the unit tests are rebuilt and run, and the mutation is
reverted. A mutation passes when its named cases turn red.

    python mutate.py
"""
import re
import subprocess
from pathlib import Path

ROOT = Path(r"D:\Arma_CWA_worktrees\gr-B11\arma_CWR")
HERE = Path(__file__).resolve().parent
TESTS = ROOT / r"build\gr-B11-rwdi\tests\unit\engine\Poseidon\PoseidonTests.exe"
TAGS = "[remaster],[Shaders],[GL33],[FeatureSwitches],[FeatureGates],[PostStages],[TemporalAA],[TerrainDetail],[LumaSharpen]"

MUTATIONS = [
    ("kept cap 4x (the research's)", "engine/Poseidon/Graphics/Remaster/TerrainDetail.hpp",
     "constexpr float kKeptCap = 2.0f;", "constexpr float kKeptCap = 4.0f;"),
    ("frame constants ignore the gate", "engine/Poseidon/Graphics/Remaster/TerrainDetail.cpp",
     "if (!gateOn || !FeatureOn(tuning.features, Feature::TerrainDetail))",
     "if (!FeatureOn(tuning.features, Feature::TerrainDetail))"),
    ("shader evaluates to 400 m", "engine/PoseidonGL33/EngineGL33_LightingShaders.cpp",
     "const float kTdMaxDistance = 300.0;", "const float kTdMaxDistance = 400.0;"),
    ("no near fade", "engine/Poseidon/Graphics/Remaster/TerrainDetail.cpp",
     "return Smoothstep(kNearFromLod, kNearToLod, baseLod + std::log2(kZoom));", "return 1.0f;"),
    ("anti-tiling on magnified detail", "engine/Poseidon/Graphics/Remaster/TerrainDetail.cpp",
     "return kAntiTile * std::clamp(lod, 0.0f, 1.0f) * DistanceFade(distance);",
     "return kAntiTile * DistanceFade(distance);"),
    ("luma sharpen not planned", "engine/Poseidon/Graphics/Remaster/PostStages.cpp",
     "    else if (in.lumaSharpen)\n        plan.runs |= Bit(PostStage::Sharpen);", ""),
    ("luma sharpen also on a TAA frame", "engine/Poseidon/Graphics/Remaster/PostStages.cpp",
     "    else if (in.lumaSharpen)", "    if (in.lumaSharpen)"),
    ("composite into the frame target under the luma sharpen", "engine/PoseidonGL33/EngineGL33_PostFX.cpp",
     "DrawTo(plan.Runs(rm::PostStage::Sharpen) ? p.taaInFbo : _ssaaFbo, f.w, f.h);",
     "DrawTo(plan.Runs(rm::PostStage::Temporal) ? p.taaInFbo : _ssaaFbo, f.w, f.h);"),
    ("terrainDetail not landed", "engine/Poseidon/Graphics/Remaster/FeatureSwitches.cpp",
     '"Light and shadow", "B11", true,\n     &FS::terrainDetail', '"Light and shadow", "B11", false,\n     &FS::terrainDetail'),
    ("no land flag in ApplyPassState", "engine/PoseidonGL33/EngineGL33_Queue.cpp",
     "        SelectTerrainDetailLayer(spec);", "        (void)spec;"),
    ("sea flagged as land", "engine/PoseidonGL33/EngineGL33_State.cpp",
     "SetLitTerrainDraw(Poseidon::render::Has(spec.backend, Poseidon::render::Backend::DetailTexture));",
     "SetLitTerrainDraw(true);"),
    ("detail slot declared before the padding", "engine/PoseidonGL33/EngineGL33_LightingShaders.cpp",
     "    vec4 litPlanned[19];  // 58-76: slots of the features still to land (padding)\n"
     "    vec4 litDetail;       // 77: terrain detail: x self-detail strength, y detail kept, z anti-tiling, w land draw\n",
     "    vec4 litDetail;       // 77: terrain detail: x self-detail strength, y detail kept, z anti-tiling, w land draw\n"
     "    vec4 litPlanned[19];  // 58-76: slots of the features still to land (padding)\n"),
    ("the stock albedo expression changed", "engine/PoseidonGL33/EngineGL33_LightingShaders.cpp",
     "vec3 albedo = t0.rgb * constColor.rgb * (detail * selfMod);",
     "vec3 albedo = t0.rgb * constColor.rgb * (detail * selfMod * 1.01);"),
]


def run(cmd, **kw):
    return subprocess.run(cmd, capture_output=True, text=True, errors="replace", **kw)


def main():
    lines = ["# B11 mutation check\n", "| mutation | build | failed cases | failed assertions | cases |", "|---|---|---|---|---|"]
    for name, rel, old, new in MUTATIONS:
        path = ROOT / rel
        text = path.read_bytes().decode("utf-8")
        assert text.count(old) == 1, (name, old)
        path.write_bytes(text.replace(old, new).encode("utf-8"))
        try:
            b = run(["powershell", "-NoProfile", "-File", str(HERE / "build-tests.ps1")])
            if b.returncode != 0:
                tail = " ".join(b.stdout.splitlines()[-3:])[:200]
                lines.append(f"| {name} | FAILED to build ({tail}) | | | |")
                continue
            t = run([str(TESTS), TAGS], cwd="D:\\")
            out = t.stdout
            m = re.search(r"test cases:\s*(\d+)\s*\|\s*(\d+) passed\s*\|\s*(\d+) failed", out)
            a = re.search(r"assertions:\s*(\d+)\s*\|\s*(\d+) passed\s*\|\s*(\d+) failed", out)
            cases = sorted(set(re.findall(r"^-{79}\n(.+)\n-{79}", out, re.M)))
            failed_cases = m.group(3) if m else ("0" if "All tests passed" in out else "?")
            failed_asserts = a.group(3) if a else "0"
            (HERE / f"mut_{len(lines):02d}.log").write_text(out, encoding="utf-8")
            lines.append(f"| {name} | ok | {failed_cases} | {failed_asserts} | {'; '.join(cases)[:400]} |")
        finally:
            path.write_bytes(text.encode("utf-8"))
        print(lines[-1], flush=True)
    b = run(["powershell", "-NoProfile", "-File", str(HERE / "build-tests.ps1")])
    t = run([str(TESTS), TAGS], cwd="D:\\")
    lines.append(f"\nRestored build: exit {b.returncode}; tests: {t.stdout.strip().splitlines()[-1] if t.stdout else '?'}")
    (HERE / "mutations.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
    print("\n".join(lines))


main()
