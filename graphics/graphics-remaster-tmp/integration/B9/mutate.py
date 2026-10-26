"""B9 mutations: each one breaks a contract of the canopy step in the scratch
copy, rebuilds the unit tests and runs the tags; every mutation must turn its
cases red. The source is restored after each one.

    python mutate.py > mutations.log
"""
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(r"D:\Arma_CWA_worktrees\gr-B9\arma_CWR")
ENV = dict(os.environ)
ENV["VCPKG_ROOT"] = r"C:\dev\vcpkg"
ENV["Path"] = (r"C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;"
               r"C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;"
               r"C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe"
               r"\ccache-4.13.6-windows-x86_64;" + ENV.get("Path", ""))
TESTS = ROOT / "build" / "gr-B9-rwdi" / "tests" / "unit" / "engine" / "Poseidon" / "PoseidonTests.exe"
TAGS = "[Canopy],[FeatureSwitches],[FeatureGates],[Shaders],[gpu],[ShadowSampler]"

MUTATIONS = [
    ("linear crossing in the mirror", "engine/Poseidon/Graphics/Remaster/Canopy.cpp",
     "const float crossing = std::sqrt(den - kCanopyMinDensity);",
     "const float crossing = (den - kCanopyMinDensity);"),
    ("a hard density threshold sets the raised edge", "engine/Poseidon/Graphics/Remaster/Canopy.cpp",
     "const float w = Smoothstep(kCanopyEdgeCrossFrom, kCanopyEdgeCrossTo, crossing);",
     "const float w = crossing > 0.45f ? 1.0f : 0.0f;"),
    ("raised receivers do not skip their own crown", "engine/Poseidon/Graphics/Remaster/Canopy.cpp",
     "if (t >= kCanopyRaisedSkip)", "if (t >= 0.0f)"),
    ("mip keeps the mean height, not the tallest", "engine/Poseidon/Graphics/Remaster/Canopy.cpp",
     "height = std::max(height, Decode(child[ci * 2]) - Decode(childGround[ci]));",
     "height += 0.25f * (Decode(child[ci * 2]) - Decode(childGround[ci]));"),
    ("removal does not update the mips", "engine/Poseidon/Graphics/Remaster/Canopy.cpp",
     "        Downsample(l, p.x0, p.z0, p.x1, p.z1);\n        rects.push_back(p);",
     "        rects.push_back(p);"),
    ("the canopy ignores the slope (flat 100 m ground)", "engine/Poseidon/Graphics/Remaster/Canopy.cpp",
     "dst[i * 2] = covered ? Encode(Decode(ground) + height) : ground;",
     "dst[i * 2] = covered ? Encode(100.0f + height) : ground;"),
    ("no fade over the gate's edges", "engine/Poseidon/Graphics/Remaster/Canopy.cpp",
     "    return Smoothstep(kGateCanopyMinSunDeg, kCanopyFadeInToSunDeg, sunElevationDeg) *",
     "    return 1.0f;\n    return Smoothstep(kGateCanopyMinSunDeg, kCanopyFadeInToSunDeg, sunElevationDeg) *"),
    ("the map never re-centres on the camera", "engine/Poseidon/Graphics/Remaster/Canopy.cpp",
     "return std::fabs(camX - cx) > limit || std::fabs(camZ - cz) > limit;", "return false;"),
    ("constants ignore the map being ready", "engine/Poseidon/Graphics/Remaster/Canopy.cpp",
     "if (!gates[Feature::CanopyShadows].on || !mapReady || !(texel > 0.0f))",
     "if (!gates[Feature::CanopyShadows].on || !(texel > 0.0f))"),
    ("the switch has not landed", "engine/Poseidon/Graphics/Remaster/FeatureSwitches.cpp",
     '"Long canopy shadows", "hfrt", "Light and shadow", "B9", true,',
     '"Long canopy shadows", "hfrt", "Light and shadow", "B9", false,'),
    ("the gate keeps a half-resolution scale", "engine/Poseidon/Graphics/Remaster/FeatureGates.cpp",
     "        case Feature::ValleyMist:\n            return 0.5f;",
     "        case Feature::ValleyMist:\n        case Feature::CanopyShadows:\n            return 0.5f;"),
    ("no canopySun sampler row", "engine/PoseidonGL33/EngineGL33_Lighting.hpp",
     '    {"canopySun", kCanopyVisUnit},           // the canopy\'s sun-visibility map (long tree shadows)\n', ""),
    ("the canopy slot below the padding", "engine/PoseidonGL33/EngineGL33_LightingShaders.cpp",
     "    vec4 litPlanned[8];   // 68-75: allocated to features still to land (clouds .. water)\n"
     "    vec4 litCanopy;",
     "    vec4 litCanopy;       // moved\n    vec4 litPlanned[8];   // 68-75: allocated to features still to land (clouds .. water)\n"
     "    vec4 litCanopyX;"),
    ("unit 15 without its stand-in at init", "engine/PoseidonGL33/EngineGL33_Canopy.cpp",
     "    BindUnit(kCanopyVisUnit, s_canopy.sunStandIn);\n    GL33Bind::ActiveUnit(0);\n    // The canopy",
     "    GL33Bind::ActiveUnit(0);\n    // The canopy"),
    ("the pass crosses with the linear density", "engine/PoseidonGL33/EngineGL33_PostCanopyShaders.cpp",
     "float crossing = sqrt(c.y - kMinDensity);", "float crossing = c.y - kMinDensity;"),
    ("the read ignores the shadow maps' reach", "engine/PoseidonGL33/EngineGL33_LightingShaders.cpp",
     "float w = clamp((depth - (reach - fade)) / fade, 0.0, 1.0);", "float w = 1.0;"),
    ("the read before the terrain lookup", "engine/PoseidonGL33/EngineGL33_LightingShaders.cpp",
     "    float sunVisibility = shadowMapVisibility(nCast, nlCast, terrain ? 0.0 : 1.0, leafLift) *\n"
     "                          terrainSunVisibility(posAbs, skyVisibility) * cloudSunVisibility(posAbs);\n"
     "    // Long tree shadows beyond the shadow maps (canopyShadows); 1 while off.\n"
     "    sunVisibility *= canopySunVisibility(posAbs);",
     "    float canopyFirst = canopySunVisibility(posAbs);\n"
     "    float sunVisibility = shadowMapVisibility(nCast, nlCast, terrain ? 0.0 : 1.0, leafLift) *\n"
     "                          terrainSunVisibility(posAbs, skyVisibility) * cloudSunVisibility(posAbs);\n"
     "    sunVisibility *= canopyFirst;"),
    ("no canopy GPU section", "engine/Poseidon/Graphics/Core/GpuTimings.cpp",
     '    {"canopy", Mark::CanopyBegin, Mark::CanopyEnd},\n', '    {"canopy", Mark::CanopyBegin, Mark::FrameEnd},\n'),
]


