"""B3 review fix mutations (seaBody takes the physical lamps and low beams): each breaks
one contract of the fix; the new case must turn red. Applies one mutation, builds PoseidonTests, runs the B3 tags, restores
the file, and logs which cases failed.

    python mutate.py [--only <n>[,<n>...]]
"""
import argparse
import os
import re
import subprocess
from pathlib import Path

ROOT = Path(r"D:\Arma_CWA_worktrees\gr-B3\arma_CWR")
LOG = Path(__file__).resolve().parent / "mutations-review.log"
TAGS = "[NightLights],[SeaWaves],[FrameLights],[remaster]"
ENV_PATH = (r"C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;"
            r"C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;"
            r"C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;")

E = "engine/"
NL = chr(10)
SEA_VIEW = "    g_lightView = -vWorldRel * inversesqrt(max(dot(vWorldRel, vWorldRel), 1e-12));" + NL
MUTATIONS = [
    ("sea body drops the physical lamps and low beams",
     E + "PoseidonGL33/EngineGL33_LightingShaders.cpp",
     SEA_VIEW + "    vec3 local = toLinear(min(localLights(n, kd, ka) * litGround.w, vec3(1.0))) + g_physicalLocal * litGround.w;",
     SEA_VIEW + "    vec3 local = toLinear(min(localLights(n, kd, ka) * litGround.w, vec3(1.0)));"),
    ("sea body leaves the view direction at its default",
     E + "PoseidonGL33/EngineGL33_LightingShaders.cpp",
     SEA_VIEW,
     ""),
    ("sea body drops the physical gleam",
     E + "PoseidonGL33/EngineGL33_LightingShaders.cpp",
     "    vec3 radiance = (a * (direct + ambient + moon + local + emissive) + g_physicalSpec * litGround.w) * exposure;",
     "    vec3 radiance = a * (direct + ambient + moon + local + emissive) * exposure;"),
    ("surfaces drop the physical lamps and low beams (the scan of every caller)",
     E + "PoseidonGL33/EngineGL33_LightingShaders.cpp",
     "    g_lightForced = litMatForced.rgb;" + NL +
     "    vec3 local = toLinear(min(localLights(n, kd, ka) * litGround.w, vec3(1.0))) + g_physicalLocal * litGround.w;",
     "    g_lightForced = litMatForced.rgb;" + NL +
     "    vec3 local = toLinear(min(localLights(n, kd, ka) * litGround.w, vec3(1.0)));"),
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
