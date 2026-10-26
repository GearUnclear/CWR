"""B5 contract mutations: apply every mutation at once (each targets a different
test case), or restore the originals.

    python mutate.py apply     (saves the originals under mutations/orig first)
    python mutate.py restore
"""
import shutil
import sys
from pathlib import Path

ROOT = Path(r"D:\Arma_CWA_worktrees\gr-B5\arma_CWR")
SAVE = Path(__file__).resolve().parent / "mutations" / "orig"

# (file, old, new, what it breaks)
MUTATIONS = [
    ("engine/Poseidon/Graphics/Remaster/SeaWaves.cpp",
     "    b.bound = std::max(0, std::min(frames - 1, bound));\n",
     "    b.bound = std::max(0, std::min(frames - 1, bound));\n    return b; // M1: a hard frame switch\n",
     "M1 the cross-fade is a hard switch"),
    ("engine/Poseidon/Graphics/Remaster/SeaWaves.cpp",
     "2.0f * w * a * kSeaFrameCorrelation",
     "2.0f * w * a * 0.0f",
     "M2 the relief gain ignores the frame correlation"),
    ("engine/Poseidon/Graphics/Remaster/SeaWaves.cpp",
     "    const float high = SeaHighSun(sunElevationDeg);\n",
     "    const float high = 0.0f * SeaHighSun(sunElevationDeg);\n",
     "M3 no high-sun ramp"),
    ("engine/Poseidon/Graphics/Remaster/SeaWaves.cpp",
     "    const float classicLum = Luminance(classicSunDiffuse);\n    const Rgb hue = classicLum > 1e-6f ? classicSunDiffuse * (1.0f / classicLum) : Rgb(1.0f, 1.0f, 1.0f);\n",
     "    const float classicLum = Luminance(ToLinear(classicSunDiffuse));\n    const Rgb hue = classicLum > 1e-6f ? ToLinear(classicSunDiffuse) * (1.0f / classicLum) : Rgb(1.0f, 1.0f, 1.0f);\n",
     "M4 the sheen hue from the linear colour"),
    ("engine/Poseidon/Graphics/Rendering/BuildRenderPassDescriptor.hpp",
     "            if (ctx.remasteredSea && ctx.isIn3DPass &&",
     "            if (false && ctx.remasteredSea && ctx.isIn3DPass &&",
     "M5 the descriptor never routes the sea"),
    ("engine/PoseidonGL33/EngineGL33_Queue.cpp",
     "    ctx.remasteredSea = SeaRouted();\n",
     "",
     "M6 the backend never sets the sea flag"),
    ("engine/PoseidonGL33/EngineGL33_Lighting.hpp",
     '    {"seaNext", kSeaNextFrameUnit},\n',
     "",
     "M7 the next-frame sampler has no unit row"),
    ("engine/PoseidonGL33/EngineGL33_LightingShaders.cpp",
     "    vec4 litWater1;",
     "    vec4 litWaterX;",
     "M8 a block member renamed"),
    ("engine/PoseidonGL33/EngineGL33_LightingShaders.cpp",
     "        grad *= litWater0.x * inversesqrt(max(spread, 1e-6));\n",
     "        grad *= litWater0.x;\n",
     "M9 the shader drops the cross-fade relief gain"),
    ("engine/Poseidon/Graphics/Remaster/FeatureSwitches.cpp",
     '"B5", true, &FS::seaWaves',
     '"B5", false, &FS::seaWaves',
     "M10 the switch not landed"),
    ("engine/PoseidonGL33/EngineGL33_Sea.cpp",
     "    if (!_featureGates.computed || !_featureGates[rm::Feature::SeaWaves].on)\n        return;\n",
     "",
     "M11 the scope ignores the gate"),
    ("engine/Poseidon/World/Terrain/LandscapeRender.cpp",
     "    GEngine->EndSea();\n}",
     "}",
     "M12 the landscape never closes the sea scope"),
]


def apply():
    SAVE.mkdir(parents=True, exist_ok=True)
    for rel in sorted({m[0] for m in MUTATIONS}):
        dst = SAVE / rel.replace("/", "__")
        if not dst.exists():
            shutil.copyfile(ROOT / rel, dst)
    for rel, old, new, what in MUTATIONS:
        p = ROOT / rel
        s = p.read_text(encoding="utf-8")
        n = s.count(old)
        if n != 1:
            raise SystemExit(f"{what}: {n} matches in {rel}")
        p.write_bytes(s.replace(old, new).encode("utf-8"))
        print("applied", what)


def restore():
    for rel in sorted({m[0] for m in MUTATIONS}):
        src = SAVE / rel.replace("/", "__")
        shutil.copyfile(src, ROOT / rel)
        src.unlink()
        print("restored", rel)


if __name__ == "__main__":
    {"apply": apply, "restore": restore}[sys.argv[1]]()
