"""B6 review round mutations: each one undoes a piece of the review fixes in the
scratch copy, builds only the unit test target (the running game's exe is not
touched), runs the [ssr] cases and records whether they turn red. Every file is
restored from the bytes read before the first mutation."""
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(r"D:\Arma_CWA_worktrees\gr-B6\arma_CWR")
SR = ROOT / "engine/Poseidon/Graphics/Remaster/ScreenReflections.cpp"
SH = ROOT / "engine/PoseidonGL33/EngineGL33_PostSsrShaders.cpp"
PS = ROOT / "engine/PoseidonGL33/EngineGL33_PostSsr.cpp"
QU = ROOT / "engine/PoseidonGL33/EngineGL33_Queue.cpp"
EXE = Path(r"E:\gr-lanes\gr-B6\build\gr-B6-rwdi\tests\unit\engine\Poseidon\PoseidonTests.exe")
LOG = Path(r"E:\gr-lanes\gr-B6\evidence\mutations-rev1.log")

MUTATIONS = [
    ("trace fog from the last draw's fog flag", PS,
     'Set4(tr, "fogParam", _ssrWorldFog[0], _ssrWorldFog[1], 0.0f, 0.0f);',
     'Set4(tr, "fogParam", _frameState.fogParams[0], _frameState.fogParams[1], _frameState.fogParams[2], 0.0f);'),
    ("BeginPass does not copy the world fog", QU,
     "            _ssrWorldFog[0] = _frameState.fogParams[0];\n            _ssrWorldFog[1] = _frameState.fogParams[1];\n",
     ""),
    ("march mirror without the plane test", SR,
     "else if (planeDist && planeDist[x] < SsrSelfTolerance(sz))",
     "else if (false && planeDist && planeDist[x] < SsrSelfTolerance(sz))"),
    ("trace without the plane test", SH,
     "            else if (plane.w > 0.5 && dot(plane.xyz, viewOfUv(uvOfHalf(s), sz) - p0) < kSsrSelfBase + kSsrSelfPerM * sz) cand = false;\n",
     ""),
    ("march mirror passes panels without keeping them", SR,
     "                    out.cover = true;\n",
     ""),
    ("panels always opaque (rays stop on bush cards)", SR,
     "    return Smoothstep(kSsrCoverSeeFrom, kSsrCoverSeeTo, d);",
     "    return d >= 0.0f ? 1.0f : 0.0f;"),
    ("march mirror: a window in a solid body counts as free-standing", SR,
     "out.afterInFront = sz < kSsrSkyDepth && std::min(zPrev, z) > sz + thick;",
     "out.afterInFront = false;"),
    ("trace: a panel set in something solid is not opaque", SH,
     "if (cover.w < 1.5 || cover.w > 2.5) return 1.0;",
     "if (cover.w < 1.5) return 1.0;"),
    ("ground trace does not lay the panel over", SH,
     "            use = mix(use, 1.0, wK);\n",
     ""),
    ("blur back to 8 fixed taps", SR,
     "    return static_cast<int>(std::clamp(std::ceil(spread), 1.0f, static_cast<float>(kSsrBlurMaxTaps)));",
     "    return 8;"),
    ("streak level from the horizontal spread only", SR,
     "    s.lod = std::clamp(std::log2(std::max(std::max(pxH, s.stepPx), 1.0f)), 0.0f, std::max(maxLod, 0.0f));",
     "    s.taps = 3;\n    s.stepPx = span / 3.0f;\n    s.lod = std::clamp(std::log2(std::max(pxH, 1.0f)), 0.0f, std::max(maxLod, 0.0f));"),
]


def env():
    e = dict(os.environ)
    e["VCPKG_ROOT"] = r"C:\dev\vcpkg"
    e["PATH"] = ";".join([r"C:\Program Files\LLVM\bin", r"C:\Program Files\CMake\bin",
                          r"C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe",
                          r"C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64",
                          e.get("PATH", "")])
    return e


def build():
    r = subprocess.run(["cmake", "--build", "build/gr-B6-rwdi", "-j", "8", "--target", "PoseidonTests"], cwd=ROOT,
                       env=env(), capture_output=True, text=True)
    return r.returncode, r.stdout[-2000:] + r.stderr[-2000:]


def run_tests():
    r = subprocess.run([str(EXE), "[ssr]"], cwd="D:\\", capture_output=True, text=True)
    tail = [ln for ln in r.stdout.splitlines() if "test cases" in ln or "All tests passed" in ln]
    return r.returncode, tail[-1] if tail else r.stdout[-300:]


def main():
    originals = {p: p.read_bytes() for p in {m[1] for m in MUTATIONS}}
    results = []
    try:
        for name, path, old, new in MUTATIONS:
            text = originals[path].decode("utf-8").replace("\r\n", "\n")
            if text.count(old) != 1:
                results.append((name, f"SKIP: pattern found {text.count(old)} times"))
                continue
            path.write_bytes(text.replace(old, new).replace("\n", "\r\n").encode("utf-8"))
            rc, out = build()
            if rc != 0:
                results.append((name, "BUILD FAILED: " + out.strip().splitlines()[-1] if out.strip() else "BUILD FAILED"))
            else:
                trc, tail = run_tests()
                results.append((name, ("RED " if trc != 0 else "GREEN (mutation survived) ") + tail))
            path.write_bytes(originals[path])
            print(results[-1], flush=True)
    finally:
        for p, b in originals.items():
            p.write_bytes(b)
    rc, out = build()
    trc, tail = run_tests()
    restored = f"restored build: {rc}, tests: {tail}"
    print(restored)
    with LOG.open("w", encoding="utf-8") as f:
        for name, res in results:
            f.write(f"{name:52s} {res}\n")
        f.write(restored + "\n")


if __name__ == "__main__":
    sys.exit(main())
