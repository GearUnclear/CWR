"""Export a Dedushka .blend to the SP3X MLOD the uddedushka addon packs.

Headless, Blender 4.3 (the .blend is never saved):

  blender --background --factory-startup <model.blend> --python export_dedushka_p3d.py -- <out_dir>
          [--objects a,b,c] [--exclude a,b] [--yaw 180] [--no-ground]

Writes into <out_dir>:
  dedushka.p3d            the model (8 or 9 LODs)
  tex\<name>.png          one PNG per face texture; the build script turns each into a PAA
  dedushka_rest.json      joint pivots for the RTM generator (only when the model has joints)
  dedushka_export.json    what was done: mode, objects taken and skipped, bounds, LODs, warnings

Two kinds of input:

* RIGGED: the blend has a collection "dedushka" holding LOD collections (the
  proxy from proxy/build_proxy.py is the worked example). It is exported as
  it is. Joint pivots come from the collection property ud_rest_json.

* WRAP: anything else, such as an art candidate that is one sculpted mesh with
  painted textures and scale props beside it. The driver wraps it into a
  loadable Man model: joins the creature's meshes, makes three coarser visual
  LODs with a Decimate modifier, builds Geometry, FireGeometry, Memory,
  LandContact and HitPoints from the bounding box, and weights every vertex
  to the root bone (a statue: enough for stills, not for animation).
  Which meshes are the creature: --objects, else every renderable mesh whose
  centre lies inside the plan-view footprint of the tallest mesh (scale props
  stand beside it and are skipped; the summary lists both sets).
  Facing: the game wants the creature to face Blender -Y with its left at +X.
  The art candidates are built facing +Y (ART-BRIEF did not fix a facing), so
  wrap mode turns the model by --yaw degrees about Z, 180 by default. Pass
  --yaw 0 for a model that already faces -Y.

In both modes textures are retargeted to the pbo prefix (uddedushka\...),
because a pbo's file name is its path prefix, and a material is exported
fullbright (face flag 0x1, FACE_NOLIGHT) when it carries p3d_fullbright, has
an emission strength above zero, or is named like glow, furnace, fire, ember
or emis. The engine has no emissive maps: fullbright faces show the painted
colour at full strength whatever the light (verified at night with the proxy).
"""
import json
import math
import os
import re
import sys

import bpy
from mathutils import Matrix, Vector

HERE = os.path.dirname(os.path.abspath(__file__))
ENGINE_ROOT = os.path.normpath(os.path.join(HERE, "..", "..", ".."))
ADDON_DIR = os.path.join(ENGINE_ROOT, "apps", "tools", "BlenderAddon")

ADDON = "uddedushka"
ROOT_COLLECTION = "dedushka"
FACE_NOLIGHT = 0x1
MASS_KG = 2.0e7
# Wrap mode LOD ladder, as fractions of the model height (LOD0 is always 1).
# Measured with the proxy: the engine switches to the last LOD whose value is
# at most distance * k, k = 0.09 to 0.14 per metre at the normal field of
# view when frames are slow (up to 16 times smaller when they are fast), and
# clamps every value to twice the bounding sphere, which is never less than
# the height. So the coarsest LOD is what a loaded scene shows from about
# 300 m on: it must still look right there.
LOD_FRACTION = [None, 0.45, 0.70, 0.95]
LOD_RATIO = [1.0, 0.40, 0.15, 0.05]
LOD_MIN_TRIS = 150
MAX_TEX = 1024
GLOW_NAME = re.compile(r"glow|furnace|fire|ember|emis|lava|reactor", re.I)
STOCK_BONES = (
    "pchodidlo", "lchodidlo", "pprsty", "lprsty", "lholen", "pholen", "pstehno", "lstehno",
    "pzadek", "lzadek", "bricho", "zebra", "hrudnik", "krk", "prameno", "lrameno", "hlava",
    "pbiceps", "lbiceps", "ploket", "lloket", "roura", "zbran", "pruka", "lruka",
)

argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
if not argv:
    raise SystemExit("usage: blender -b <model.blend> --python export_dedushka_p3d.py -- <out_dir> [options]")
