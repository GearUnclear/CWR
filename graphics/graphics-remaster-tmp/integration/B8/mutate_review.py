"""B8 review: the new contract tests must fail without the review's changes.

    python mutate_review.py

Each mutation puts one part of the review's change back the way it was, builds
the unit test exe, runs [AirMedium],[FeatureGates], records pass or fail and
restores the file. Every mutation must turn the run red.
"""
import subprocess
from pathlib import Path

ROOT = Path(r"D:\Arma_CWA_worktrees\gr-B8\arma_CWR")
BUILD = ROOT / "build" / "gr-B8-rwdi"
EXE = BUILD / "tests" / "unit" / "engine" / "Poseidon" / "PoseidonTests.exe"
ENV_PS = (
    "$env:VCPKG_ROOT='C:\\dev\\vcpkg'; $env:Path='C:\\Program Files\\LLVM\\bin;C:\\Program Files\\CMake\\bin;"
    "C:\\Users\\Dane\\AppData\\Local\\Microsoft\\WinGet\\Packages\\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;"
    "C:\\Users\\Dane\\AppData\\Local\\Microsoft\\WinGet\\Packages\\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\\"
    "ccache-4.13.6-windows-x86_64;'+$env:Path; "
)
SH = ROOT / "engine/PoseidonGL33/EngineGL33_PostAirShaders.cpp"
PA = ROOT / "engine/PoseidonGL33/EngineGL33_PostAir.cpp"
AM = ROOT / "engine/Poseidon/Graphics/Remaster/AirMedium.hpp"
FG = ROOT / "engine/Poseidon/Graphics/Remaster/FeatureGates.hpp"

MUTATIONS = [
    ("beam contrast on the scattering itself (the band)", SH,
     "vec2 shaft = vec2(clamp(air.r / max(air.a, 1e-7), 0.0, 1.0), air.g);", "vec2 shaft = air.rg;"),
    ("mean share from the lit mean alone", SH,
     "vec2 local = vec2(clamp(mean.x / max(mean.z, 1e-7), 0.0, 1.0), mean.y);", "vec2 local = mean.xy;"),
    ("glow not scaled back by the open scattering", SH,
     "glow = airSunColor.rgb * (shaft.x * air.a) + airSpotTint.rgb * shaft.y;",
     "glow = airSunColor.rgb * shaft.x + airSpotTint.rgb * shaft.y;"),
    ("march without the open scattering", SH, "fragColor = vec4(sunBeams, spots, vis, sunOpen);",
     "fragColor = vec4(sunBeams, spots, vis, min(z, 6e4));"),
    ("mean averaging only r and g", SH, "mean += texture(airTex, uv).rga * wk;",
     "mean += vec3(texture(airTex, uv).rg, 0.0) * wk;"),
    ("morning mist to 15 degrees (lowsun_fp's veil)", AM, "constexpr float kMorningGoneSunDeg = 10.0f;",
     "constexpr float kMorningGoneSunDeg = 15.0f;"),
    ("morning gate to 15 degrees", FG, "constexpr float kGateMorningMaxSunDeg = 10.0f;",
     "constexpr float kGateMorningMaxSunDeg = 15.0f;"),
    ("shaded-air sky light hard-coded", SH, "zen * (skyLum * kAirHazeShadowAmbient)", "zen * (skyLum * 0.55)"),
    ("terrain maps only with terrain shadows", PA,
     "(_lighting.terrainShadows > 0.0f || _lighting.terrainOcclusion > 0.0f);\n    const bool hills",
     "_lighting.terrainShadows > 0.0f;\n    const bool hills"),
]


def build():
    cmd = ENV_PS + f"cmake --build '{BUILD}' --target PoseidonTests -j 8"
    r = subprocess.run(["powershell", "-NoProfile", "-Command", cmd], capture_output=True, text=True)
    return r.returncode == 0, r.stdout[-2000:] + r.stderr[-2000:]


def run_tests():
    r = subprocess.run([str(EXE), "[AirMedium],[FeatureGates]"], capture_output=True, text=True, cwd=EXE.parent)
    tail = [ln for ln in r.stdout.splitlines() if "test cases" in ln or "passed" in ln]
    return r.returncode == 0, tail[-1] if tail else r.stdout[-300:]


def main():
    log = []
    for name, path, old, new in MUTATIONS:
        src = path.read_text(encoding="utf-8")
        assert src.count(old) == 1, (name, src.count(old))
        path.write_text(src.replace(old, new), encoding="utf-8", newline="\n")
        try:
            ok, out = build()
            if not ok:
                err = [ln.strip() for ln in out.splitlines() if "error" in ln][:1]
                line = f"RED (build fails)  {name}: {err[0] if err else ''}"
            else:
                passed, tail = run_tests()
                line = f"{'GREEN (BAD)' if passed else 'RED'}  {name}: {tail}"
        finally:
            path.write_text(src, encoding="utf-8", newline="\n")
        print(line, flush=True)
        log.append(line)
    ok, out = build()
    passed, tail = run_tests()
    line = f"restored: build {'ok' if ok else 'FAILED'}, tests {'green' if passed else 'RED'}: {tail}"
    print(line)
    log.append(line)
    (Path(__file__).parent / "mutations-review.log").write_text("\n".join(log) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
