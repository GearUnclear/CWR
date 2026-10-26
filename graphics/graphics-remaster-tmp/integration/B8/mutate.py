"""B8 mutations: break one contract at a time, rebuild the test binary, run the
B8 tags, restore the file. Each mutation must turn its case red.

    python mutate.py  ->  mutations.log
"""
import os
import subprocess
from pathlib import Path

ROOT = Path(r"D:\Arma_CWA_worktrees\gr-B8\arma_CWR")
TESTS = ROOT / r"build\gr-B8-rwdi\tests\unit\engine\Poseidon\PoseidonTests.exe"
TAGS = "[AirMedium],[PostStages],[FeatureSwitches],[FeatureGates],[ShadowSampler],[Shaders]"
ENV = dict(os.environ)
ENV["VCPKG_ROOT"] = r"C:\dev\vcpkg"
ENV["Path"] = (r"C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;"
               r"C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;"
               r"C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;"
               + ENV.get("Path", ""))

MUTATIONS = [
    ("the Air stage not landed", "engine/Poseidon/Graphics/Remaster/PostStages.cpp",
     '{PostStage::Air, "air", "B8", true, true,', '{PostStage::Air, "air", "B8", false, true,'),
    ("the beams do not replace the shafts", "engine/Poseidon/Graphics/Remaster/PostStages.cpp",
     "&& !(air && in.airBeams))", ")"),
    ("the air pass over an optics overlay", "engine/Poseidon/Graphics/Remaster/PostStages.cpp",
     "const bool air = in.air && in.worldDepth && in.glow;", "const bool air = in.air && in.worldDepth;"),
    ("the classic fog's edge does not move", "engine/Poseidon/Graphics/Remaster/AirMedium.cpp",
     "    return (fogEnd - fogStart) / (fogEnd - start);", "    return 1.0f + 0.0f * start;"),
    ("the haze's density at the far end", "engine/Poseidon/Graphics/Remaster/AirMedium.cpp",
     "    return len * std::exp(-yA / scaleHeight) * f;", "    return len * std::exp(-yB / scaleHeight) * f;"),
    ("morning mist with a western sun", "engine/Poseidon/Graphics/Remaster/AirMedium.cpp",
     "    return SmoothStep(-0.15f, 0.25f, sunEast) *", "    return 1.0f *"),
    ("fog does not thicken the beam mist", "engine/Poseidon/Graphics/Remaster/AirMedium.cpp",
     "    return std::max(fog, std::max(rain, lowSun));", "    return std::max(rain, lowSun) + 0.0f * fog;"),
    ("the shader's scale height drifts", "engine/PoseidonGL33/EngineGL33_PostAirShaders.cpp",
     "const float kAirHazeScaleHeight = 350.0;", "const float kAirHazeScaleHeight = 300.0;"),
    ("the shader's chroma drifts", "engine/PoseidonGL33/EngineGL33_PostAirShaders.cpp",
     "vec3(0.85, 1.0, 1.2)", "vec3(0.72, 1.0, 1.5)"),
    ("the forward fog ignores the haze", "engine/PoseidonGL33/EngineGL33_LightingShaders.cpp",
     "    r0.rgb = mix(fogC, r0.rgb, min(vFogTC * (1.0 + litHaze.x), 1.0));\n",
     "    r0.rgb = mix(fogC, r0.rgb, vFogTC);\n"),
    ("litHaze dropped from the block", "engine/PoseidonGL33/EngineGL33_LightingShaders.cpp",
     "    vec4 litHaze;         // 78", "    vec4 litHazeX;        // 78"),
    ("unit 2 keeps the comparison sampler after the march", "engine/PoseidonGL33/EngineGL33_PostAir.cpp",
     "        glBindSampler(kShadowMapUnit, 0);\n", ""),
    ("the march may meet texture 0 on unit 2", "engine/PoseidonGL33/EngineGL33_PostAir.cpp",
     "shadowMap ? _shadowMapTex : _dummyDepthArray", "_shadowMapTex"),
    ("the haze's edge moves without the post pass", "engine/PoseidonGL33/EngineGL33_PostAir.cpp",
     "        const bool post = _lighting.postEffects && SSAAActive() && !_post.unavailable",
     "        const bool post = SSAAActive() && !_post.unavailable"),
    ("airHaze not landed", "engine/Poseidon/Graphics/Remaster/FeatureSwitches.cpp",
     '"B8", true, &FS::airHaze', '"B8", false, &FS::airHaze'),
    ("the mist ignores the valley depth", "engine/Poseidon/Graphics/Remaster/AirMedium.cpp",
     "    return SmoothStep(kValleyDepthFrom, kValleyDepthFull, depth);", "    return 1.0f + 0.0f * depth;"),
    ("the mist is not under the classic fog's edge", "engine/PoseidonGL33/EngineGL33_PostAirShaders.cpp",
     "        scat = mix(skyLim * (1.0 - T), scat, fEdge);", "        scat = scat * (fEdge + 1.0 - fEdge);"),
    ("the morning lasts to 38 degrees", "engine/Poseidon/Graphics/Remaster/AirMedium.hpp",
     "constexpr float kMorningGoneSunDeg = 15.0f;", "constexpr float kMorningGoneSunDeg = 38.0f;"),
    ("the composite reads the resolved frame under the air", "engine/PoseidonGL33/EngineGL33_PostFX.cpp",
     "    BindTex(0, plan.Runs(rm::PostStage::Air) ? p.airOutTex : p.colorTex);", "    BindTex(0, p.colorTex);"),
]


def run(cmd, **kw):
    return subprocess.run(cmd, capture_output=True, text=True, env=ENV, **kw)


def main():
    log = []
    for name, rel, old, new in MUTATIONS:
        path = ROOT / rel
        text = path.read_text(encoding="utf-8")
        if text.count(old) != 1:
            log.append(f"SKIP {name}: pattern found {text.count(old)} times")
            continue
        path.write_text(text.replace(old, new), encoding="utf-8", newline="\n")
        try:
            b = run(["cmake", "--build", "build/gr-B8-rwdi", "--target", "PoseidonTests", "-j", "8"], cwd=ROOT)
            if b.returncode != 0:
                log.append(f"BUILD-FAIL {name}: {b.stdout[-400:]}")
                continue
            t = run([str(TESTS), TAGS], cwd="D:\\")
            summary = [ln.strip() for ln in t.stdout.splitlines() if ln.strip().startswith("test cases:")]
            red = t.returncode != 0
            cases = []
            lines = t.stdout.splitlines()
            for i, ln in enumerate(lines):
                if ln.startswith("----") and i + 1 < len(lines) and i + 2 < len(lines) and lines[i + 2].startswith("----"):
                    cases.append(lines[i + 1].strip())
            log.append(f"{'RED  ' if red else 'GREEN'} {name}: {summary[0] if summary else 'all passed'}"
                       + (f" [{'; '.join(sorted(set(cases)))}]" if cases else ""))
        finally:
            path.write_text(text, encoding="utf-8", newline="\n")
    b = run(["cmake", "--build", "build/gr-B8-rwdi", "--target", "PoseidonTests", "-j", "8"], cwd=ROOT)
    t = run([str(TESTS), TAGS], cwd="D:\\")
    log.append(f"restored: build {b.returncode}, tests {t.returncode} ({t.stdout.strip().splitlines()[-1]})")
    out = Path(__file__).with_name("mutations.log")
    out.write_text("\n".join(log) + "\n", encoding="utf-8")
    print("\n".join(log))


if __name__ == "__main__":
    main()
