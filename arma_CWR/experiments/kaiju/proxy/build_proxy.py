"""Builds the Dedushka pipeline proxy: a blocky 40 m upright biped, from nothing.

Headless, Blender 4.3:

  blender --background --factory-startup --python build_proxy.py -- <out_dir> [--raw]

Writes into <out_dir>:
  dedushka_proxy.blend   every LOD a Man needs, as LOD collections under "dedushka"
  dedushka_rest.json     joint pivots (ud-rtm-rest/1, RTM file space) for the RTM tools
  ded_*.png              the proxy's textures (the build script converts them to PAA)
  dedushka_proxy_raw.blend   with --raw: only the LOD0 mesh, no LODs, no groups,
                         to exercise the wrap mode of export_dedushka_p3d.py

The proxy is not art. It exists to prove the road from a .blend to in-game
stills and to measure what the engine does with a 40 m Man:

* Rest space is the game's standing space: origin on the ground between the
  feet, +Z up, facing Blender -Y, the creature's left at +X, 1 unit = 1 m.
  With that choice an identity RTM is a correct standing pose and the model's
  bounding sphere sits where the body is (see NOTES.md, "Skeleton").
* 23 boxes rigidly skinned to the stock man bone names. Every joint pivot is
  written to dedushka_rest.json, so clips are made for THIS skeleton
  (gen_dedushka_rtm.py), not borrowed from the stock man.
* The head of each visual LOD carries a different flat colour (LOD0 bone
  white, LOD1 red, LOD2 green, LOD3 blue), so a screenshot shows which LOD
  the engine picked at that distance.
* A fullbright orange patch on the chest (face flag 0x1, FACE_NOLIGHT) in
  every visual LOD tests unlit faces at night.
"""
import json
import os
import sys

import bpy

argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
if not argv:
    raise SystemExit("usage: blender -b --factory-startup --python build_proxy.py -- <out_dir> [--raw]")
OUT_DIR = os.path.abspath(argv[0])
RAW = "--raw" in argv
os.makedirs(OUT_DIR, exist_ok=True)

ADDON = "uddedushka"
FACE_NOLIGHT = 0x1          # Core/Data3D.h: fullbright ("shining") face
MASS_KG = 2.0e7             # 20 000 t: a tank cannot push it (survey section 4)

# LOD resolutions for a giant. The engine picks a LOD from distance alone
# (SceneDraw.cpp LevelFromDistance2): the LOD is the last one whose resolution
# is at most distance * k, where k was measured at 0.09 to 0.14 per metre at
# the normal field of view in the photo runs (it adapts to frame time, and is
# up to 16 times smaller on a fast frame). A first try with 4 / 10 / 20 was on
# its coarsest LOD by 230 m. Every value under 900 is also clamped to twice
# the bounding sphere (ShapeLOD.cpp:1166-1172), about 49 here, so the ladder
# is tied to the height: 0.45, 0.70 and 0.95 times 40 m.
LOD_RES = [1.0, 18.0, 28.0, 38.0]
# How far the chest patch stands proud of the chest, per LOD. A thin overlay
# z-fights with the surface under it at range: 6 cm was lost at 2 km and
# 2.5 km (photo runs 1 and 2), 15 cm held at 2 km, 40 cm at 2.9 km.
PATCH_PROUD = [0.25, 0.25, 0.40, 0.80]
MARKER = ["ded_m0", "ded_m1", "ded_m2", "ded_m3"]

