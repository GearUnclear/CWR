"""B4 review-fix mutation check (scratch): apply or restore the reverts of
the review fixes, one per new contract.

    python mutate.py apply | restore

restore writes the saved bytes back (a new mtime, so ninja rebuilds them).
"""
import sys
from pathlib import Path

ROOT = Path(r"D:\Arma_CWA_worktrees\gr-B4\arma_CWR\engine")
SAVE = Path(r"C:\Users\Dane\AppData\Local\Temp\claude\D--Arma-CWA\0cfe8c80-fc33-4f8e-a86e-52fa1175350d\scratchpad\b4fix\mutation_save")

MUTATIONS = [
    ("N1 smoke transmittance off", "Poseidon/Graphics/Rendering/Lighting/CombatLights.cpp",
     "    if (kind != CombatLightKind::Fire)\n        return 1.0f;\n    const float inside",
     "    if (true)\n        return 1.0f;\n    const float inside"),
    ("N2 steady lights cut like the rest", "Poseidon/Graphics/Rendering/Lighting/CombatLights.cpp",
     "    return kind == CombatLightKind::Fire || kind == CombatLightKind::Flare || kind == CombatLightKind::Motor;",
     "    return false;"),
    ("N3 fire colour back", "Poseidon/Graphics/Rendering/Lighting/CombatLights.cpp",
     "constexpr float kFireRgb[3] = {1.0f, 0.70f, 0.45f};",
     "constexpr float kFireRgb[3] = {1.0f, 0.66f, 0.39f};"),
    ("N4 DrawDecal without the transmittance", "Poseidon/World/Scene/Object.cpp",
     "        irr *= lighting::CombatSmokeTransmittance(r.kind, r.sample, std::sqrt(d2), albedo);\n",
     ""),
    ("N5 fire's lit air back to the 4.4 m ball", "PoseidonGL33/EngineGL33_PostCombat.cpp",
     "            return 2.0f * core + 0.8f;", "            return 3.0f * core + 3.0f;"),
]


def main():
    mode = sys.argv[1]
    files = sorted({m[1] for m in MUTATIONS})
    if mode == "apply":
        SAVE.mkdir(parents=True, exist_ok=True)
        for f in files:
            (SAVE / f.replace("/", "__")).write_bytes((ROOT / f).read_bytes())
        for name, f, old, new in MUTATIONS:
            p = ROOT / f
            data = p.read_bytes().decode("utf-8")
            crlf = "\r\n" in data
            o = old.replace("\n", "\r\n") if crlf else old
            n = new.replace("\n", "\r\n") if crlf else new
            if data.count(o) != 1:
                raise SystemExit(f"{name}: expected one match in {f}, found {data.count(o)}")
            p.write_bytes(data.replace(o, n).encode("utf-8"))
            print("applied", name)
    elif mode == "restore":
        for f in files:
            (ROOT / f).write_bytes((SAVE / f.replace("/", "__")).read_bytes())
            print("restored", f)
    else:
        raise SystemExit("apply or restore")


if __name__ == "__main__":
    main()