OUT_DIR = os.path.abspath(argv[0])
OPT = {"objects": None, "exclude": [], "ground": True, "yaw": 180.0}
i = 1
while i < len(argv):
    a = argv[i]
    if a == "--objects":
        OPT["objects"] = [s for s in argv[i + 1].split(",") if s]
        i += 1
    elif a == "--exclude":
        OPT["exclude"] = [s for s in argv[i + 1].split(",") if s]
        i += 1
    elif a == "--no-ground":
        OPT["ground"] = False
    elif a == "--yaw":
        OPT["yaw"] = float(argv[i + 1])
        i += 1
    else:
        raise SystemExit("unknown option %s" % a)
    i += 1
os.makedirs(os.path.join(OUT_DIR, "tex"), exist_ok=True)

sys.path.insert(0, ADDON_DIR)
import io_import_p3d as addon  # noqa: E402

addon.register()
from io_import_p3d.export_p3d import export_p3d  # noqa: E402

WARN = []
SUMMARY = {"blend": bpy.data.filepath, "warnings": WARN}


def warn(msg):
    WARN.append(msg)
    print("[uddedushka] warning:", msg, flush=True)


# ---------------------------------------------------------------------------
# Textures
# ---------------------------------------------------------------------------
def pow2(n):
    p = 8
    while p * 2 <= min(n, MAX_TEX):
        p *= 2
    return p


def base_image(mat):
    """(image or None, flat colour) for a material: its base colour, or its emission when it glows."""
    colour = (0.5, 0.5, 0.5)
    image = None
    if mat.use_nodes and mat.node_tree:
        bsdf = next((n for n in mat.node_tree.nodes if n.type == "BSDF_PRINCIPLED"), None)
        sockets = []
        if bsdf is not None:
            # Blender 4 defaults: emission colour white at strength 0, so the
            # emission socket only counts when the strength is above zero.
            strength = bsdf.inputs.get("Emission Strength")
            emits = strength is not None and (strength.is_linked or strength.default_value > 0.0)
            if emits and "Emission Color" in bsdf.inputs:
                sockets.append(bsdf.inputs["Emission Color"])
            sockets.append(bsdf.inputs["Base Color"])
        for sock in sockets:
            if sock.is_linked:
                node = sock.links[0].from_node
                seen = 0
                while node is not None and node.type != "TEX_IMAGE" and seen < 8:   # walk through mix or ramp nodes
                    nxt = next((l.from_node for inp in node.inputs for l in inp.links), None)
                    node = nxt
                    seen += 1
                if node is not None and node.type == "TEX_IMAGE" and node.image:
                    image = node.image
                    break
            else:
                c = sock.default_value
                if sum(c[:3]) > 0.02 or sock is sockets[-1]:
                    colour = tuple(c[:3])
                    break
        if image is None and bsdf is None:
            node = next((n for n in mat.node_tree.nodes if n.type == "TEX_IMAGE" and n.image), None)
            image = node.image if node else None
    else:
        colour = tuple(mat.diffuse_color[:3])
    return image, colour


def is_glow(mat):
    if mat.get("p3d_fullbright"):
        return True
    if GLOW_NAME.search(mat.name):
        return True
    if mat.use_nodes and mat.node_tree:
        for n in mat.node_tree.nodes:
            if n.type == "BSDF_PRINCIPLED" and "Emission Strength" in n.inputs:
                s = n.inputs["Emission Strength"]
                e = n.inputs.get("Emission Color")
                lit = e is not None and (e.is_linked or sum(e.default_value[:3]) > 0.02)
                if lit and (s.is_linked or s.default_value > 0.0):
                    return True
            if n.type == "EMISSION":
                return True
    return False


def linear_to_srgb(c):
    return 12.92 * c if c <= 0.0031308 else 1.055 * (c ** (1.0 / 2.4)) - 0.055


