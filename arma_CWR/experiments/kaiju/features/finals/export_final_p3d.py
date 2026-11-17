"""Export a finished Dedushka candidate (A or D, iteration 3) to the SP3X MLOD the uddedushka addon packs.

A changed copy of experiments/kaiju/addon/export_dedushka_p3d.py (wrap mode only), for the two
final candidates. Headless, Blender 4.3; nothing is saved back.

  blender --background --factory-startup [<model.blend>] --python export_final_p3d.py -- <out_dir>
          --tag a|d [--obj <model.obj>] [--yaw 180] [--skin statue|bones] [--rest <dedushka_rest.json>]
          [--weights <weights.json>]

What differs from the pipeline's driver:

* Textures are NOT copied through Blender. Each material's image file is named in the manifest as it
  lies on disk (4096 and 2048 PNG, already powers of two) and the build script converts it straight to
  DXT1. The pipeline's driver resized everything to 1024.
* Texture names come from the sheet: dedushka_<tag>_<sheet>.png -> uddedushka\d<tag>_<sheet>.paa
  (25 characters at most here; the format's limit is 31).
* --obj: the model is read from an OBJ (candidate D was built in Blender 5.2 and saved an OBJ for 4.3).
* --skin bones keeps the stock bone groups AND the ded_* groups (the pipeline's wrap mode deletes the
  ded_* groups and then finds vertices without a bone, so it always made a statue of these models).
  With --weights the groups come from a JSON dumped in the model's own Blender version and are
  matched to the OBJ's vertices by position.
* Fullbright faces are split off the lit mesh, so no vertex is shared between a lit and a fullbright
  face, and both the face flag (FACE_NOLIGHT 0x1) and the point flag (POINT_NOLIGHT 0x10) are written.
* One hidden "mip anchor" triangle per sheet in LOD0 and LOD1 (TEXTURE-BUDGET.md, exporter point 3).
* Faces are stored sorted by material, one run per sheet (exporter point 2).
* LOD ladder: full mesh, 50 percent, 20 percent, and a last LOD of at most 480 triangles which is the
  only one that may cast the shadow (the engine refuses a shadow LOD over 512 faces, Shadow.cpp:496).
* With --rest the Memory LOD is placed from the joint file (aim point at the furnace, eye, soles,
  hands, one point per bone), each point in the bone it follows.

Writes into <out_dir>: dedushka.p3d, dedushka_export.json, and dedushka_rest.json (a copy of --rest).
"""
import json
import math
import os
import re
import sys

import bmesh
import bpy
from mathutils import Matrix, Vector
from mathutils.bvhtree import BVHTree
from mathutils.kdtree import KDTree

HERE = os.path.dirname(os.path.abspath(__file__))
ENGINE_ROOT = os.path.normpath(os.path.join(HERE, "..", "..", "..", ".."))
ADDON_DIR = os.path.join(ENGINE_ROOT, "apps", "tools", "BlenderAddon")

ADDON = "uddedushka"
ROOT_COLLECTION = "dedushka"
FACE_NOLIGHT = 0x1
POINT_NOLIGHT = 0x10
MASS_KG = 2.0e7
# (fraction of the height used as the LOD's resolution value, share of LOD0's triangles, hard cap)
LADDER = [(None, 1.0, None), (0.70, 0.50, None), (0.95, 0.20, None), (1.15, 0.03, 480)]
GLOW_NAME = re.compile(r"glow|furnace|fire|ember|emis|lava|reactor", re.I)
STOCK_BONES = (
    "pchodidlo", "lchodidlo", "pprsty", "lprsty", "lholen", "pholen", "pstehno", "lstehno",
    "pzadek", "lzadek", "bricho", "zebra", "hrudnik", "krk", "prameno", "lrameno", "hlava",
    "pbiceps", "lbiceps", "ploket", "lloket", "roura", "zbran", "pruka", "lruka",
)
DED_BONES = ("ded_jaw", "ded_rib_l", "ded_rib_r", "ded_tail1", "ded_tail2", "ded_tail3", "ded_hang1", "ded_hang2")
ALL_BONES = STOCK_BONES + DED_BONES

argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
if not argv:
    raise SystemExit("usage: blender -b [<model.blend>] --python export_final_p3d.py -- <out_dir> --tag a|d [options]")
OUT_DIR = os.path.abspath(argv[0])
OPT = {"tag": None, "obj": None, "yaw": 180.0, "skin": "statue", "rest": None, "weights": None}
i = 1
while i < len(argv):
    key = argv[i].lstrip("-")
    if key not in OPT:
        raise SystemExit("unknown option %s" % argv[i])
    OPT[key] = argv[i + 1]
    i += 2
