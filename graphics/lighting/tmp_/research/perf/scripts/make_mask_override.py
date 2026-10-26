"""Build the perf-mask override of psRemasterCommon.glsl from the engine dump.

The mask rides on the saturation knob (litFlags.w): triSetLighting
"saturation=<10 + mask>" turns on the experiment bits below and keeps the
stock saturation 1.12; a stock saturation (< 9.5) means mask 0, so the
override renders exactly like the engine when no experiment is selected.

Bits:
    1    shadow map lookup off (sun visibility 1)
    2    terrain horizon / sky visibility / ground tint lookups off
    4    cloud shadow lookup off
    8    fogColorToward off (classic fog colour)
    16   glossy off
    32   local lights off
    64   foliage crown path off (foliage lit like any surface)
    128  unlit: shadeRemastered returns the albedo (fragment cost floor)
    256  shadow map: 1 Poisson tap instead of 12
    512  shadow map: primary cascade only (no cross-fade partner)
    1024 shadow map: 4 Poisson taps instead of 12
"""
import sys
from pathlib import Path

src = Path(sys.argv[1]).read_text()
out = Path(sys.argv[2])


def rep(old, new, count=1):
    global src
    n = src.count(old)
    if n != count:
        raise SystemExit(f"expected {count} match(es), found {n}: {old[:60]!r}")
    src = src.replace(old, new)


# Mask decode, placed after the varyings / output declaration.
rep("out vec4 fragColor;\n",
    "out vec4 fragColor;\n\n"
    "// PERF RESEARCH: experiment mask on the saturation knob (see make_mask_override.py).\n"
    "int perfMask() { return litFlags.w > 9.5 ? int(litFlags.w + 0.5) - 10 : 0; }\n"
    "bool perfBit(int b) { return (perfMask() & b) != 0; }\n"
    "float perfSaturation() { return litFlags.w > 9.5 ? 1.12 : litFlags.w; }\n")

# Shadow map: off, tap count, primary only.
rep("float shadowMapVisibility(vec3 n, float nl, float offsetScale) {\n    if (shadowCtl.x < 0.5) return 1.0;\n",
    "float shadowMapVisibility(vec3 n, float nl, float offsetScale) {\n    if (shadowCtl.x < 0.5 || perfBit(1)) return 1.0;\n"
    "    int nTaps = perfBit(256) ? 1 : (perfBit(1024) ? 4 : 12);\n"
    "    int nPass = perfBit(512) ? 1 : 4;\n")
rep("    for (int p = 0; p < 4; ++p) {\n        int c = ci + p;\n        if (c >= nC) break;\n        float w = (p == 0)",
    "    for (int p = 0; p < 4; ++p) {\n        if (p >= nPass) break;\n        int c = ci + p;\n        if (c >= nC) break;\n        float w = (p == 0)")
rep("            for (int k = 0; k < 12; ++k)\n                lit += texture(shadowMap, vec4(suv + kPoisson[k] * radius, float(c), sc.z - bias));\n            litSum += w * (lit / 12.0);",
    "            if (nTaps == 1) lit = texture(shadowMap, vec4(suv, float(c), sc.z - bias));\n"
    "            else for (int k = 0; k < nTaps; ++k)\n                lit += texture(shadowMap, vec4(suv + kPoisson[k * (12 / nTaps)] * radius, float(c), sc.z - bias));\n"
    "            litSum += w * (lit / float(nTaps));")

# Terrain horizon.
rep("    skyVisibility = 1.0;\n    if (litTerrain.x < 0.5 || litFlags.y < 0.5) return 1.0;\n",
    "    skyVisibility = 1.0;\n    if (litTerrain.x < 0.5 || litFlags.y < 0.5 || perfBit(2)) return 1.0;\n")
# Cloud.
rep("    if (litCloud.w < 0.5 || litFlags.y < 0.5) return 1.0;\n",
    "    if (litCloud.w < 0.5 || litFlags.y < 0.5 || perfBit(4)) return 1.0;\n")
# Fog toward the sun.
rep("    if (litFogSun.w < 0.5) return fogColor.rgb;\n",
    "    if (litFogSun.w < 0.5 || perfBit(8)) return fogColor.rgb;\n")
# Local lights.
rep("    int count = int(litLightCount.x);\n",
    "    int count = perfBit(32) ? 0 : int(litLightCount.x);\n")
# Glossy.
rep("    float power = litMatSpecular.w;\n    if (power <= 0.0) return vec3(0.0);\n",
    "    float power = litMatSpecular.w;\n    if (power <= 0.0 || perfBit(16)) return vec3(0.0);\n")
# Unlit floor + foliage off.
rep("vec3 shadeRemastered(vec3 albedo, bool terrain, out float ambientShare) {\n",
    "vec3 shadeRemastered(vec3 albedo, bool terrain, out float ambientShare) {\n"
    "    if (perfBit(128)) { ambientShare = 0.0; return albedo; }\n")
rep("    bool foliage = dot(litMatForced.rgb, kLuma) > 1e-4;\n",
    "    bool foliage = dot(litMatForced.rgb, kLuma) > 1e-4 && !perfBit(64);\n")
# Saturation.
rep("    return max(mix(vec3(grey), display, litFlags.w), vec3(0.0));\n",
    "    return max(mix(vec3(grey), display, perfSaturation()), vec3(0.0));\n")

out.write_text(src)
print("wrote", out)