# ---------------------------------------------------------------------------
# Body parts: (bone, (x0, x1), (y0, y1), (z0, z1), texture, lods)
# Left side only for limbs; mirror() makes the right ("p") side.
# ---------------------------------------------------------------------------
ALL = (0, 1, 2, 3)
CENTER = [
    ("bricho",  (-7.5, 7.5), (-4.2, 4.2), (22.5, 26.5), "ded_body", (0, 1)),
    ("zebra",   (-8.2, 8.2), (-4.6, 4.6), (26.5, 30.0), "ded_body", (0, 1)),
    ("hrudnik", (-9.0, 9.0), (-5.0, 5.0), (30.0, 34.5), "ded_body", (0, 1)),
    ("krk",     (-2.2, 2.2), (-2.5, 2.0), (34.5, 36.0), "ded_limb", (0, 1)),
    ("hlava",   (-3.2, 3.2), (-4.2, 2.6), (36.0, 40.0), "MARKER",   ALL),
    ("hlava",   (-1.8, 1.8), (-6.0, -4.2), (36.5, 38.5), "ded_limb", (0, 1)),   # snout: shows the facing
    # coarse torso for LOD2 and LOD3 (one box each for belly and chest, then one for all)
    ("bricho",  (-8.2, 8.2), (-4.6, 4.6), (18.5, 28.0), "ded_body", (2,)),
    ("hrudnik", (-9.0, 9.0), (-5.0, 5.0), (28.0, 36.0), "ded_body", (2,)),
    ("zebra",   (-9.0, 9.0), (-5.0, 5.0), (18.5, 36.0), "ded_body", (3,)),
]
LEFT = [
    ("lzadek",    (0.0, 8.5),   (-3.8, 3.8), (18.5, 22.5), "ded_body", (0, 1)),
    ("lstehno",   (2.2, 7.8),   (-2.6, 2.6), (10.5, 19.5), "ded_limb", (0, 1, 2)),
    ("lholen",    (3.0, 7.0),   (-2.0, 2.0), (2.5, 10.5),  "ded_limb", (0, 1, 2)),
    ("lchodidlo", (2.5, 7.5),   (-2.5, 3.5), (0.0, 2.5),   "ded_foot", (0,)),
    ("lprsty",    (2.5, 7.5),   (-6.0, -2.5), (0.0, 2.0),  "ded_foot", (0,)),
    ("lchodidlo", (2.5, 7.5),   (-6.0, 3.5), (0.0, 2.5),   "ded_foot", (1, 2)),   # foot and toes as one
    ("lstehno",   (2.5, 7.5),   (-2.5, 2.5), (0.0, 18.5),  "ded_limb", (3,)),     # the whole leg
    ("lrameno",   (9.0, 12.5),  (-2.5, 2.5), (31.0, 34.5), "ded_body", (0, 1)),
    ("lbiceps",   (9.8, 13.2),  (-2.0, 2.0), (23.5, 31.0), "ded_limb", (0, 1, 2)),
    ("lloket",    (10.0, 13.0), (-1.8, 1.8), (16.0, 23.5), "ded_limb", (0, 1, 2)),
    ("lruka",     (10.0, 13.0), (-2.2, 1.8), (12.5, 16.0), "ded_foot", (0, 1)),
    ("lbiceps",   (9.5, 13.0),  (-2.0, 2.0), (13.0, 34.5), "ded_limb", (3,)),     # the whole arm
]
# The fullbright chest patch, 5 cm proud of the chest front, facing -Y.
PATCH = ("hrudnik", (-3.5, 3.5), -5.0, (30.6, 33.9))

# Joint pivots in Blender coordinates (left side; mirrored for "p").
PIVOT_CENTER = {
    "zebra": (0, 0, 26.5), "hrudnik": (0, 0, 30.0), "krk": (0, 0, 34.5), "hlava": (0, 0, 36.0),
    "roura": (0, 4.0, 32.0),
}
PIVOT_LEFT = {
    "lzadek": (2.5, 0, 20.5), "lstehno": (5.0, 0, 19.5), "lholen": (5.0, 0, 10.5),
    "lchodidlo": (5.0, 0, 2.5), "lprsty": (5.0, -2.5, 0.8),
    "lrameno": (9.0, 0, 33.0), "lbiceps": (11.5, 0, 31.0), "lloket": (11.5, 0, 23.5), "lruka": (11.5, 0, 16.0),
}
# Parents (the stock man hierarchy, as rtm_rig.STOCK_PARENTS).
PARENTS = {
    "bricho": None, "zebra": "bricho", "hrudnik": "zebra", "krk": "hrudnik", "hlava": "krk",
    "lrameno": "hrudnik", "prameno": "hrudnik", "lbiceps": "lrameno", "pbiceps": "prameno",
    "lloket": "lbiceps", "ploket": "pbiceps", "lruka": "lloket", "pruka": "ploket",
    "lzadek": "bricho", "pzadek": "bricho", "lstehno": "lzadek", "pstehno": "pzadek",
    "lholen": "lstehno", "pholen": "pstehno", "lchodidlo": "lholen", "pchodidlo": "pholen",
    "lprsty": "lchodidlo", "pprsty": "pchodidlo", "zbran": "pruka", "roura": "hrudnik",
}