def stage_textures(objects):
    """Give every material of ``objects`` a uddedushka\\ texture and save its PNG. Returns the manifest."""
    manifest = []
    done = {}
    used = []
    for obj in objects:
        for slot in obj.material_slots:
            if slot.material and slot.material not in used:
                used.append(slot.material)
    for mat in used:
        tex = str(mat.get("p3d_texture") or "").replace("/", "\\").lstrip("\\")
        if tex.lower().startswith(ADDON + "\\") and len(tex) <= 31:
            stem = os.path.splitext(tex.split("\\", 1)[1])[0]
        else:
            stem = "ded_t%d" % len(done)
        image, colour = base_image(mat)
        key = image.name if image else ("flat", tuple(round(c, 3) for c in colour))
        if key in done:
            stem = done[key]
        else:
            done[key] = stem
            png = os.path.join(OUT_DIR, "tex", stem + ".png")
            if image is not None:
                w, h = image.size[0], image.size[1]
                if w == 0 or h == 0:
                    warn("material '%s': image '%s' has no pixels; using a flat colour" % (mat.name, image.name))
                    image = None
                else:
                    # Copy the pixels (this also forces a file-backed image to load;
                    # Image.copy() of an unloaded image has nothing to save).
                    cp = bpy.data.images.new(stem, w, h, alpha=True)
                    cp.pixels = image.pixels[:]
                    if image.is_float:
                        warn("texture '%s' is a float image; its colours are written without a view transform"
                             % image.name)
                    tw, th = pow2(w), pow2(h)
                    if (tw, th) != (w, h):
                        cp.scale(tw, th)
                        warn("texture '%s' resized from %dx%d to %dx%d (PAA needs powers of two, at most %d)"
                             % (image.name, w, h, tw, th, MAX_TEX))
                    cp.filepath_raw = png
                    cp.file_format = "PNG"
                    cp.save()
                    bpy.data.images.remove(cp)
                    manifest.append({"png": png, "paa": stem + ".paa", "material": mat.name,
                                     "source": image.name, "size": [tw, th]})
            if image is None:
                img = bpy.data.images.new(stem, 8, 8, alpha=False)
                r, g, b = (min(1.0, max(0.0, linear_to_srgb(c))) for c in colour)
                img.pixels = [r, g, b, 1.0] * 64
                img.filepath_raw = png
                img.file_format = "PNG"
                img.save()
                bpy.data.images.remove(img)
                manifest.append({"png": png, "paa": stem + ".paa", "material": mat.name,
                                 "source": "flat colour", "size": [8, 8]})
        mat["p3d_texture"] = "%s\\%s.paa" % (ADDON, stem)
    return manifest


def authored_glow_slots(obj):
    """Material slots whose faces the artist already flagged FACE_NOLIGHT (more than half of them)."""
    mesh = obj.data
    attr = mesh.attributes.get("p3d_face_flags")
    if attr is None or attr.domain != "FACE" or attr.data_type != "INT":
        return set()
    total, lit = {}, {}
    for poly in mesh.polygons:
        total[poly.material_index] = total.get(poly.material_index, 0) + 1
        if attr.data[poly.index].value & FACE_NOLIGHT:
            lit[poly.material_index] = lit.get(poly.material_index, 0) + 1
    return {i for i, n in lit.items() if n * 2 > total[i]}


def flag_glow_faces(obj, extra_slots=()):
    """Write p3d_face_flags = FACE_NOLIGHT on the faces of glowing materials. Returns the face count."""
    mesh = obj.data
    glow_slots = {i for i, s in enumerate(obj.material_slots) if s.material and is_glow(s.material)}
    glow_slots |= set(extra_slots)
    if not glow_slots or not mesh.polygons:
        return 0
    attr = mesh.attributes.get("p3d_face_flags")
    if attr is None:
        attr = mesh.attributes.new("p3d_face_flags", "INT", "FACE")
    n = 0
    for poly in mesh.polygons:
        if poly.material_index in glow_slots:
            attr.data[poly.index].value |= FACE_NOLIGHT
            n += 1
    return n


# ---------------------------------------------------------------------------
# Wrap mode
# ---------------------------------------------------------------------------
def world_bounds(obj):
    pts = [obj.matrix_world @ Vector(c) for c in obj.bound_box]
    lo = Vector((min(p.x for p in pts), min(p.y for p in pts), min(p.z for p in pts)))
    hi = Vector((max(p.x for p in pts), max(p.y for p in pts), max(p.z for p in pts)))
    return lo, hi