OPT["yaw"] = float(OPT["yaw"])
if OPT["tag"] not in ("a", "d"):
    raise SystemExit("--tag a or --tag d is required")
os.makedirs(OUT_DIR, exist_ok=True)

sys.path.insert(0, ADDON_DIR)
import io_import_p3d as addon  # noqa: E402

addon.register()
from io_import_p3d.export_p3d import export_p3d  # noqa: E402

WARN = []
SUMMARY = {"blend": bpy.data.filepath, "obj": OPT["obj"], "warnings": WARN, "mode": "final-" + OPT["skin"]}


def warn(msg):
    WARN.append(msg)
    print("[final] warning:", msg, flush=True)


if OPT["obj"]:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.wm.obj_import(filepath=OPT["obj"], forward_axis="Y", up_axis="Z")


# ---------------------------------------------------------------------------
# Textures and glow
# ---------------------------------------------------------------------------
def material_image(mat):
    if not (mat.use_nodes and mat.node_tree):
        return None
    nodes = [n for n in mat.node_tree.nodes if n.type == "TEX_IMAGE" and n.image]
    return nodes[0].image if nodes else None


def is_glow(mat):
    if mat.get("p3d_fullbright") or GLOW_NAME.search(mat.name):
        return True
    if mat.use_nodes and mat.node_tree:
        for n in mat.node_tree.nodes:
            if n.type == "EMISSION":
                return True
            if n.type == "BSDF_PRINCIPLED" and "Emission Strength" in n.inputs:
                s = n.inputs["Emission Strength"]
                if s.is_linked or s.default_value > 0.0:
                    return True
    return False


def stage_textures(obj):
    manifest = []
    for slot in obj.material_slots:
        mat = slot.material
        image = material_image(mat)
        if image is None:
            raise SystemExit("material '%s' has no image texture" % mat.name)
        src = os.path.normpath(bpy.path.abspath(image.filepath, library=image.library))
        if not os.path.isfile(src):
            raise SystemExit("texture file missing: %s" % src)
        w, h = image.size[0], image.size[1]
        for side in (w, h):
            if side & (side - 1) or side > 4096 or side < 8:
                raise SystemExit("texture %s is %dx%d: sides must be powers of two, 4096 at most" % (src, w, h))
        stem = os.path.splitext(os.path.basename(src))[0].lower()
        stem = stem.replace("dedushka_%s_" % OPT["tag"], "d%s_" % OPT["tag"])
        tex = "%s\\%s.paa" % (ADDON, stem)
        if len(tex) > 31:
            raise SystemExit("texture path over 31 characters: %s" % tex)
        mat["p3d_texture"] = tex
        manifest.append({"png": src, "paa": stem + ".paa", "material": mat.name, "size": [w, h],
                         "fullbright": bool(is_glow(mat))})
    return manifest


# ---------------------------------------------------------------------------
# Mesh helpers
# ---------------------------------------------------------------------------
def world_bounds(obj):
    # From the vertices, not obj.bound_box: the box is cached and still holds the mesh as it was
    # before the turn (seen here: the summary's bounds were the unturned ones).
    pts = [obj.matrix_world @ v.co for v in obj.data.vertices]
    lo = Vector((min(p.x for p in pts), min(p.y for p in pts), min(p.z for p in pts)))
    hi = Vector((max(p.x for p in pts), max(p.y for p in pts), max(p.z for p in pts)))
    return lo, hi


def tri_count(mesh):
    return sum(len(p.vertices) - 2 for p in mesh.polygons)


def lod_collection(root, name, resolution, props=None):
    col = bpy.data.collections.new(name)
    root.children.link(col)
    col["p3d_resolution"] = resolution
    for k, v in (props or {}).items():
        col["p3d_prop_" + k] = v
    return col


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


def inside_point(obj, guesses):
    """A point inside the closed body: every one of 14 rays from it hits a face."""
    mesh = obj.data
    bvh = BVHTree.FromPolygons([v.co for v in mesh.vertices], [tuple(p.vertices) for p in mesh.polygons])
    dirs = [Vector(d).normalized() for d in ((1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1),
                                             (1, 1, 1), (-1, 1, 1), (1, -1, 1), (-1, -1, 1),
                                             (1, 1, -1), (-1, 1, -1), (1, -1, -1), (-1, -1, -1))]
    best, best_hits = None, -1
    for g in guesses:
        p = Vector(g)
        hits = sum(1 for d in dirs if bvh.ray_cast(p, d)[0] is not None)
        if hits > best_hits:
            best, best_hits = p, hits
        if hits == len(dirs):
            break
    return best, best_hits