def mirror_name(name):
    return "p" + name[1:]


def mirror_part(part):
    bone, (x0, x1), ys, zs, tex, lods = part
    return (mirror_name(bone), (-x1, -x0), ys, zs, tex, lods)


PARTS = CENTER + LEFT + [mirror_part(p) for p in LEFT]

PIVOTS = dict(PIVOT_CENTER)
for name, (x, y, z) in PIVOT_LEFT.items():
    PIVOTS[name] = (x, y, z)
    PIVOTS[mirror_name(name)] = (-x, y, z)
PIVOTS["zbran"] = (-11.5, -2.0, 14.0)   # right hand

# ---------------------------------------------------------------------------
# Textures: flat or lightly mottled squares, made here from nothing.
# ---------------------------------------------------------------------------
TEXTURES = {
    # name: (size, base colour, mottle)
    "ded_body": (128, (0.36, 0.30, 0.24), 0.10),
    "ded_limb": (128, (0.24, 0.20, 0.17), 0.08),
    "ded_foot": (128, (0.14, 0.13, 0.13), 0.05),
    "ded_glow": (32, (1.00, 0.45, 0.04), 0.0),
    "ded_m0": (32, (0.86, 0.83, 0.74), 0.0),   # LOD0 head: bone white
    "ded_m1": (32, (0.85, 0.08, 0.08), 0.0),   # LOD1 head: red
    "ded_m2": (32, (0.10, 0.75, 0.12), 0.0),   # LOD2 head: green
    "ded_m3": (32, (0.10, 0.20, 0.90), 0.0),   # LOD3 head: blue
}


