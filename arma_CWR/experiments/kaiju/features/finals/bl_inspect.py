import bpy, os, sys
argv = sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []
if argv:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.wm.obj_import(filepath=argv[0], forward_axis="Y", up_axis="Z")
print("VERSION", bpy.app.version_string)
for c in bpy.data.collections:
    print("COL", c.name, [o.name for o in c.objects][:40], dict((k, str(c[k])[:60]) for k in c.keys()))
for ob in bpy.data.objects:
    if ob.type != "MESH":
        print("OBJ", ob.type, ob.name, tuple(round(x,2) for x in ob.location)); continue
    me = ob.data
    xs=[v.co.x for v in me.vertices]; ys=[v.co.y for v in me.vertices]; zs=[v.co.z for v in me.vertices]
    print("MESH", ob.name, "verts", len(me.vertices), "polys", len(me.polygons), "tris", sum(len(p.vertices)-2 for p in me.polygons),
          "uv", [l.name for l in me.uv_layers], "x", round(min(xs),2), round(max(xs),2), "y", round(min(ys),2), round(max(ys),2), "z", round(min(zs),2), round(max(zs),2),
          "hide_render", ob.hide_render, "mw", [tuple(round(x,2) for x in r) for r in ob.matrix_world], "mods", [m.type for m in ob.modifiers])
    print("  groups", [vg.name for vg in ob.vertex_groups])
    print("  attrs", [(a.name, a.domain, a.data_type) for a in me.attributes])
    print("  props", dict((k, str(ob[k])[:80]) for k in ob.keys()))
    cnt = {}
    for p in me.polygons: cnt[p.material_index] = cnt.get(p.material_index,0)+1
    runs = 1 + sum(1 for i in range(1,len(me.polygons)) if me.polygons[i].material_index != me.polygons[i-1].material_index)
    print("  material runs", runs)
    for i, s in enumerate(ob.material_slots):
        m = s.material
        print("  MAT", i, m.name, cnt.get(i), dict((k, str(m[k])) for k in m.keys()))
        if m.use_nodes:
            for n in m.node_tree.nodes:
                if n.type == "TEX_IMAGE" and n.image: print("     img", n.image.name, tuple(n.image.size), n.image.filepath, os.path.exists(bpy.path.abspath(n.image.filepath)))
                if n.type == "BSDF_PRINCIPLED": print("     emis", n.inputs["Emission Strength"].default_value, n.inputs["Emission Strength"].is_linked, n.inputs["Emission Color"].is_linked)
                if n.type == "EMISSION": print("     EMISSION node")