def finish_lod(obj, glow_slots, anchor_at):
    """Split fullbright faces off, add mip anchors, sort faces by material, write the light flags."""
    mesh = obj.data
    bm = bmesh.new()
    bm.from_mesh(mesh)
    bm.faces.ensure_lookup_table()
    glow_faces = [f for f in bm.faces if f.material_index in glow_slots]
    if glow_faces and len(glow_faces) < len(bm.faces):
        bmesh.ops.split(bm, geom=glow_faces)
    anchors = 0
    if anchor_at is not None:
        uv = bm.loops.layers.uv.active
        dv = bm.verts.layers.deform.verify()
        root_group = obj.vertex_groups["bricho"].index
        bm.faces.ensure_lookup_table()
        first = {}
        for f in bm.faces:
            if f.material_index not in first:
                first[f.material_index] = f.loops[0][uv].uv.copy()
        for k, (slot, at) in enumerate(sorted(first.items())):
            base = anchor_at + Vector((0.05 * k, 0.0, 0.0))
            vs = [bm.verts.new(base), bm.verts.new(base + Vector((0.03, 0, 0))), bm.verts.new(base + Vector((0, 0, 0.03)))]
            for v in vs:
                v[dv][root_group] = 1.0
            f = bm.faces.new(vs)
            f.material_index = slot
            f.smooth = True
            for loop in f.loops:
                loop[uv].uv = at
            anchors += 1
    bm.faces.sort(key=lambda f: f.material_index)
    bm.faces.index_update()
    bm.to_mesh(mesh)
    bm.free()
    mesh.update()
    for name in ("p3d_face_flags", "p3d_point_flags"):
        stale = mesh.attributes.get(name)
        if stale is not None:
            mesh.attributes.remove(stale)
    ff = mesh.attributes.new("p3d_face_flags", "INT", "FACE")
    pf = mesh.attributes.new("p3d_point_flags", "INT", "POINT")
    lit = set()
    dark = set()
    n = 0
    for poly in mesh.polygons:
        if poly.material_index in glow_slots:
            ff.data[poly.index].value = FACE_NOLIGHT
            lit.update(poly.vertices)
            n += 1
        else:
            dark.update(poly.vertices)
    shared = lit & dark
    for vi in lit - dark:
        pf.data[vi].value = POINT_NOLIGHT
    runs = 1 + sum(1 for k in range(1, len(mesh.polygons))
                   if mesh.polygons[k].material_index != mesh.polygons[k - 1].material_index)
    return {"fullbright_faces": n, "shared_vertices": len(shared), "anchors": anchors, "material_runs": runs,
            "triangles": tri_count(mesh), "vertices": len(mesh.vertices)}


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


def from_file(p):
    """RTM file space (x left, y up, z back) -> Blender after the turn (x left, y back, z up)."""
    return Vector((p[0], p[2], p[1]))


# ---------------------------------------------------------------------------
# Build
# ---------------------------------------------------------------------------
meshes = [o for o in bpy.context.scene.objects if o.type == "MESH" and len(o.data.polygons) > 0]
if len(meshes) != 1:
    raise SystemExit("expected one creature mesh, found %s" % [o.name for o in meshes])
src = meshes[0]
SUMMARY["objects"] = [src.name]
manifest = stage_textures(src)
glow_slots = {k for k, s in enumerate(src.material_slots) if is_glow(s.material)}
if len(glow_slots) != 1:
    warn("expected one fullbright material, found %d" % len(glow_slots))

root = bpy.data.collections.new(ROOT_COLLECTION)
bpy.context.scene.collection.children.link(root)
lod0_col = lod_collection(root, "lod0", 1.0, {"lodnoshadow": "1"})
lod0 = src.copy()
lod0.data = src.data.copy()
lod0.name = "dedushka_lod0"
lod0_col.objects.link(lod0)
lod0.data.transform(src.matrix_world)
lod0.matrix_world = Matrix.Identity(4)
if abs(OPT["yaw"]) > 1e-6:
    lod0.data.transform(Matrix.Rotation(math.radians(OPT["yaw"]), 4, "Z"))
lod0.data.update()
SUMMARY["yaw_deg"] = OPT["yaw"]
lo, hi = world_bounds(lod0)
if abs(lo.z) > 0.05:
    warn("lowest point is at z = %.2f m; moved by %.2f m so the feet stand on z = 0" % (lo.z, -lo.z))
    lod0.data.transform(Matrix.Translation((0, 0, -lo.z)))
    lod0.data.update()
    lo, hi = world_bounds(lod0)