def make_texture(name):
    size, (r, g, b), mottle = TEXTURES[name]
    img = bpy.data.images.new(name, size, size, alpha=False)
    px = [0.0] * (size * size * 4)
    seed = sum(ord(c) for c in name) * 7919
    for y in range(size):
        for x in range(size):
            # A cheap repeatable hash: blotches 8 px wide, no random module state.
            h = ((x // 8) * 73856093 ^ (y // 8) * 19349663 ^ seed) & 0xFFFF
            k = 1.0 + mottle * ((h / 65535.0) * 2.0 - 1.0)
            i = (y * size + x) * 4
            px[i] = min(1.0, r * k)
            px[i + 1] = min(1.0, g * k)
            px[i + 2] = min(1.0, b * k)
            px[i + 3] = 1.0
    img.pixels = px
    img.filepath_raw = os.path.join(OUT_DIR, name + ".png")
    img.file_format = "PNG"
    img.save()
    return img


MATERIALS = {}


def material(name):
    mat = MATERIALS.get(name)
    if mat is None:
        img = make_texture(name)
        mat = bpy.data.materials.new(name)
        mat.use_nodes = True
        bsdf = mat.node_tree.nodes.get("Principled BSDF")
        tex = mat.node_tree.nodes.new("ShaderNodeTexImage")
        tex.image = img
        mat.node_tree.links.new(bsdf.inputs["Base Color"], tex.outputs["Color"])
        mat.use_backface_culling = True
        # The pbo's file name is its path prefix, so the face texture names it.
        mat["p3d_texture"] = "%s\\%s.paa" % (ADDON, name)
        if name == "ded_glow":
            mat["p3d_fullbright"] = 1
        MATERIALS[name] = mat
    return mat


# ---------------------------------------------------------------------------
# Mesh helpers
# ---------------------------------------------------------------------------
BOX_FACES = [  # outward (counter-clockwise seen from outside)
    (0, 3, 2, 1), (4, 5, 6, 7), (0, 1, 5, 4), (1, 2, 6, 5), (2, 3, 7, 6), (3, 0, 4, 7),
]
QUAD_UV = [(0.05, 0.05), (0.95, 0.05), (0.95, 0.95), (0.05, 0.95)]


def box_verts(xs, ys, zs):
    (x0, x1), (y0, y1), (z0, z1) = xs, ys, zs
    return [(x0, y0, z0), (x1, y0, z0), (x1, y1, z0), (x0, y1, z0),
            (x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1)]


class MeshBuilder:
    def __init__(self, name):
        self.name = name
        self.verts = []
        self.faces = []
        self.face_mat = []
        self.face_flags = []
        self.groups = {}      # group name -> [vertex index]
        self.mats = []        # material names in slot order

    def _slot(self, tex):
        if tex not in self.mats:
            self.mats.append(tex)
        return self.mats.index(tex)

    def add_box(self, group, xs, ys, zs, tex=None, extra_groups=()):
        base = len(self.verts)
        self.verts += box_verts(xs, ys, zs)
        for f in BOX_FACES:
            self.faces.append(tuple(base + i for i in f))
            self.face_mat.append(self._slot(tex) if tex else 0)
            self.face_flags.append(0)
        for g in (group,) + tuple(extra_groups):
            self.groups.setdefault(g, []).extend(range(base, base + 8))

    def add_quad(self, group, corners, tex, flags=0):
        base = len(self.verts)
        self.verts += corners
        self.faces.append((base, base + 1, base + 2, base + 3))
        self.face_mat.append(self._slot(tex))
        self.face_flags.append(flags)
        self.groups.setdefault(group, []).extend(range(base, base + 4))

    def add_point(self, co, groups):
        self.verts.append(tuple(co))
        for g in groups:
            self.groups.setdefault(g, []).append(len(self.verts) - 1)

    def build(self, collection, with_groups=True):
        mesh = bpy.data.meshes.new(self.name)
        mesh.from_pydata(self.verts, [], self.faces)
        mesh.update()
        obj = bpy.data.objects.new(self.name, mesh)
        collection.objects.link(obj)
        for tex in self.mats:
            mesh.materials.append(material(tex))
        if self.faces:
            for poly, mi in zip(mesh.polygons, self.face_mat):
                poly.material_index = mi
                poly.use_smooth = False
            uv = mesh.uv_layers.new(name="UVMap")
            for poly in mesh.polygons:
                for k, li in enumerate(poly.loop_indices):
                    uv.data[li].uv = QUAD_UV[k % 4]
            if any(self.face_flags):
                attr = mesh.attributes.new("p3d_face_flags", "INT", "FACE")
                for i, fl in enumerate(self.face_flags):
                    attr.data[i].value = fl
        if with_groups:
            for g, idx in self.groups.items():
                obj.vertex_groups.new(name=g).add(idx, 1.0, "REPLACE")
        return obj


def lod_collection(root, name, resolution, props=None):
    col = bpy.data.collections.new(name)
    root.children.link(col)
    col["p3d_resolution"] = resolution
    for k, v in (props or {}).items():
        col["p3d_prop_" + k] = v
    return col


def visual_lod(root, index):
    col = lod_collection(root, "lod%d" % index, LOD_RES[index],
                         {"lodnoshadow": "1"} if index < 2 else None)
    mb = MeshBuilder("dedushka_lod%d" % index)
    for bone, xs, ys, zs, tex, lods in PARTS:
        if index in lods:
            mb.add_box(bone, xs, ys, zs, MARKER[index] if tex == "MARKER" else tex)
    bone, (x0, x1), y, (z0, z1) = PATCH
    y -= PATCH_PROUD[index]
    # Counter-clockwise seen from the front (-Y).
    mb.add_quad("hrudnik" if index < 3 else "zebra", [(x0, y, z0), (x1, y, z0), (x1, y, z1), (x0, y, z1)],
                "ded_glow", FACE_NOLIGHT)
    return mb.build(col)


def reset_scene():
    for block in (bpy.data.objects, bpy.data.meshes, bpy.data.materials, bpy.data.images, bpy.data.collections):
        for item in list(block):
            block.remove(item)


def to_file_space(p):
    """Blender (x, y, z) -> game/RTM file space (x, y up, z)."""
    return [round(p[0], 5), round(p[2], 5), round(p[1], 5)]


def build_full():
    reset_scene()
    scene = bpy.context.scene
    root = bpy.data.collections.new("dedushka")
    scene.collection.children.link(root)

    tris = []
    for i in range(4):
        obj = visual_lod(root, i)
        tris.append(sum(len(p.vertices) - 2 for p in obj.data.polygons))

    # Geometry: eight closed boxes like the stock man's (head, trunk, thighs,
    # shins, feet), each following its bone. No arms, as in the stock model.
    geo = lod_collection(root, "Geometry", "Geometry", {"autocenter": "0"})
    geo["p3d_mass"] = MASS_KG
    mb = MeshBuilder("dedushka_geometry")
    mb.add_box("hlava", (-3.2, 3.2), (-4.2, 2.6), (36.0, 40.0))
    mb.add_box("zebra", (-9.0, 9.0), (-5.0, 5.0), (19.5, 34.5))
    for side, sx in (("l", 1), ("p", -1)):
        def xs(a, b):
            return (min(a * sx, b * sx), max(a * sx, b * sx))
        mb.add_box(side + "stehno", xs(2.2, 7.8), (-2.6, 2.6), (10.5, 19.5))
        mb.add_box(side + "holen", xs(3.0, 7.0), (-2.0, 2.0), (2.5, 10.5))
        mb.add_box(side + "chodidlo", xs(2.5, 7.5), (-6.0, 3.5), (0.0, 2.5))
    mb.build(geo)

    # FireGeometry: the same plus the arms (14 boxes in the stock model).
    fire = lod_collection(root, "FireGeometry", "FireGeometry")
    mb = MeshBuilder("dedushka_firegeometry")
    mb.add_box("hlava", (-3.2, 3.2), (-4.2, 2.6), (36.0, 40.0))
    mb.add_box("zebra", (-9.0, 9.0), (-5.0, 5.0), (19.5, 34.5))
    for side, sx in (("l", 1), ("p", -1)):
        def xs(a, b):
            return (min(a * sx, b * sx), max(a * sx, b * sx))
        mb.add_box(side + "stehno", xs(2.2, 7.8), (-2.6, 2.6), (10.5, 19.5))
        mb.add_box(side + "holen", xs(3.0, 7.0), (-2.0, 2.0), (2.5, 10.5))
        mb.add_box(side + "chodidlo", xs(2.5, 7.5), (-6.0, 3.5), (0.0, 2.5))
        mb.add_box(side + "biceps", xs(9.8, 13.2), (-2.0, 2.0), (23.5, 31.0))
        mb.add_box(side + "loket", xs(10.0, 13.0), (-1.8, 1.8), (16.0, 23.5))
        mb.add_box(side + "ruka", xs(10.0, 13.0), (-2.2, 1.8), (12.5, 16.0))
    mb.build(fire)

    # Memory: the point names ManType::InitShape reads, placed for this body.
    # A point follows every bone selection it is in.
    mem = lod_collection(root, "Memory", "Memory")
    mb = MeshBuilder("dedushka_memory")
    mb.add_point((0, -5.3, 32.0), ["zamerny", "hrudnik"])         # aim and camera point: the chest
    mb.add_point((0, 0, 30.0), ["osa mireni", "bricho"])           # aiming axis
    mb.add_point((0, 0, 34.5), ["osa otaceni", "hrudnik"])         # head axis
    mb.add_point((0, -4.4, 38.6), ["pilot", "hlava"])              # eye
    mb.add_point((5.0, -1.5, 0.05), ["stopaL", "lchodidlo"])       # footprints
    mb.add_point((-5.0, -1.5, 0.05), ["stopaP", "pchodidlo"])
    mb.add_point((5.0, 0, 2.5), ["lchodidlo"])                     # ankles (lean code)
    mb.add_point((-5.0, 0, 2.5), ["pchodidlo"])
    mb.add_point((0, 0, 24.5), ["bricho"])
    mb.add_point((0, 0, 28.0), ["zebra"])
    mb.add_point((0, 0, 32.0), ["hrudnik"])
    mb.add_point((0, 0, 38.0), ["hlava"])
    mb.add_point((-11.5, -2.0, 14.0), ["zbran"])                   # weapon bone: right hand
    mb.add_point((-11.5, -4.0, 14.0), ["zbran"])
    mb.add_point((0, 4.0, 32.0), ["roura"])                        # launcher bone: the back
    mb.add_point((0, 4.0, 34.0), ["roura"])
    mb.add_point((-11.5, 0, 14.0), ["pruka", "granat"])
    mb.add_point((11.5, 0, 14.0), ["lruka", "granat2"])
    mb.add_point((0, -5.3, 31.0), ["doplnovani", "hrudnik"])
    mb.add_point((0, -5.3, 30.0), ["doplnovani2", "hrudnik"])
    mb.build(mem)

    # LandContact: one point at the origin, on the ground between the feet.
    land = lod_collection(root, "LandContact", "LandContact")
    mb = MeshBuilder("dedushka_landcontact")
    mb.add_point((0, 0, 0), [])
    mb.build(land)

    # HitPoints: the four zones a Man registers (hlava, telo, ruce, nohy),
    # each point also in its bone so the zone follows the animation.
    hit = lod_collection(root, "HitPoints", "HitPoints")
    mb = MeshBuilder("dedushka_hitpoints")
    mb.add_point((0, 0, 38.0), ["hlava"])
    mb.add_point((0, 0, 28.0), ["zebra", "telo"])
    for side, sx in (("l", 1), ("p", -1)):
        mb.add_point((11.5 * sx, 0, 27.0), [side + "biceps", "ruce"])
        mb.add_point((11.5 * sx, 0, 19.5), [side + "loket", "ruce"])
        mb.add_point((5.0 * sx, 0, 15.0), [side + "stehno", "nohy"])
        mb.add_point((5.0 * sx, 0, 6.5), [side + "holen", "nohy"])
        mb.add_point((5.0 * sx, 0, 1.2), [side + "chodidlo", "nohy"])
    mb.build(hit)

    # Joint pivots for the RTM tools, in RTM file space.
    pivots = {b: to_file_space(p) for b, p in PIVOTS.items()}
    rest = {
        "format": "ud-rtm-rest/1",
        "space": "RTM file space: X (creature's left), Y up, Z (back); the rest pose IS the standing pose",
        "note": "Dedushka proxy joints, written by build_proxy.py; not derived from any stock file",
        "pivots": pivots,
        "parents": PARENTS,
        "root": to_file_space((0, 0, 20.5)),
        "height_m": 40.0,
    }
    with open(os.path.join(OUT_DIR, "dedushka_rest.json"), "w", encoding="utf-8") as fh:
        json.dump(rest, fh, indent=1, sort_keys=True)
    root["ud_rest_json"] = json.dumps(rest)

    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUT_DIR, "dedushka_proxy.blend"))
    print("[proxy] triangles per visual LOD:", tris, flush=True)
    return tris