def pick_creature_objects():
    meshes = [o for o in bpy.context.scene.objects if o.type == "MESH" and len(o.data.polygons) > 0]
    skipped = {}
    cand = []
    for o in meshes:
        if o.get("p3d_ignore"):
            skipped[o.name] = "p3d_ignore"
        elif o.name in OPT["exclude"]:
            skipped[o.name] = "--exclude"
        elif OPT["objects"] is not None:
            if o.name in OPT["objects"]:
                cand.append(o)
            else:
                skipped[o.name] = "not in --objects"
        elif o.hide_render:
            skipped[o.name] = "hidden from render"
        else:
            cand.append(o)
    if not cand:
        raise SystemExit("no mesh objects to export")
    if OPT["objects"] is None:
        tallest = max(cand, key=lambda o: world_bounds(o)[1].z - world_bounds(o)[0].z)
        lo, hi = world_bounds(tallest)
        keep = []
        for o in cand:
            olo, ohi = world_bounds(o)
            c = (olo + ohi) * 0.5
            if lo.x <= c.x <= hi.x and lo.y <= c.y <= hi.y:
                keep.append(o)
            else:
                skipped[o.name] = "outside the footprint of '%s' (a scale prop?)" % tallest.name
        cand = keep
    return cand, skipped


def join_as_mesh(objects, name, collection):
    """One new object holding copies of ``objects`` with modifiers and world transforms applied."""
    deps = bpy.context.evaluated_depsgraph_get()
    copies = []
    for o in objects:
        for m in o.modifiers:
            if m.type == "ARMATURE":
                m.show_viewport = False
    bpy.context.view_layer.update()
    deps = bpy.context.evaluated_depsgraph_get()
    for o in objects:
        mesh = bpy.data.meshes.new_from_object(o.evaluated_get(deps), preserve_all_data_layers=True, depsgraph=deps)
        mesh.transform(o.matrix_world)
        if o.matrix_world.determinant() < 0:
            mesh.flip_normals()
        c = bpy.data.objects.new(name + "_part", mesh)
        collection.objects.link(c)
        for vg in o.vertex_groups:
            if vg.name not in c.vertex_groups:
                c.vertex_groups.new(name=vg.name)
        # One UV layer called UVMap, so a join lines the layers up.
        if mesh.uv_layers:
            act = next((l for l in mesh.uv_layers if l.active_render), mesh.uv_layers.active) or mesh.uv_layers[0]
            keep = act.name
            for layer in [l for l in mesh.uv_layers if l.name != keep]:
                mesh.uv_layers.remove(layer)
            mesh.uv_layers[0].name = "UVMap"
        copies.append(c)
    bpy.context.view_layer.update()
    if len(copies) > 1:
        with bpy.context.temp_override(active_object=copies[0], object=copies[0],
                                       selected_objects=copies, selected_editable_objects=copies):
            bpy.ops.object.join()
    obj = copies[0]
    obj.name = name
    obj.data.name = name
    return obj


def tri_count(mesh):
    return sum(len(p.vertices) - 2 for p in mesh.polygons)


def decimated_copy(src, name, collection, ratio):
    obj = src.copy()
    obj.data = src.data.copy()
    obj.name = name
    collection.objects.link(obj)
    mod = obj.modifiers.new("lod", "DECIMATE")
    mod.decimate_type = "COLLAPSE"
    mod.ratio = ratio
    mod.use_collapse_triangulate = True
    bpy.context.view_layer.update()
    deps = bpy.context.evaluated_depsgraph_get()
    mesh = bpy.data.meshes.new_from_object(obj.evaluated_get(deps), preserve_all_data_layers=True, depsgraph=deps)
    obj.modifiers.clear()
    old = obj.data
    obj.data = mesh
    bpy.data.meshes.remove(old)
    return obj


def lod_collection(root, name, resolution, props=None):
    col = bpy.data.collections.new(name)
    root.children.link(col)
    col["p3d_resolution"] = resolution
    for k, v in (props or {}).items():
        col["p3d_prop_" + k] = v
    return col


