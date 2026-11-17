"""Dumps candidate D's vertex groups to JSON, run in the Blender the model was built with (5.2):

  blender-launcher.exe -b --factory-startup <dedushka_D.blend> -P dump_weights_52.py -- <out.json>

The P3D add-on is proven in Blender 4.3, which cannot open a 5.2 .blend, and the OBJ the artist
saved for 4.3 carries no weights. export_final_p3d.py --weights matches this dump to the OBJ's
vertices by position. The .blend is not saved. A marker file <out.json>.done is written last.
"""
import json
import sys

import bpy

out = sys.argv[sys.argv.index("--") + 1]
meshes = [o for o in bpy.context.scene.objects if o.type == "MESH" and len(o.data.polygons) > 0 and o.vertex_groups]
obj = max(meshes, key=lambda o: len(o.data.vertices))
names = [vg.name for vg in obj.vertex_groups]
mw = obj.matrix_world
co = []
weights = []
for v in obj.data.vertices:
    p = mw @ v.co
    co.append([round(p.x, 5), round(p.y, 5), round(p.z, 5)])
    weights.append({names[g.group]: round(g.weight, 4) for g in v.groups if g.weight > 0})
with open(out, "w", encoding="utf-8") as fh:
    json.dump({"blender": bpy.app.version_string, "object": obj.name, "groups": names, "co": co, "weights": weights}, fh)
with open(out + ".done", "w") as fh:
    fh.write("ok %d vertices, %d groups, object %s, blender %s\n" % (len(co), len(names), obj.name, bpy.app.version_string))
