"""B10 review round mutations: the lit-cloud terms must be identity at a high
sun. Each mutation brings back the reviewed bug (or a piece of it) in the
scratch copy, rebuilds PoseidonTests, runs the B10 tags and must turn at least
one case red. The file is restored after each mutation.

    python mutate_review.py > mutations-review.log
"""
import os
import subprocess
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

SHADER = r"engine\PoseidonGL33\EngineGL33_LightingShadersSky.cpp"
MIRROR = r"engine\Poseidon\Graphics\Remaster\SkyDensity.cpp"

# The reviewed shader: relief and lining faded, the darkening against the sun
# and the bellies not.
SHADER_NEW = """    float relief = clamp((d - tau) * kLitCloudRelief, -0.35, 0.8);
    float gain = 1.0 + relief * direct;
    gain *= 1.0 + (0.55 - dark) * 0.7 * deck;
    gain *= 1.0 - 0.35 * pow(max(cosT, 0.0), 6.0) * smoothstep(0.35, 0.9, feature) * direct;
    float warm = clamp(relief, 0.0, 1.0) * direct;
    // The rim is where the density falls off toward the sun; Henyey-Greenstein
    // (g 0.8) relative to its peak toward the sun.
    float rim = clamp((d - tau) * 3.0, 0.0, 1.0) * (1.0 - 0.5 * d);
    float hg = 0.008 / pow(max(1.64 - 1.6 * cosT, 1e-4), 1.5); // 1 toward the sun
    float silver = hg * rim * kLitCloudSilver * direct;
    // All of it fades with the low sun, the darkening against the sun and the
    // bellies included: identity from kLitCloudFadeEndDeg up.
    float lowSun = 1.0 - smoothstep(kLitCloudFadeStartDeg, kLitCloudFadeEndDeg, litSun.w);
    return vec4(mix(1.0, gain, lowSun), warm * lowSun, silver * lowSun, litSky.x * smoothstep(0.02, 0.25, d));"""
SHADER_OLD = """    float lowSun = 1.0 - smoothstep(kLitCloudFadeStartDeg, kLitCloudFadeEndDeg, litSun.w);
    float relief = clamp((d - tau) * kLitCloudRelief, -0.35, 0.8) * lowSun;
    float gain = 1.0 + relief * direct;
    gain *= 1.0 + (0.55 - dark) * 0.7 * deck;
    gain *= 1.0 - 0.35 * pow(max(cosT, 0.0), 6.0) * smoothstep(0.35, 0.9, feature) * direct;
    float warm = clamp(relief, 0.0, 1.0) * direct;
    // The rim is where the density falls off toward the sun; Henyey-Greenstein
    // (g 0.8) relative to its peak toward the sun.
    float rim = clamp((d - tau) * 3.0, 0.0, 1.0) * (1.0 - 0.5 * d);
    float hg = 0.008 / pow(max(1.64 - 1.6 * cosT, 1e-4), 1.5); // 1 toward the sun
    float silver = hg * rim * kLitCloudSilver * direct * lowSun;
    return vec4(gain, warm, silver, litSky.x * smoothstep(0.02, 0.25, d));"""

MIRROR_NEW = """    const float relief = std::clamp((density - towardSun) * kLitCloudRelief, -0.35f, 0.8f);
    float gain = 1.0f + relief * direct;
    gain *= 1.0f + (0.55f - dark) * 0.7f * deck;
    gain *= 1.0f - 0.35f * std::pow(std::max(cosViewSun, 0.0f), 6.0f) * Smoothstep(0.35f, 0.9f, feature) * direct;
    const float warm = Clamp01(relief) * direct;
    const float rim = Clamp01((density - towardSun) * 3.0f) * (1.0f - 0.5f * density);
    const float hg = 0.008f / std::pow(std::max(1.64f - 1.6f * cosViewSun, 1e-4f), 1.5f);
    const float silver = hg * rim * kLitCloudSilver * direct;
    // Every term fades with the low sun, the darkening against the sun and the
    // bellies included.
    const float low = LitCloudLowSun(sunElevationDeg);
    return LitCloudShade{1.0f + (gain - 1.0f) * low, warm * low, silver * low};"""
MIRROR_OLD = """    const float low = LitCloudLowSun(sunElevationDeg);
    const float relief = std::clamp((density - towardSun) * kLitCloudRelief, -0.35f, 0.8f) * low;
    float gain = 1.0f + relief * direct;
    gain *= 1.0f + (0.55f - dark) * 0.7f * deck;
    gain *= 1.0f - 0.35f * std::pow(std::max(cosViewSun, 0.0f), 6.0f) * Smoothstep(0.35f, 0.9f, feature) * direct;
    const float warm = Clamp01(relief) * direct;
    const float rim = Clamp01((density - towardSun) * 3.0f) * (1.0f - 0.5f * density);
    const float hg = 0.008f / std::pow(std::max(1.64f - 1.6f * cosViewSun, 1e-4f), 1.5f);
    const float silver = hg * rim * kLitCloudSilver * direct * low;
    return LitCloudShade{gain, warm, silver};"""

MUTATIONS = [
    ("shader: the reviewed litCloudTerms (darkening against the sun and bellies not faded)", SHADER, SHADER_NEW,
     SHADER_OLD),
    ("mirror: the reviewed rule (darkening against the sun and bellies not faded)", MIRROR, MIRROR_NEW, MIRROR_OLD),
    ("shader: the gain not faded on the way out", SHADER,
     "return vec4(mix(1.0, gain, lowSun), warm * lowSun,", "return vec4(gain, warm * lowSun,"),
    ("mirror: the gain not faded on the way out", MIRROR,
     "return LitCloudShade{1.0f + (gain - 1.0f) * low, warm * low, silver * low};",
     "return LitCloudShade{gain, warm * low, silver * low};"),
    ("shader: the dome marches at a high sun", SHADER,
     "if (litSky.x <= 0.0 || litSun.w >= kLitCloudFadeEndDeg) return vec4(1.0, 0.0, 0.0, 0.0);",
     "if (litSky.x <= 0.0) return vec4(1.0, 0.0, 0.0, 0.0);"),
    ("shader: the sheets march at a high sun", SHADER,
     "if (litSky.x <= 0.0 || litSun.w >= kLitCloudFadeEndDeg || alpha <= 0.02)",
     "if (litSky.x <= 0.0 || alpha <= 0.02)"),
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
