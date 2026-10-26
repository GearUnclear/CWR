import sys
from pathlib import Path
src = Path(sys.argv[1]); out = Path(sys.argv[2]); out.mkdir(exist_ok=True)
s = (src / 'psRemasterCommon.glsl').read_text()
s = s.replace("int g_wetDraw = 0;", "int g_wetDraw = 0;\nvec3 g_probe = vec3(-1.0);", 1)
anchor = "    w = max(w, pud.y);\n    if (w <= 0.0 && pud.x <= 0.0) return;"
assert anchor in s
probe = """    if (ground && g_wetDraw != 2 && litTerrain.x > 0.5) {
        float sl; float bas = puddleBasin(posAbs.xz, sl);
        g_probe = vec3(clamp(sl * 4.0, 0.0, 1.0), clamp(0.5 + bas * 2.0, 0.0, 1.0), clamp(pud.x * 0.7 + pud.y * 0.3, 0.0, 1.0));
    }
"""
s = s.replace(anchor, probe + anchor, 1)
dbg = "        return albedo;\n    }\n    if (litFlags.z > 10.5) {"
assert dbg in s
s = s.replace(dbg, "        return g_probe.x >= 0.0 ? g_probe : albedo * 0.3;\n    }\n    if (litFlags.z > 10.5) {", 1)
(out / 'psRemasterCommon.glsl').write_text(s)
import shutil
for f in src.glob('*.glsl'):
    if f.name != 'psRemasterCommon.glsl': shutil.copy(f, out / f.name)
print('ok')