BOX_FACES = [(0, 3, 2, 1), (4, 5, 6, 7), (0, 1, 5, 4), (1, 2, 6, 5), (2, 3, 7, 6), (3, 0, 4, 7)]


def helper_object(name, collection, boxes=(), points=()):
    """A mesh of boxes [(group, (x0,x1), (y0,y1), (z0,z1))] and loose points [(co, [groups])]."""
    verts, faces, groups = [], [], {}
    for group, (x0, x1), (y0, y1), (z0, z1) in boxes:
        base = len(verts)
        verts += [(x0, y0, z0), (x1, y0, z0), (x1, y1, z0), (x0, y1, z0),
                  (x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1)]
        faces += [tuple(base + k for k in f) for f in BOX_FACES]
        groups.setdefault(group, []).extend(range(base, base + 8))
    for co, names in points:
        verts.append(tuple(co))
        for g in names:
            groups.setdefault(g, []).append(len(verts) - 1)
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(name, mesh)
    collection.objects.link(obj)
    for g, idx in groups.items():
        obj.vertex_groups.new(name=g).add(idx, 1.0, "REPLACE")
    return obj


def wrap():
    objects, skipped = pick_creature_objects()
    SUMMARY["objects"] = [o.name for o in objects]
    SUMMARY["skipped"] = skipped
    print("[uddedushka] wrap: creature =", SUMMARY["objects"], "skipped =", skipped, flush=True)
    manifest = stage_textures(objects)

    root = bpy.data.collections.new(ROOT_COLLECTION)
    bpy.context.scene.collection.children.link(root)
    lod0_col = lod_collection(root, "lod0", 1.0, {"lodnoshadow": "1"})
    lod0 = join_as_mesh(objects, "dedushka_lod0", lod0_col)

    if abs(OPT["yaw"]) > 1e-6:
        lod0.data.transform(Matrix.Rotation(math.radians(OPT["yaw"]), 4, "Z"))
        lod0.data.update()
    SUMMARY["yaw_deg"] = OPT["yaw"]
    lo, hi = world_bounds(lod0)
    if OPT["ground"] and abs(lo.z) > 0.05:
        warn("lowest point is at z = %.2f m; moved by %.2f m so the feet stand on z = 0" % (lo.z, -lo.z))
        lod0.data.transform(Matrix.Translation((0, 0, -lo.z)))
        lod0.data.update()
        lo, hi = world_bounds(lod0)
    size = hi - lo
    height = hi.z
    SUMMARY["bounds"] = {"min": [round(c, 3) for c in lo], "max": [round(c, 3) for c in hi]}
    if height < 20 or height > 60:
        warn("the model is %.1f m tall; Dedushka should be 35 to 45 (1 Blender unit = 1 m)" % height)
    if abs((lo.x + hi.x) * 0.5) > 3 or abs((lo.y + hi.y) * 0.5) > 6:
        warn("the footprint centre is %.1f, %.1f m from the origin; the origin must be on the ground "
             "between the feet" % ((lo.x + hi.x) * 0.5, (lo.y + hi.y) * 0.5))
    reach = max(abs(lo.x), abs(hi.x), abs(lo.y), abs(hi.y))
    if reach > 25:
        warn("a part reaches %.1f m from the origin in plan view; the engine's object grid assumes 25 m "
             "(Collisions.cpp:51-55), keep it within 20" % reach)

    # Skin: keep stock-named vertex groups when they cover every vertex, else a statue.
    named = [vg for vg in lod0.vertex_groups if vg.name.lower() in STOCK_BONES]
    named_idx = {vg.index for vg in named}
    covered = bool(named) and all(any(g.group in named_idx and g.weight > 0 for g in v.groups) for v in lod0.data.vertices)
    for vg in [vg for vg in lod0.vertex_groups if vg.name.lower() not in STOCK_BONES]:
        lod0.vertex_groups.remove(vg)
    if covered:
        SUMMARY["skin"] = "vertex groups (%d bones)" % len(named)
    else:
        for vg in list(lod0.vertex_groups):
            lod0.vertex_groups.remove(vg)
        lod0.vertex_groups.new(name="bricho").add(range(len(lod0.data.vertices)), 1.0, "REPLACE")
        SUMMARY["skin"] = "statue (every vertex on bricho)"
    tris0 = tri_count(lod0.data)
    lods = [lod0]
    for k in (1, 2, 3):
        target = max(LOD_MIN_TRIS, tris0 * LOD_RATIO[k])
        ratio = min(1.0, target / max(1, tris0))
        col = lod_collection(root, "lod%d" % k, round(LOD_FRACTION[k] * height, 2),
                             {"lodnoshadow": "1"} if k < 2 else None)
        lods.append(decimated_copy(lod0, "dedushka_lod%d" % k, col, ratio))
    # Fullbright faces are flagged per LOD from the material, after the
    # decimation (a Decimate modifier does not carry face attributes over reliably).
    authored = authored_glow_slots(lod0)
    glow = [flag_glow_faces(lod0, authored)]
    for o in lods[1:]:
        stale = o.data.attributes.get("p3d_face_flags")
        if stale is not None:
            o.data.attributes.remove(stale)
        glow.append(flag_glow_faces(o, authored))
    SUMMARY["fullbright_faces"] = glow
    SUMMARY["lod_triangles"] = [tri_count(o.data) for o in lods]
    corners = len({(l.vertex_index, tuple(round(c, 4) for c in n.vector))
                   for l, n in zip(lod0.data.loops, lod0.data.corner_normals)})
    SUMMARY["lod0_split_vertices_estimate"] = corners
    if corners > 32767:
        warn("LOD0 has about %d vertices after normal splits; the engine limit is 32767 per LOD" % corners)

    # Helper LODs from the bounding box. x: left/right, y: front is -y, z: up.
    w = max(abs(lo.x), abs(hi.x))
    y0, y1 = lo.y, hi.y
    H = height
    trunk = ("bricho", (-0.62 * w, 0.62 * w), (0.75 * y0, 0.75 * y1), (0.46 * H, 0.86 * H))
    head = ("bricho", (-0.22 * w, 0.22 * w), (0.75 * y0, 0.5 * y1), (0.86 * H, H))
    legl = ("bricho", (0.12 * w, 0.55 * w), (0.6 * y0, 0.6 * y1), (0.0, 0.46 * H))
    legp = ("bricho", (-0.55 * w, -0.12 * w), (0.6 * y0, 0.6 * y1), (0.0, 0.46 * H))
    geo = lod_collection(root, "Geometry", "Geometry", {"autocenter": "0"})
    geo["p3d_mass"] = MASS_KG
    helper_object("dedushka_geometry", geo, boxes=[trunk, head, legl, legp])
    fire = lod_collection(root, "FireGeometry", "FireGeometry")
    helper_object("dedushka_firegeometry", fire, boxes=[trunk, head, legl, legp])
    mem = lod_collection(root, "Memory", "Memory")
    foot = 0.33 * w
    helper_object("dedushka_memory", mem, points=[
        ((0, 0.8 * y0, 0.78 * H), ["zamerny", "bricho"]),
        ((0, 0, 0.74 * H), ["osa mireni", "bricho"]),
        ((0, 0, 0.86 * H), ["osa otaceni", "bricho"]),
        ((0, 0.8 * y0, 0.95 * H), ["pilot", "bricho"]),
        ((foot, 0, 0.05), ["stopaL", "bricho"]),
        ((-foot, 0, 0.05), ["stopaP", "bricho"]),
        ((foot, 0, 0.06 * H), ["lchodidlo", "bricho"]),
        ((-foot, 0, 0.06 * H), ["pchodidlo", "bricho"]),
        ((0, 0, 0.6 * H), ["bricho"]),
        ((0, 0, 0.7 * H), ["zebra", "bricho"]),
        ((0, 0, 0.8 * H), ["hrudnik", "bricho"]),
        ((0, 0, 0.93 * H), ["hlava", "bricho"]),
        ((-0.9 * w, 0, 0.35 * H), ["zbran", "bricho"]),
        ((-0.9 * w, -1, 0.35 * H), ["zbran", "bricho"]),
        ((0, 0.8 * y1, 0.8 * H), ["roura", "bricho"]),
        ((0, 0.8 * y1, 0.85 * H), ["roura", "bricho"]),
        ((-0.9 * w, 0, 0.35 * H), ["pruka", "granat", "bricho"]),
        ((0.9 * w, 0, 0.35 * H), ["lruka", "granat2", "bricho"]),
        ((0, 0.8 * y0, 0.76 * H), ["doplnovani", "bricho"]),
        ((0, 0.8 * y0, 0.74 * H), ["doplnovani2", "bricho"]),
    ])
    land = lod_collection(root, "LandContact", "LandContact")
    helper_object("dedushka_landcontact", land, points=[((0, 0, 0), [])])
    hit = lod_collection(root, "HitPoints", "HitPoints")
    helper_object("dedushka_hitpoints", hit, points=[
        ((0, 0, 0.93 * H), ["hlava", "bricho"]),
        ((0, 0, 0.7 * H), ["telo", "bricho"]),
        ((0.9 * w, 0, 0.5 * H), ["ruce", "bricho"]),
        ((-0.9 * w, 0, 0.5 * H), ["ruce", "bricho"]),
        ((foot, 0, 0.25 * H), ["nohy", "bricho"]),
        ((-foot, 0, 0.25 * H), ["nohy", "bricho"]),
    ])
    SUMMARY["depth_forward_m"] = round(-y0, 2)
    return root, manifest, None


