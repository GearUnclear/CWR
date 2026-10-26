"""Patch the stock psRemasterCommon.glsl (dumped with glsl_extract.py) so a
terrain main can hand shadeRemastered a perturbed normal.

    python patch_common.py <stock psRemasterCommon.glsl> <out psRemasterCommon.glsl>
"""
import sys
from pathlib import Path

src = Path(sys.argv[1]).read_text(encoding="utf-8").replace("\r\n", "\n")
old_n = "    vec3 n = vNormalW * inversesqrt(max(dot(vNormalW, vNormalW), 1e-12));\n    vec3 posAbs = vWorldRel + litCamPos.xyz;"
new_n = ("    // texsharp: a terrain main may hand in a perturbed normal (relief from albedo).\n"
         "    vec3 n = g_bumpOn ? g_bumpN : vNormalW * inversesqrt(max(dot(vNormalW, vNormalW), 1e-12));\n"
         "    vec3 posAbs = vWorldRel + litCamPos.xyz;")
assert src.count(old_n) == 1, "normal line not found"
src = src.replace(old_n, new_n)
anchor = "// Sun visibility of the last shaded fragment (water reuses it for its glint)."
assert src.count(anchor) == 1
src = src.replace(anchor, "// texsharp: optional perturbed shading normal, set by psDetailRemasterMain.\n"
                          "bool g_bumpOn = false;\nvec3 g_bumpN = vec3(0.0, 1.0, 0.0);\n\n" + anchor)
Path(sys.argv[2]).write_text(src, encoding="utf-8", newline="\n")
print("patched", sys.argv[2])