def run(cmd, **kw):
    return subprocess.run(cmd, env=ENV, capture_output=True, text=True, errors="replace", **kw)


def main():
    only = set(sys.argv[1:])
    for i, (name, rel, old, new) in enumerate(MUTATIONS):
        if only and str(i) not in only:
            continue
        p = ROOT / rel
        src = p.read_bytes()
        text = src.decode("utf-8")
        if old not in text:
            print(f"[{i}] {name}: PATTERN NOT FOUND in {rel}", flush=True)
            continue
        p.write_bytes(text.replace(old, new, 1).encode("utf-8"))
        try:
            b = run(["cmake", "--build", "build/gr-B9-rwdi", "--target", "PoseidonTests", "-j", "8"], cwd=ROOT)
            if b.returncode != 0:
                tail = [l for l in b.stdout.splitlines() if "error" in l.lower()][:3]
                print(f"[{i}] {name}: BUILD FAILED (counts as red) {tail}", flush=True)
                continue
            t = run([str(TESTS), TAGS], cwd="D:\\")
            lines = t.stdout.splitlines()
            summary = [l for l in lines if l.startswith("test cases:") or l.startswith("All tests passed")]
            cases = []
            for j, l in enumerate(lines):
                if l.startswith("------------") and j + 1 < len(lines) and not lines[j + 1].startswith("-") \
                        and not lines[j + 1].startswith("D:") and lines[j + 1].strip():
                    nm = lines[j + 1].strip()
                    if nm not in cases and not nm.startswith("..."):
                        cases.append(nm)
            verdict = "RED" if t.returncode != 0 else "GREEN (mutation survived)"
            print(f"[{i}] {name}: {verdict}; {summary[-1] if summary else '?'}; cases: {cases[:6]}", flush=True)
        finally:
            p.write_bytes(src)
    b = run(["cmake", "--build", "build/gr-B9-rwdi", "--target", "PoseidonTests", "-j", "8"], cwd=ROOT)
    print("restored build exit:", b.returncode, flush=True)


if __name__ == "__main__":
    main()
