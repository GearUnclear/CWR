"""override_lod: override_cutout plus shadow-tap LOD bits (research only).

  4096  foliage fragments take 4 Poisson taps instead of 12 (leaf texture hides the penumbra)
  8192  shadow tiers past the first (c > 0, farther from the camera) take 4 taps

usage: make_lod_override.py <override_cutout dir> <override_lod dir>
"""
import sys
from pathlib import Path

src, dst = Path(sys.argv[1]), Path(sys.argv[2])
dst.mkdir(parents=True, exist_ok=True)
for f in src.glob("*.glsl"):
    if f.name != "psRemasterCommon.glsl":
        (dst / f.name).write_bytes(f.read_bytes())
text = (src / "psRemasterCommon.glsl").read_bytes().decode("utf-8").replace("\r\n", "\n")


def sub(old, new):
    global text
    if text.count(old) != 1:
        raise SystemExit(f"pattern found {text.count(old)} times: {old[:60]!r}")
    text = text.replace(old, new)


sub("float shadowMapVisibility(vec3 n, float nl, float offsetScale) {\n",
    "// PERF RESEARCH (override_lod): set by shadeRemastered before the shadow lookup.\n"
    "bool g_foliageFrag = false;\n"
    "float shadowMapVisibility(vec3 n, float nl, float offsetScale) {\n")
sub("            if (nTaps == 1) lit = texture(shadowMap, vec4(suv, float(c), sc.z - bias));\n"
    "            else for (int k = 0; k < nTaps; ++k)\n"
    "                lit += texture(shadowMap, vec4(suv + kPoisson[k * (12 / nTaps)] * radius, float(c), sc.z - bias));\n"
    "            litSum += w * (lit / float(nTaps));\n",
    "            // PERF RESEARCH (override_lod): 4096 = foliage takes 4 taps, 8192 = tiers\n"
    "            // past the first take 4 taps.\n"
    "            int nt = nTaps;\n"
    "            if (perfBit(4096) && g_foliageFrag) nt = min(nt, 4);\n"
    "            if (perfBit(8192) && c > 0) nt = min(nt, 4);\n"
    "            if (nt == 1) lit = texture(shadowMap, vec4(suv, float(c), sc.z - bias));\n"
    "            else for (int k = 0; k < nt; ++k)\n"
    "                lit += texture(shadowMap, vec4(suv + kPoisson[k * (12 / nt)] * radius, float(c), sc.z - bias));\n"
    "            litSum += w * (lit / float(nt));\n")
sub("    bool foliage = dot(litMatForced.rgb, kLuma) > 1e-4 && !perfBit(64);\n",
    "    bool foliage = dot(litMatForced.rgb, kLuma) > 1e-4 && !perfBit(64);\n"
    "    g_foliageFrag = foliage;\n")
(dst / "psRemasterCommon.glsl").write_bytes(text.encode("utf-8"))
print("wrote", dst)