height = hi.z
SUMMARY["bounds"] = {"min": [round(c, 3) for c in lo], "max": [round(c, 3) for c in hi]}
SUMMARY["height_m"] = round(height, 3)
SUMMARY["depth_forward_m"] = round(-lo.y, 2)
reach = max(abs(lo.x), abs(hi.x), abs(lo.y), abs(hi.y))
if reach > 20:
    warn("a part reaches %.1f m from the origin in plan view (keep within 20, grid margin 25)" % reach)

rest = None
if OPT["rest"]:
    with open(OPT["rest"], "r", encoding="utf-8") as fh:
        rest = json.load(fh)

# Skin.
if OPT["skin"] == "bones" and OPT["weights"]:
    with open(OPT["weights"], "r", encoding="utf-8") as fh:
        dump = json.load(fh)
    # The dump is in the model's build space (before the turn): turn it the same way.
    rot = Matrix.Rotation(math.radians(OPT["yaw"]), 4, "Z")
    kd = KDTree(len(dump["co"]))
    for k, co in enumerate(dump["co"]):
        kd.insert(rot @ Vector(co), k)
    kd.balance()
    for vg in list(lod0.vertex_groups):
        lod0.vertex_groups.remove(vg)
    groups = {name: lod0.vertex_groups.new(name=name) for name in dump["groups"]}
    worst = 0.0
    for v in lod0.data.vertices:
        _co, k, dist = kd.find(v.co)
        worst = max(worst, dist)
        for name, w in dump["weights"][k].items():
            groups[name].add([v.index], w, "REPLACE")
    SUMMARY["weights_from"] = OPT["weights"]
    SUMMARY["weights_worst_match_m"] = round(worst, 5)
    if worst > 0.01:
        warn("weights matched by position, worst distance %.4f m" % worst)
if OPT["skin"] == "bones":
    for vg in [vg for vg in lod0.vertex_groups if vg.name.lower() not in ALL_BONES]:
        warn("vertex group '%s' is not a bone of SKELETON.md; dropped" % vg.name)
        lod0.vertex_groups.remove(vg)
    if "bricho" not in lod0.vertex_groups:
        lod0.vertex_groups.new(name="bricho")
    bone_idx = {vg.index for vg in lod0.vertex_groups}
    bare = [v.index for v in lod0.data.vertices if not any(g.group in bone_idx and g.weight > 0 for g in v.groups)]
    if bare:
        warn("%d vertices carry no bone; put on bricho" % len(bare))
        lod0.vertex_groups["bricho"].add(bare, 1.0, "REPLACE")
    over4 = sum(1 for v in lod0.data.vertices if sum(1 for g in v.groups if g.weight > 0) > 4)
    if over4:
        warn("%d vertices carry more than 4 bones" % over4)
    SUMMARY["skin"] = "bones: %s" % sorted(vg.name for vg in lod0.vertex_groups)
else:
    for vg in list(lod0.vertex_groups):
        lod0.vertex_groups.remove(vg)
    lod0.vertex_groups.new(name="bricho").add(range(len(lod0.data.vertices)), 1.0, "REPLACE")
    SUMMARY["skin"] = "statue (every vertex on bricho)"

# LODs.
tris0 = tri_count(lod0.data)
lods = [lod0]
for k in range(1, len(LADDER)):
    frac, share, cap = LADDER[k]
    target = tris0 * share
    if cap:
        target = min(target, cap)
    props = {"lodnoshadow": "1"} if k < len(LADDER) - 1 else None
    col = lod_collection(root, "lod%d" % k, round(frac * height, 2), props)
    obj = decimated_copy(lod0, "dedushka_lod%d" % k, col, min(1.0, target / tris0))
    if cap and tri_count(obj.data) > cap:      # the modifier overshoots a little: ask again, lower
        ratio = (target / tris0) * cap / tri_count(obj.data) * 0.97
        bpy.data.objects.remove(obj)
        obj = decimated_copy(lod0, "dedushka_lod%d" % k, col, ratio)
    if OPT["skin"] == "bones":
        idx = {vg.index for vg in obj.vertex_groups}
        bare = [v.index for v in obj.data.vertices if not any(g.group in idx and g.weight > 0 for g in v.groups)]
        if bare:
            obj.vertex_groups["bricho"].add(bare, 1.0, "REPLACE")
    lods.append(obj)

# Mip anchor: inside the pelvis.
guesses = []
if rest:
    r = from_file(rest["root"])
    guesses += [r, r + Vector((0, 0, 1)), r + Vector((0, 0, -1)), r + Vector((0, 1, 0)), r + Vector((0, -1, 0))]