def build_raw():
    """Only the LOD0 mesh, the way an unrigged art candidate arrives."""
    reset_scene()
    MATERIALS.clear()
    scene = bpy.context.scene
    mb = MeshBuilder("dedushka_candidate")
    for bone, xs, ys, zs, tex, lods in PARTS:
        if 0 in lods:
            mb.add_box(bone, xs, ys, zs, MARKER[0] if tex == "MARKER" else tex)
    bone, (x0, x1), y, (z0, z1) = PATCH
    y -= PATCH_PROUD[0]
    mb.add_quad("hrudnik", [(x0, y, z0), (x1, y, z0), (x1, y, z1), (x0, y, z1)], "ded_glow", 0)
    obj = mb.build(scene.collection, with_groups=False)
    for mat in bpy.data.materials:      # a candidate knows nothing about p3d names
        for key in ("p3d_texture", "p3d_fullbright"):
            if key in mat:
                del mat[key]
    bpy.data.materials["ded_glow"].name = "furnace_glow"
    attr = obj.data.attributes.get("p3d_face_flags")
    if attr is not None:
        obj.data.attributes.remove(attr)
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUT_DIR, "dedushka_proxy_raw.blend"))
    print("[proxy] raw candidate saved", flush=True)


build_full()
if RAW:
    build_raw()