def rigged(root):
    objects = [o for o in root.all_objects if o.type == "MESH"]
    visual = [o for o in objects if any(s.material for s in o.material_slots)]
    manifest = stage_textures(visual)
    SUMMARY["fullbright_faces"] = {o.name: flag_glow_faces(o) for o in visual}
    SUMMARY["objects"] = [o.name for o in objects]
    rest = root.get("ud_rest_json")
    lo = Vector((1e9, 1e9, 1e9))
    hi = -lo
    for o in visual:
        a, b = world_bounds(o)
        lo = Vector((min(lo.x, a.x), min(lo.y, a.y), min(lo.z, a.z)))
        hi = Vector((max(hi.x, b.x), max(hi.y, b.y), max(hi.z, b.z)))
    SUMMARY["bounds"] = {"min": [round(c, 3) for c in lo], "max": [round(c, 3) for c in hi]}
    SUMMARY["depth_forward_m"] = round(-lo.y, 2)
    SUMMARY["skin"] = "as authored"
    return root, manifest, (json.loads(rest) if rest else None)


root = bpy.data.collections.get(ROOT_COLLECTION)
if root is not None:
    SUMMARY["mode"] = "rigged"
    root, manifest, rest = rigged(root)
else:
    SUMMARY["mode"] = "wrap"
    root, manifest, rest = wrap()