cx, cy = (lo.x + hi.x) * 0.5, (lo.y + hi.y) * 0.5
for zf in (0.45, 0.5, 0.4, 0.55, 0.6):
    for dy in (0.0, 2.0, -2.0, 4.0):
        guesses.append(Vector((cx, cy + dy, zf * height)))
anchor, hits = inside_point(lod0, guesses)
SUMMARY["mip_anchor_at"] = [round(c, 2) for c in anchor]
SUMMARY["mip_anchor_rays_hit"] = "%d of 14" % hits
if hits < 14:
    warn("no fully enclosed point found for the mip anchors (best %d of 14 rays)" % hits)
SUMMARY["lod_detail"] = []
for k, obj in enumerate(lods):
    SUMMARY["lod_detail"].append(finish_lod(obj, glow_slots, anchor if k < 2 else None))
corners = len({(l.vertex_index, tuple(round(c, 4) for c in n.vector), tuple(round(c, 4) for c in uv.uv))
               for l, n, uv in zip(lod0.data.loops, lod0.data.corner_normals, lod0.data.uv_layers.active.data)})
SUMMARY["lod0_split_vertices_estimate"] = corners
if corners > 32767:
    warn("LOD0 has about %d vertices after normal and UV splits; the engine limit is 32767" % corners)

# Helper LODs. x: left (+) and right, y: front is -y, z: up.
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
if rest and OPT["skin"] == "bones":
    P = {k: from_file(v) for k, v in rest["pivots"].items()}
    P["bricho"] = from_file(rest["root"])
    Q = {k: from_file(v) for k, v in rest.get("points", {}).items()}
    furnace = Q.get("furnace", (P["hrudnik"] + P["krk"]) * 0.5)
    mouth = Q.get("mouth", P["hlava"])
    eye = (P["hlava"] + Q.get("head_top", P["hlava"]) + mouth) / 3.0
    hand_r, hand_l = Q.get("hand_r", P["pruka"]), Q.get("hand_l", P["lruka"])
    foot_l, foot_r = Q.get("foot_l", P["lchodidlo"]), Q.get("foot_r", P["pchodidlo"])
    back = P["hrudnik"] + Vector((0, 4.0, 3.0))
    pts = [
        (furnace, ["zamerny", "hrudnik"]),
        (P["bricho"] + Vector((0, 0, 6)), ["osa mireni", "bricho"]),
        (P["krk"], ["osa otaceni", "hrudnik"]),
        (eye, ["pilot", "hlava"]),
        (Vector((foot_l.x, foot_l.y, 0.05)), ["stopaL", "lchodidlo"]),
        (Vector((foot_r.x, foot_r.y, 0.05)), ["stopaP", "pchodidlo"]),
        (hand_r, ["zbran"]), (hand_r + Vector((0, -2, 0)), ["zbran"]),
        (back, ["roura"]), (back + Vector((0, 0, 2)), ["roura"]),
        (hand_r, ["pruka", "granat"]), (hand_l, ["lruka", "granat2"]),
        (furnace, ["ded_furnace", "hrudnik"]), (furnace + Vector((0, -1, 0)), ["ded_furnace_dir", "hrudnik"]),
        (mouth, ["ded_mouth", "hlava"]),
        (furnace + Vector((0, 0, -1)), ["doplnovani", "hrudnik"]), (furnace + Vector((0, 0, -2)), ["doplnovani2", "hrudnik"]),
    ]
    for bone in ("bricho", "zebra", "hrudnik", "krk", "hlava", "lstehno", "pstehno", "lholen", "pholen",
                 "lchodidlo", "pchodidlo", "lbiceps", "pbiceps", "lloket", "ploket"):
        pts.append((P[bone], [bone]))
    helper_object("dedushka_memory", mem, points=pts)
    SUMMARY["memory"] = "from the joint file"
else:
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
    SUMMARY["memory"] = "from the bounding box (statue)"
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
long_tex = [t for l in result["lods"] for t in l["textures"] if len(t) > 31]
if long_tex:
    raise SystemExit("texture names over 31 characters: %s" % sorted(set(long_tex)))
with open(os.path.join(OUT_DIR, "dedushka_export.json"), "w", encoding="utf-8") as fh:
    json.dump(SUMMARY, fh, indent=1, default=str)
print("[final] %s, wrote %s (%d bytes), %d textures, %d warnings"
      % (SUMMARY["mode"], out_p3d, result["bytes"], len(manifest), len(WARN)), flush=True)
