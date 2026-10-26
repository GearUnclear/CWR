"""B1 mutations: break one contract at a time, rebuild the test binary, run the
B1 tags, restore the file. Each mutation must turn its case red.

    python mutate.py  ->  mutations.log
"""
import os
import subprocess
from pathlib import Path

ROOT = Path(r"D:\Arma_CWA_worktrees\gr-B1\arma_CWR")
TESTS = ROOT / r"build\gr-B1-rwdi\tests\unit\engine\Poseidon\PoseidonTests.exe"
TAGS = "[SunShadows],[FeatureSwitches],[FeatureGates],[ShadowSampler],[Shaders]"
ENV = dict(os.environ)
ENV["VCPKG_ROOT"] = r"C:\dev\vcpkg"
ENV["Path"] = (r"C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;"
               r"C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;"
               r"C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;"
               + ENV.get("Path", ""))

MUTATIONS = [
    ("unit 6 not rebound after a new cascade array", "engine/PoseidonGL33/EngineGL33_ShadowDepth.cpp",
     "    GL33Bind::ActiveUnit(kShadowRawUnit);\n    glBindTexture(GL_TEXTURE_2D_ARRAY, s_arrTex);\n", ""),
    ("unit 6 without the stand-in at init", "engine/PoseidonGL33/EngineGL33_Lighting.cpp",
     "    GL33Bind::ActiveUnit(kShadowRawUnit);\n    glBindTexture(GL_TEXTURE_2D_ARRAY, _dummyDepthArray);\n", ""),
    ("Taa and Pcss swapped in the GLSL block", "engine/PoseidonGL33/EngineGL33_LightingShaders.cpp",
     "    vec4 litTaa;          // 59", "    vec4 litPcss0;        // 59"),
    ("soft filter without the fade at the gate edge", "engine/Poseidon/Graphics/Remaster/SunShadows.cpp",
     "    return t * t * (3.0f - 2.0f * t);", "    return t > 0.0f ? 1.0f : 0.0f;"),
    ("constants follow the switch, not the gate", "engine/Poseidon/Graphics/Remaster/SunShadows.cpp",
     "    const float tanRadius = soft.on ? SunTanRadius(tuning.sunSize) : 0.0f;",
     "    const float tanRadius = (soft.on || FeatureOn(tuning.features, Feature::SoftShadows)) ? "
     "SunTanRadius(tuning.sunSize) : 0.0f;"),
    ("static objects cast without the switch", "engine/Poseidon/Graphics/Remaster/SunShadows.cpp",
     "    return castShadow || (staticObject && treeShadowsOn);", "    return castShadow || staticObject;"),
    ("shadow pass asks the object alone", "engine/Poseidon/World/Scene/SceneShadowPass.cpp",
     "!remaster::CastsIntoSunShadowMap(oi->object->CastShadow(), oi->object->Static(), smTreeShadows))",
     "!oi->object->CastShadow())"),
    ("shadow pass tightens the first tier", "engine/Poseidon/World/Scene/SceneShadowPass.cpp",
     "    bp.omniCoef[0] = tune.omniCoef0;", "    bp.omniCoef[0] = std::min(tune.omniCoef0, 0.04f); // SoftShadows"),
    ("stock filter skipped at soft strength 0", "engine/PoseidonGL33/EngineGL33_LightingShaders.cpp",
     "            if (soft < 1.0) lit = pcfVisibility(suv, c, sc.z - bias, radius);",
     "            if (soft < 0.5) lit = pcfVisibility(suv, c, sc.z - bias, radius);"),
    ("grass lifted like leaves", "engine/PoseidonGL33/EngineGL33_LightingShaders.cpp",
     "    float leafLift = (foliage && !terrain) ? litPcss.w : 0.0;", "    float leafLift = foliage ? litPcss.w : 0.0;"),
    ("softShadows not landed", "engine/Poseidon/Graphics/Remaster/FeatureSwitches.cpp",
     '"B1", true,\n     &FS::softShadows', '"B1", false,\n     &FS::softShadows'),
    ("search taps not 16 on the near tiers", "engine/PoseidonGL33/EngineGL33_LightingShaders.cpp",
     "(c < int(cascadeCtl.w) ? 16 : 8)", "(c < int(cascadeCtl.w) ? 12 : 8)"),
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
            b = run(["cmake", "--build", "build/gr-B1-rwdi", "--target", "PoseidonTests", "-j", "8"], cwd=ROOT)
            if b.returncode != 0:
                log.append(f"BUILD-FAIL {name}: {b.stdout[-400:]}")
                continue
            t = run([str(TESTS), TAGS], cwd="D:\\")
            failed = [ln.strip() for ln in t.stdout.splitlines() if ln.strip().startswith("test cases:")]
            red = t.returncode != 0
            cases = sorted({ln.split(":")[0] for ln in t.stdout.splitlines() if "FAILED" in ln})
            log.append(f"{'RED ' if red else 'GREEN'} {name}: {failed[0] if failed else 'all passed'}")
        finally:
            path.write_text(text, encoding="utf-8", newline="\n")
    b = run(["cmake", "--build", "build/gr-B1-rwdi", "--target", "PoseidonTests", "-j", "8"], cwd=ROOT)
    t = run([str(TESTS), TAGS], cwd="D:\\")
    log.append(f"restored: build {b.returncode}, tests {t.returncode} ({t.stdout.strip().splitlines()[-1]})")
    out = Path(__file__).with_name("mutations.log")
    out.write_text("\n".join(log) + "\n", encoding="utf-8")
    print("\n".join(log))


main()
