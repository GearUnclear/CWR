"""Diagnostic shader override for the crown-nan bug (research only).

Takes the remastered GLSL of the branch tip and, under debug view 3
(triSetLighting "debug=3"), paints each foliage fragment by the source of a bad
lighting normal instead of the normal itself:

  red     the vertex carries a zero mesh normal (normalize(0) in the VS)
  yellow  the crown uniforms (centre / inverse half extents) are not finite
  green   vCrown is NaN / inf although the crown uniforms are finite
  blue    vNormalW is NaN / inf although the mesh normal is not zero
  grey    foliage with a finite normal and a finite crown frame (healthy)
  non-foliage: dark, magenta where its mesh normal is zero
"""
import re
import sys
from pathlib import Path

src_cpp = Path(sys.argv[1]).read_text()
out = Path(sys.argv[2])
out.mkdir(parents=True, exist_ok=True)


def raw(name):
    m = re.search(r'static const char ' + name + r'\[\] = R"\((.*?)\)";', src_cpp, re.S)
    assert m, name
    return m.group(1)


vs = raw("s_vsTransformRemasterGLSL")
old = "    vNormalW         = worldNormal;\n"
assert old in vs
vs = vs.replace(old, "    vNormalW         = dot(normal, normal) < 1e-20 ? vec3(0.0) : worldNormal;\n", 1)
old = "    if (crownCenter.w > 0.5) {\n"
assert old in vs
vs = vs.replace(
    old,
    "    bool crownBad = any(isnan(crownCenter.xyz)) || any(isinf(crownCenter.xyz)) ||\n"
    "                    any(isnan(crownScale.xyz)) || any(isinf(crownScale.xyz));\n"
    "    if (crownCenter.w > 0.5 && crownBad) {\n"
    "        vCrown = vec4(0.0, 0.0, 0.0, -1.0);\n"
    "    } else if (crownCenter.w > 0.5) {\n",
    1,
)
(out / "vsTransformRemaster.glsl").write_text(vs)

ps = raw("s_psRemasterCommon")
old = "vec3 shadeRemastered(vec3 albedo, bool terrain, out float ambientShare) {\n"
assert old in ps
ps = ps.replace(
    old,
    old
    + "    if (litFlags.z > 2.5 && litFlags.z < 3.5) {\n"
    "        ambientShare = 0.0;\n"
    "        bool fol = dot(litMatForced.rgb, kLuma) > 1e-4;\n"
    "        bool zeroN = !(dot(vNormalW, vNormalW) > 0.0) && !(any(isnan(vNormalW)) || any(isinf(vNormalW)));\n"
    "        bool nanN = any(isnan(vNormalW)) || any(isinf(vNormalW));\n"
    "        bool crownUniBad = vCrown.w < 0.0;\n"
    "        bool crownNan = any(isnan(vCrown)) || any(isinf(vCrown));\n"
    "        float g = 0.15 + 0.35 * dot(albedo, kLuma);\n"
    "        if (!fol) return zeroN ? vec3(1.0, 0.0, 1.0) : vec3(g * 0.4);\n"
    "        if (zeroN) return vec3(1.0, 0.0, 0.0);\n"
    "        if (crownUniBad) return vec3(1.0, 1.0, 0.0);\n"
    "        if (crownNan) return vec3(0.0, 1.0, 0.0);\n"
    "        if (nanN) return vec3(0.0, 0.0, 1.0);\n"
    "        return vec3(g);\n"
    "    }\n",
    1,
)
(out / "psRemasterCommon.glsl").write_text(ps)
print("ok", out)