out_p3d = os.path.join(OUT_DIR, "dedushka.p3d")
result = export_p3d(out_p3d, root)
WARN.extend(result.get("warnings") or [])
SUMMARY["p3d"] = out_p3d
SUMMARY["lods"] = result["lods"]
SUMMARY["textures"] = manifest
rest_path = os.path.join(OUT_DIR, "dedushka_rest.json")
if rest:
    with open(rest_path, "w", encoding="utf-8") as fh:
        json.dump(rest, fh, indent=1, sort_keys=True)
    SUMMARY["rest_json"] = rest_path
else:
    if os.path.isfile(rest_path):
        os.remove(rest_path)
    SUMMARY["rest_json"] = None
SUMMARY["height_m"] = SUMMARY["bounds"]["max"][2]
long_tex = [t for l in result["lods"] for t in l["textures"] if len(t) > 31]
if long_tex:
    raise SystemExit("texture names over 31 characters: %s" % sorted(set(long_tex)))
with open(os.path.join(OUT_DIR, "dedushka_export.json"), "w", encoding="utf-8") as fh:
    json.dump(SUMMARY, fh, indent=1, default=str)
print("[uddedushka] mode %s, wrote %s (%d bytes), %d textures, %d warnings"
      % (SUMMARY["mode"], out_p3d, result["bytes"], len(manifest), len(WARN)), flush=True)
