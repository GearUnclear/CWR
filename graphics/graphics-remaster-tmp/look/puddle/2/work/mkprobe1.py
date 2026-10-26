import sys
from pathlib import Path
base = Path('glsl_base/psRemasterCommon.glsl').read_text()
s = base
# global probe
s = s.replace("int g_wetDraw = 0;", "int g_wetDraw = 0;\nvec3 g_probe = vec3(-1.0);", 1)
anchor = "    w = max(w, pud.y);\n    if (w <= 0.0 && pud.x <= 0.0) return;"
assert anchor in s
probe = """    if (ground && g_wetDraw != 2 && litTerrain.x > 0.5) {
        float cell = 1.0 / litTerrain.y;
        float h0 = puddleHeight(posAbs.xz);
        float hx0 = puddleHeight(posAbs.xz - vec2(cell, 0.0));
        float hx1 = puddleHeight(posAbs.xz + vec2(cell, 0.0));
        float hz0 = puddleHeight(posAbs.xz - vec2(0.0, cell));
        float hz1 = puddleHeight(posAbs.xz + vec2(0.0, cell));
        vec2 g = vec2(hx1 - hx0, hz1 - hz0) * (0.5 / cell);
        float hollow = (hx0 + hx1 + hz0 + hz1) * 0.25 - h0;
        g_probe = vec3(clamp(length(g) * 10.0, 0.0, 1.0), clamp(0.5 + hollow * 2.0, 0.0, 1.0), pud.x * 0.5 + (g_wetDraw == 0 ? 0.5 : 0.0));
        if (fract(posAbs.x / cell) < 0.02 || fract(posAbs.z / cell) < 0.02) g_probe = vec3(1.0, 1.0, 1.0);
    }
"""
s = s.replace(anchor, probe + anchor, 1)
dbg = "        return albedo;\n    }\n    if (litFlags.z > 10.5) {"
assert dbg in s
s = s.replace(dbg, "        return g_probe.x >= 0.0 ? g_probe : albedo * 0.3;\n    }\n    if (litFlags.z > 10.5) {", 1)
Path(sys.argv[1]).mkdir(exist_ok=True)
(Path(sys.argv[1]) / 'psRemasterCommon.glsl').write_text(s)
print('ok')
