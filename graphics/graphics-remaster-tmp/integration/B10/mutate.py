"""B10 mutation check: each mutation breaks one contract in the scratch copy,
rebuilds PoseidonTests, runs the B10 tags and must turn at least one case red.
The file is restored after each mutation.

    python mutate.py > mutations.log
"""
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(r"D:\Arma_CWA_worktrees\gr-B10\arma_CWR")
TESTS = ROOT / r"build\gr-B10-rwdi\tests\unit\engine\Poseidon\PoseidonTests.exe"
TAGS = "[SkyDensity],[clouds],[FeatureSwitches],[FeatureGates],[remaster]"
ENV = dict(os.environ)
ENV["VCPKG_ROOT"] = r"C:\dev\vcpkg"
ENV["Path"] = (r"C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;"
               r"C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;"
               r"C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;"
               + ENV.get("Path", ""))

MUTATIONS_ALL = [
    ("density map without the blur", r"engine\Poseidon\Graphics\Remaster\SkyDensity.cpp",
     "    BlurPlane(feature, w, h);\n    BlurPlane(dark, w, h);\n", ""),
    ("whiteness against the whole sky, not the row", r"engine\Poseidon\Graphics\Remaster\SkyDensity.cpp",
     "        float qMin = 1e9f;\n", "        static float qMin = 1e9f;\n"),
    ("rig ignores the sky's covered share", r"engine\Poseidon\Graphics\Remaster\LightRig.cpp",
     "    if (clouds && in.skyShadowCoverage >= 0.0f)\n", "    if (false)\n"),
    ("shader threshold 0.5", r"engine\PoseidonGL33\EngineGL33_LightingShaders.cpp",
     "const float kSkyShadowThreshold = 0.55;", "const float kSkyShadowThreshold = 0.5;"),
    ("dome mapping with the wrong second ring", r"engine\PoseidonGL33\EngineGL33_LightingShaders.cpp",
     "else if (e < 29.5) v = mix(0.9869, 0.5434, (e - 0.87) / 28.63);",
     "else if (e < 29.5) v = mix(0.9869, 0.55, (e - 0.87) / 28.63);"),
    ("the dome turns with the clouds off", r"engine\PoseidonGL33\EngineGL33_LightingShaders.cpp",
     "if (dome && litSky.z != 0.0) vUV0.x += litSky.z;", "if (dome) vUV0.x += litSky.z;"),
    ("relief at any sun height", r"engine\Poseidon\Graphics\Remaster\SkyDensity.hpp",
     "constexpr float kLitCloudFadeEndDeg = 45.0f;", "constexpr float kLitCloudFadeEndDeg = 90.0f;"),
    ("shadows gated at the horizon", r"engine\Poseidon\Graphics\Remaster\FeatureGates.hpp",
     "constexpr float kGateSkyShadowsMinSunDeg = -1.5f;", "constexpr float kGateSkyShadowsMinSunDeg = 0.0f;"),
    ("no cross-fade after a step", r"engine\Poseidon\Graphics\Remaster\SkyDensity.cpp",
     "    prevWeight = 1.0f;\n}", "    prevWeight = 0.0f;\n}"),
    ("the gates ignore the density map", r"engine\Poseidon\Graphics\Remaster\FeatureGates.cpp",
     "            if (!in.skyDensityReady)\n                return Off(\"sky density map building\");\n            return On(\"clouds in the sky\");",
     "            return On(\"clouds in the sky\");"),
    ("lit clouds not landed", r"engine\Poseidon\Graphics\Remaster\FeatureSwitches.cpp",
     "\"B10\", true, &FS::litClouds, \"sky\"}", "\"B10\", false, &FS::litClouds, \"sky\"}"),
    ("slot layout without SkyShadow", r"engine\PoseidonGL33\EngineGL33_LightingShaders.cpp",
     "    vec4 litSkyShadow;    // 69", "    vec4 litSkyShadowX;   // 69"),
]


def build():
    r = subprocess.run(["cmake", "--build", "build/gr-B10-rwdi", "-j", "8", "--target", "PoseidonTests"], cwd=ROOT,
                       env=ENV, capture_output=True, text=True)
    return r.returncode, r.stdout[-2000:] + r.stderr[-2000:]


def run_tests():
    r = subprocess.run([str(TESTS), TAGS], cwd="D:\\", capture_output=True, text=True)
    tail = [l for l in r.stdout.splitlines() if l.startswith("test cases") or l.startswith("assertions")
            or l.startswith("All tests")]
    failed = [l for l in r.stdout.splitlines() if l.strip().startswith("Sky density") or l.startswith("Remaster ")
              or l.startswith("Feature ")]
    return r.returncode, tail, failed


MUTATIONS_LATE = [
    ("coverage ignores the sun height", r"engine\Poseidon\Graphics\Remaster\SkyDensity.cpp",
     "    const float lead = kSkyShadowDeckAltitude * std::cos(e) / std::max(std::sin(e), 0.06f);",
     "    const float lead = 0.0f * e;"),
    ("the sun disk dims its colour only", r"engine\PoseidonGL33\EngineGL33_LightingShaders.cpp",
     "            r0 *= mix(1.0, kSkySunBehindCloud, skyShadowCoverToward(litSunDir.xyz));",
     "            r0.rgb *= mix(1.0, kSkySunBehindCloud, skyShadowCoverToward(litSunDir.xyz));"),
]
MUTATIONS = MUTATIONS_LATE if "--late" in sys.argv else MUTATIONS_ALL

for name, rel, old, new in MUTATIONS:
    path = ROOT / rel
    text = path.read_text(encoding="utf-8")
    if text.count(old) != 1:
        print(f"SKIP {name}: pattern found {text.count(old)} times", flush=True)
        continue
    path.write_text(text.replace(old, new), encoding="utf-8", newline="\n")
    try:
        rc, out = build()
        if rc != 0:
            print(f"RED (build fails) {name}", flush=True)
            continue
        rc, tail, failed = run_tests()
        verdict = "RED" if rc != 0 else "GREEN (mutation not caught)"
        print(f"{verdict} {name}: {' '.join(tail)}", flush=True)
        for f in sorted(set(failed))[:6]:
            print(f"    {f}", flush=True)
    finally:
        path.write_text(text, encoding="utf-8", newline="\n")

rc, out = build()
rc2, tail, _ = run_tests()
print(f"restored: build {rc}, tests {rc2}: {' '.join(tail)}", flush=True)
