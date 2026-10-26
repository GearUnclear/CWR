"""B3 mutations: each breaks one contract of the night step; the new cases must
turn red. Applies one mutation, builds PoseidonTests, runs the B3 tags, restores
the file, and logs which cases failed.

    python mutate.py [--only <n>[,<n>...]]
"""
import argparse
import os
import re
import subprocess
from pathlib import Path

ROOT = Path(r"D:\Arma_CWA_worktrees\gr-B3\arma_CWR")
LOG = Path(__file__).resolve().parent / "mutations.log"
TAGS = "[NightLights],[FeatureSwitches],[FeatureGates],[FrameLights],[remaster]"
ENV_PATH = (r"C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;"
            r"C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;"
            r"C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;")

E = "engine/"
MUTATIONS = [
    ("moon key ignores the gate's sun edge",
     E + "Poseidon/Graphics/Remaster/NightLights.cpp",
     "const float sun = Smooth01((kGateMoonKeyMaxSunDeg - in.sunElevationDeg) / kMoonKeyFadeDeg);",
     "const float sun = 1.0f;"),
    ("moon key without its fade (switches on)",
     E + "Poseidon/Graphics/Remaster/NightLights.cpp",
     "return sun * moon * phase;",
     "return 1.0f;"),
    ("key light keeps the moon in the shadowless term too",
     E + "Poseidon/Graphics/Remaster/LightRig.cpp",
     "rig.moon = rig.moon * (1.0f - share);",
     ""),
    ("key light by day picks the moon",
     E + "Poseidon/Graphics/Remaster/LightRig.cpp",
     "const float* toward = share > 0.0f ? in.towardMoon : in.towardSun;",
     "const float* toward = in.towardMoon;"),
    ("low beam aimed along the model's axis",
     E + "Poseidon/Graphics/Rendering/Lighting/FrameLights.cpp",
     "const float slope = bodyForward[1] / fl * along - kLowBeamTilt;",
     "const float slope = lampDir[1] / std::max(hl, 1e-4f) - kLowBeamTilt;"),
    ("low beam without the 1% tilt",
     E + "Poseidon/Graphics/Rendering/Lighting/FrameLights.cpp",
     "const float slope = bodyForward[1] / fl * along - kLowBeamTilt;",
     "const float slope = bodyForward[1] / fl * along;"),
    ("low beam lights behind the lamp",
     E + "Poseidon/Graphics/Rendering/Lighting/FrameLights.cpp",
     "    const float c = Dot(toPoint, beam);",
     "    const float c = std::fabs(Dot(toPoint, beam));"),
    ("lamps keep the flat disc of full light",
     E + "Poseidon/Graphics/Rendering/Lighting/FrameLights.cpp",
     "    const float e = intensity / (d2 + kPhysicalSoftening);",
     "    const float e = intensity / std::max(d2, light.start * light.start);"),
    ("lamp colour not taken to linear",
     E + "Poseidon/Graphics/Rendering/Lighting/FrameLights.cpp",
     "out[i] = std::pow(std::max(light.diffuse[i], 0.0f), 2.2f) * e;",
     "out[i] = light.diffuse[i] * e;"),
    ("no near fade off the housing",
     E + "Poseidon/Graphics/Rendering/Lighting/FrameLights.cpp",
     " * Smoothstep(kPhysicalNearStart, kPhysicalNearEnd, d);",
     ";"),
    ("night slot on without the tiled list",
     E + "Poseidon/Graphics/Remaster/NightLights.cpp",
     "if (!gates[Feature::NightLights].on || !tiledLights)",
     "if (!gates[Feature::NightLights].on)"),
    ("night lights gate ignores the light list switch",
     E + "Poseidon/Graphics/Remaster/FeatureGates.cpp",
     "            if (!FeatureOn(t.features, Feature::LightTiles))\n                return Off(\"light list off\");\n",
     ""),
    ("shader cut-off drifts from the C++ model",
     E + "PoseidonGL33/EngineGL33_LightingShadersLights.cpp",
     "const float kBeamCutHigh = 0.025;",
     "const float kBeamCutHigh = 0.015;"),
    ("shader leaves Lambert-only for leaves (no wrap)",
     E + "PoseidonGL33/EngineGL33_LightingShadersLights.cpp",
     "float wrap = clamp((dot(g_lightLeafNormal, L) + 0.35) / 1.35, 0.0, 1.0);",
     "float wrap = max(dot(g_lightLeafNormal, L), 0.0);"),
    ("physical lights stay in the classic clamp",
     E + "PoseidonGL33/EngineGL33_LightingShaders.cpp",
     "vec3 local = toLinear(min(localLights(n, kd, ka) * litGround.w, vec3(1.0))) + g_physicalLocal * litGround.w;",
     "vec3 local = toLinear(min(localLights(n, kd, ka) * litGround.w + g_physicalLocal, vec3(1.0)));"),
    ("shadow pass from the sun under the moon key",
     E + "Poseidon/World/Scene/SceneShadowPass.cpp",
     "sd = GEngine->RemasterMoonKey() ? _mainLight->MoonDirection() : _mainLight->SunDirection();",
     "sd = _mainLight->SunDirection();"),
    ("no shadow pass for the moon key",
     E + "Poseidon/World/Scene/SceneDraw.cpp",
     "if (sunFactor > 0.01f || remasterSunUp || GEngine->RemasterMoonKey())",
     "if (sunFactor > 0.01f || remasterSunUp)"),
    ("star splat without its centre gain",
     E + "Poseidon/Graphics/Remaster/NightLights.cpp",
     "s.alpha[0] = std::clamp(alpha * kStarCentreGain, 0.0f, 255.0f);",
     "s.alpha[0] = std::clamp(alpha, 0.0f, 255.0f);"),
    ("smoke keeps the classic lamps on top of the physical ones",
     E + "Poseidon/World/Scene/Object.cpp",
     "                        if (night && IsPhysicalNightLight(*lights[index]))\n                            continue;\n",
     ""),
    ("night switches not landed",
     E + "Poseidon/Graphics/Remaster/FeatureSwitches.cpp",
     '"night", "Night and combat", "B3", true, &FS::moonShadows',
     '"night", "Night and combat", "B3", false, &FS::moonShadows'),
    ("night slot moved off 71",
     E + "PoseidonGL33/EngineGL33_LightingShaders.cpp",
     "    vec4 litPlanned[3];   // 68-70",
     "    vec4 litPlanned[2];   // 68-70"),
]


