"""AFTER check override for the crown-nan fix (research only).

From the fixed remastered GLSL: under debug view 3, paint
  green    any fragment whose lighting normal is still NaN / inf (failure)
  red      foliage whose mesh normal is zero (repaired leaves), brightness =
           the foliage sun term, so a crown lit as a volume shows a bright
           sun side and a dark far side
  grey     everything else
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
# Mark zero mesh normals for the classification only (the fixed VS already
# keeps them zero, this just makes the marker explicit and independent).
old = "    vNormalW         = worldNormal;\n"
assert old in vs
vs = vs.replace(old, "    vNormalW         = dot(normal, normal) < 1e-20 ? vec3(0.0) : worldNormal;\n", 1)
(out / "vsTransformRemaster.glsl").write_text(vs)

ps = raw("s_psRemasterCommon")
old = "    if (litFlags.z > 0.5) {\n        // Debug views"
assert old in ps
ps = ps.replace(
    old,
    "    if (litFlags.z > 2.5 && litFlags.z < 3.5) {\n"
    "        bool bad = any(isnan(nLight)) || any(isinf(nLight)) || any(isnan(radiance)) || any(isinf(radiance));\n"
    "        bool zeroMesh = !(dot(vNormalW, vNormalW) > 0.0);\n"
    "        float g = 0.15 + 0.35 * dot(albedo, kLuma);\n"
    "        if (bad) return vec3(0.0, 1.0, 0.0);\n"
    "        if (foliage && zeroMesh) return vec3(0.15 + 0.85 * sunTerm, 0.0, 0.0);\n"
    "        return vec3(g);\n"
    "    }\n" + old,
    1,
)
(out / "psRemasterCommon.glsl").write_text(ps)
print("ok", out)