def run(cmd, **kw):
    env = dict(os.environ)
    env["VCPKG_ROOT"] = r"C:\dev\vcpkg"
    env["PATH"] = ENV_PATH + env.get("PATH", "")
    return subprocess.run(cmd, cwd=kw.get("cwd", ROOT), env=env, capture_output=True, text=True,
                          encoding="utf-8", errors="replace")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", default="")
    a = ap.parse_args()
    only = {int(x) for x in a.only.split(",") if x.strip()}
    out = LOG.open("a", encoding="utf-8")
    for i, (name, rel, old, new) in enumerate(MUTATIONS, 1):
        if only and i not in only:
            continue
        p = ROOT / rel
        text = p.read_bytes().decode("utf-8")
        if old not in text:
            out.write(f"{i}. {name}: MUTATION NOT APPLICABLE (text not found)\n")
            out.flush()
            continue
        p.write_bytes(text.replace(old, new, 1).encode("utf-8"))
        try:
            b = run(["cmake", "--build", "build/gr-B3-rwdi", "-j", "8", "--target", "PoseidonTests"])
            if b.returncode != 0:
                tail = "\n".join(b.stdout.splitlines()[-6:])
                out.write(f"{i}. {name}: BUILD FAILED (counts as caught by the compiler)\n{tail}\n")
                out.flush()
                continue
            t = run([str(ROOT / "build/gr-B3-rwdi/tests/unit/engine/Poseidon/PoseidonTests.exe"), TAGS], cwd="D:\\")
            failed = sorted({" ".join(m.split()) for m in re.findall(r"^-{20,}\n((?:.+\n)+?)-{20,}\n", t.stdout, re.M)})
            summary = [l for l in t.stdout.splitlines() if l.startswith("test cases:")]
            out.write(f"{i}. {name}: {'RED' if t.returncode else 'GREEN (not caught)'} "
                      f"{summary[0] if summary else ''}\n")
            for f in failed:
                out.write(f"     {f}\n")
            out.flush()
        finally:
            p.write_bytes(text.encode("utf-8"))
    out.close()


if __name__ == "__main__":
    main()
