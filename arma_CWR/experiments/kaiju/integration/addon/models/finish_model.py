"""The finishing exporter: a round 3 Dedushka model in, a finished game model out.

  blender --background --factory-startup [<model.blend>] --python finish_model.py -- <out_dir> --tag a|d
          --rest <dedushka_rest.json> [--obj <model.obj> --weights <weights.json>] [--yaw 180]
          [--far hand|auto|none] [--texdir <folder of the sheets as PNG, any size>] [--blend <save as>]

Blender 4.3, headless. Nothing is saved back into the source folder. Started from the finals'
export_final_p3d.py; what the finishing round adds (kaiju-notes\\finishing\\models\\NOTES.md):

* WEIGHTS. The limbs of both models are closed tubes pushed into the trunk and weighted to one bone
  up to their very end, so a swinging limb carried its root out of the hip. The root of each tube now
  blends into the bone it sits in; the spine tube bends over its length; the ball of the foot bends.
  (creature_cfg.WEIGHTS)
* COLLISION AND HIT SHAPES. One convex tapered box per bone (creature_cfg.HULLS), each in that bone's
  selection, so the shape follows the animation. FireGeometry reaches the ground and has the furnace
  as its own piece and named regions (ded_hit_furnace, ded_hit_head, ded_hit_body, ded_hit_arms,
  ded_hit_legs). Geometry (collision) has nothing under 0.17 of his height.
* FAR LODS made for their distance (far_lod.py): each body part rebuilt as one closed skin, holes
  shut, small parts dropped, claws kept as blades, the furnace a flat bright face, and its own small
  sheets baked from LOD0.
* MEMORY POINTS: the full set, each placed from the joint file or measured on the mesh, each in the
  bone it follows, and listed with its distance to the surface in the summary.

Writes into <out_dir>: dedushka.p3d, dedushka_export.json, dedushka_rest.json (the joint file with
this round's clip settings and extra points), far_<n>_hide.png / far_<n>_glow.png.

Blender space after the turn: x = his left, y = back, z = up. File space: (x, z, y).
"""
import json
import math
import os
import re
import sys

import bmesh
import bpy
import numpy as np
from mathutils import Matrix, Vector
from mathutils.bvhtree import BVHTree
from mathutils.kdtree import KDTree

HERE = os.path.dirname(os.path.abspath(__file__))
ENGINE_ROOT = os.path.normpath(os.path.join(HERE, "..", "..", "..", "..", ".."))
ADDON_DIR = os.path.join(ENGINE_ROOT, "apps", "tools", "BlenderAddon")
sys.path.insert(0, HERE)
import creature_cfg as CFG  # noqa: E402

ADDON = "uddedushka"
ROOT_COLLECTION = "dedushka"
FACE_NOLIGHT = 0x1
POINT_NOLIGHT = 0x10
MASS_KG = 2.0e7
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
    raise SystemExit("usage: blender -b [<model.blend>] --python finish_model.py -- <out_dir> --tag a|d --rest <json> [options]")
OUT_DIR = os.path.abspath(argv[0])
OPT = {"tag": None, "obj": None, "yaw": 180.0, "rest": None, "weights": None, "far": "hand", "texdir": None,
       "blend": None, "weightfix": "1", "glowtex": None, "wounds": "1"}
i = 1
while i < len(argv):
    key = argv[i].lstrip("-")
    if key not in OPT:
        raise SystemExit("unknown option %s" % argv[i])
    OPT[key] = argv[i + 1]
    i += 2
OPT["yaw"] = float(OPT["yaw"])
TAG = OPT["tag"]
if TAG not in ("a", "d"):
    raise SystemExit("--tag a or --tag d is required")
if not OPT["rest"]:
    raise SystemExit("--rest <dedushka_rest.json> is required")
os.makedirs(OUT_DIR, exist_ok=True)

sys.path.insert(0, ADDON_DIR)
import io_import_p3d as addon  # noqa: E402

addon.register()
from io_import_p3d.export_p3d import export_p3d  # noqa: E402

WARN = []
SUMMARY = {"blend": bpy.data.filepath, "obj": OPT["obj"], "warnings": WARN, "mode": "finished"}
LOG = open(os.path.join(OUT_DIR, "finish.log"), "w", encoding="utf-8")


def say(msg):
    print("[finish] " + msg, flush=True)
    LOG.write(msg + "\n")
    LOG.flush()


def warn(msg):
    WARN.append(msg)
    say("warning: " + msg)


if OPT["obj"]:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.wm.obj_import(filepath=OPT["obj"], forward_axis="Y", up_axis="Z")

with open(OPT["rest"], "r", encoding="utf-8") as fh:
    REST = json.load(fh)


def from_file(p):
    return Vector((p[0], p[2], p[1]))


def to_file(v):
    return [round(float(v[0]), 3), round(float(v[2]), 3), round(float(v[1]), 3)]


PIV = {k: from_file(v) for k, v in REST["pivots"].items()}
PIV["bricho"] = from_file(REST["root"])
PTS = {k: from_file(v) for k, v in REST.get("points", {}).items()}


# ---------------------------------------------------------------------------------------------------
# Textures and glow
# ---------------------------------------------------------------------------------------------------
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
        stem = os.path.splitext(os.path.basename(src))[0].lower()
        stem = stem.replace("dedushka_%s_" % TAG, "d%s_" % TAG)
        glow = bool(is_glow(mat))
        if glow and OPT["glowtex"]:
            src = os.path.abspath(OPT["glowtex"])       # this round's reworked furnace sheet
        tex = "%s\\%s.paa" % (ADDON, stem)
        if len(tex) > 31:
            raise SystemExit("texture path over 31 characters: %s" % tex)
        mat["p3d_texture"] = tex
        mat["ud_stem"] = stem
        manifest.append({"png": src, "paa": stem + ".paa", "material": mat.name, "size": [w, h], "fullbright": glow})
    return manifest


def world_bounds(obj):
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


def inside_point(obj, guesses):
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
    # The FACE flag alone marks a fullbright face. The finals' exporter also set the POINT flag on its
    # points; the loader makes material 220 of a shining face on shining points, the stock material
    # table has no 220 and lit the furnace like hide: it was never self-lit at night (found by the
    # engine agent, finishing\engine\FOR-MODELS.md item 2). No point flag is written any more.
    runs = 1 + sum(1 for k in range(1, len(mesh.polygons))
                   if mesh.polygons[k].material_index != mesh.polygons[k - 1].material_index)
    return {"fullbright_faces": n, "shared_vertices": len(shared), "anchors": anchors, "material_runs": runs,
            "triangles": tri_count(mesh), "vertices": len(mesh.vertices)}


# ---------------------------------------------------------------------------------------------------
# Weights as arrays
# ---------------------------------------------------------------------------------------------------
class Skin:
    def __init__(self, obj):
        self.obj = obj
        mesh = obj.data
        self.n = len(mesh.vertices)
        self.P = np.array([v.co[:] for v in mesh.vertices], dtype=np.float64)
        self.names = [vg.name.lower() for vg in obj.vertex_groups]
        self.col = {n: k for k, n in enumerate(self.names)}
        self.W = np.zeros((self.n, len(self.names)))
        for v in mesh.vertices:
            for g in v.groups:
                self.W[v.index, g.group] = g.weight
        # connected pieces
        parent = np.arange(self.n)

        def find(a):
            while parent[a] != a:
                parent[a] = parent[parent[a]]
                a = parent[a]
            return a
        for e in mesh.edges:
            a, b = find(e.vertices[0]), find(e.vertices[1])
            if a != b:
                parent[b] = a
        self.piece = np.array([find(k) for k in range(self.n)])

    def bone(self, name):
        if name not in self.col:
            self.col[name] = len(self.names)
            self.names.append(name)
            self.W = np.concatenate([self.W, np.zeros((self.n, 1))], axis=1)
        return self.col[name]

    def main(self):
        return np.argmax(self.W, axis=1)

    def pieces_with(self, name, share=0.02):
        """Ids of the pieces in which bone `name` is the main bone of at least `share` of the vertices."""
        k = self.bone(name)
        dom = self.main() == k
        out = []
        for pid in np.unique(self.piece[dom]):
            members = self.piece == pid
            if dom[members].sum() >= max(3, share * members.sum()):
                out.append(pid)
        return out

    def write(self):
        obj = self.obj
        s = self.W.sum(axis=1)
        bare = int((s < 1e-6).sum())
        if bare:
            warn("%d vertices carry no bone; put on bricho" % bare)
            self.W[s < 1e-6, self.bone("bricho")] = 1.0
        # at most 4 bones a vertex
        order = np.argsort(-self.W, axis=1)
        keep = np.zeros_like(self.W, dtype=bool)
        np.put_along_axis(keep, order[:, :4], True, axis=1)
        self.W[~keep] = 0.0
        self.W[self.W < 0.02] = 0.0
        self.W /= self.W.sum(axis=1)[:, None]
        for vg in list(obj.vertex_groups):
            obj.vertex_groups.remove(vg)
        for k, name in enumerate(self.names):
            idx = np.nonzero(self.W[:, k] > 0)[0]
            if len(idx) == 0:
                continue
            vg = obj.vertex_groups.new(name=name)
            for i_ in idx:
                vg.add([int(i_)], float(self.W[i_, k]), "REPLACE")


def smoothstep(t):
    t = np.clip(t, 0.0, 1.0)
    return t * t * (3.0 - 2.0 * t)


def fix_weights(skin):
    report = []
    P = skin.P
    for rule in CFG.WEIGHTS[TAG]:
        kind = rule[0]
        if kind == "root":
            _, child, parent, joint, toward, t_in, t_out = rule
            origin = np.array(PIV[joint])
            d = np.array(PIV[toward]) - origin
            d /= np.linalg.norm(d)
            ic, ip = skin.bone(child), skin.bone(parent)
            pieces = skin.pieces_with(child)
            mask = np.isin(skin.piece, pieces) & ((skin.W[:, ic] + skin.W[:, ip]) > 0)
            t = (P[mask] - origin) @ d
            near = t < t_out
            idx = np.nonzero(mask)[0][near]
            s = smoothstep((t[near] - t_in) / (t_out - t_in))
            m = skin.W[idx, ic] + skin.W[idx, ip]
            before = float(skin.W[idx, ip].sum())
            skin.W[idx, ic] = m * s
            skin.W[idx, ip] = m * (1.0 - s)
            report.append("root of %s in %s: %d vertices re-weighted (%d pieces), parent weight %.1f -> %.1f"
                          % (child, parent, len(idx), len(pieces), before, float(skin.W[idx, ip].sum())))
        elif kind == "spine":
            for low, high, y0, y1 in rule[1]:
                il, ih = skin.bone(low), skin.bone(high)
                # the trunk tube: pieces that carry BOTH bones
                both = [p for p in skin.pieces_with(low, 0.01) if p in set(skin.pieces_with(high, 0.01))]
                mask = np.isin(skin.piece, both) & ((skin.W[:, il] + skin.W[:, ih]) > 0)
                idx = np.nonzero(mask)[0]
                s = smoothstep((P[idx, 2] - y0) / (y1 - y0))
                m = skin.W[idx, il] + skin.W[idx, ih]
                skin.W[idx, il] = m * (1.0 - s)
                skin.W[idx, ih] = m * s
                report.append("spine %s to %s between %.1f and %.1f m: %d vertices (%d pieces)" % (low, high, y0, y1, len(idx), len(both)))
        elif kind == "ball":
            s_ = rule[1]
            foot, toe = s_ + "chodidlo", s_ + "prsty"
            origin = np.array(PIV[toe])
            i_f, i_t = skin.bone(foot), skin.bone(toe)
            pieces = [p for p in skin.pieces_with(foot, 0.2)]
            mask = np.isin(skin.piece, pieces) & ((skin.W[:, i_f] + skin.W[:, i_t]) > 0.5)
            idx = np.nonzero(mask)[0]
            t = -(P[idx, 1] - origin[1])                 # metres ahead of the toe joint (forward is -y)
            s = smoothstep((t + 1.1) / 1.7)              # all foot 1.1 m behind the joint, all toes 0.6 m ahead
            m = skin.W[idx, i_f] + skin.W[idx, i_t]
            skin.W[idx, i_f] = m * (1.0 - s)
            skin.W[idx, i_t] = m * s
            report.append("ball of the %s foot: %d vertices, %d now carry both bones" % (s_, len(idx), int(((s > 0.02) & (s < 0.98)).sum())))
    return report


# ---------------------------------------------------------------------------------------------------
# Helper meshes
# ---------------------------------------------------------------------------------------------------
def helper_object(name, collection, hulls=(), points=()):
    """hulls: [(corners (8 x 3), [selection names])]; points: [(co, [selection names])]."""
    verts, faces, groups = [], [], {}
    # corners: 0..3 the near end (u-,v-), (u+,v-), (u+,v+), (u-,v+); 4..7 the far end, same order
    quads = [(0, 3, 2, 1), (4, 5, 6, 7), (0, 1, 5, 4), (1, 2, 6, 5), (2, 3, 7, 6), (3, 0, 4, 7)]
    for corners, names in hulls:
        base = len(verts)
        verts += [tuple(c) for c in corners]
        centre = np.mean(np.array(corners), axis=0)
        for q in quads:
            a, b, c = (np.array(corners[q[k]]) for k in range(3))
            nrm = np.cross(b - a, c - a)
            out = float(np.dot(nrm, a - centre))
            qq = q if out > 0 else tuple(reversed(q))
            faces.append((base + qq[0], base + qq[1], base + qq[2]))
            faces.append((base + qq[0], base + qq[2], base + qq[3]))
        for g in names:
            groups.setdefault(g, []).extend(range(base, base + 8))
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


def frame_for(axis):
    """(u, v, w): w along the axis (or up), u as near to his left as it can be, v square to both."""
    w = np.array([0.0, 0.0, 1.0]) if axis is None else np.array(axis, dtype=np.float64)
    w /= np.linalg.norm(w)
    ref = np.array([1.0, 0.0, 0.0])
    if abs(float(ref @ w)) > 0.9:
        ref = np.array([0.0, 1.0, 0.0])
    u = ref - float(ref @ w) * w
    u /= np.linalg.norm(u)
    v = np.cross(w, u)
    return u, v, w


def fit_box(pts, axis, pct=1.5, min_side=0.0, clip_lo=None):
    """A convex tapered box round `pts`: a rectangle at each end of the axis, both in the same frame.

    Returns 8 corners (near end first) or None when there is nothing to wrap."""
    if len(pts) < 6:
        return None
    u, v, w = frame_for(axis)
    a, b, t = pts @ u, pts @ v, pts @ w
    lo, hi = np.percentile(t, pct), np.percentile(t, 100.0 - pct)
    if clip_lo is not None:
        lo = max(lo, clip_lo)
    if hi - lo < 0.3:
        return None
    span = hi - lo
    ends = []
    for sel in (t <= lo + 0.45 * span, t >= hi - 0.45 * span):
        if sel.sum() < 3:
            sel = np.ones(len(pts), dtype=bool)
        a0, a1 = np.percentile(a[sel], pct), np.percentile(a[sel], 100.0 - pct)
        b0, b1 = np.percentile(b[sel], pct), np.percentile(b[sel], 100.0 - pct)
        if a1 - a0 < min_side:
            m = 0.5 * (a0 + a1)
            a0, a1 = m - 0.5 * min_side, m + 0.5 * min_side
        if b1 - b0 < min_side:
            m = 0.5 * (b0 + b1)
            b0, b1 = m - 0.5 * min_side, m + 0.5 * min_side
        ends.append((a0, a1, b0, b1))
    corners = []
    for (a0, a1, b0, b1), tt in zip(ends, (lo, hi)):
        for aa, bb in ((a0, b0), (a1, b0), (a1, b1), (a0, b1)):
            corners.append(aa * u + bb * v + tt * w)
    return corners


def box_volume(corners):
    c = np.array(corners)
    centre = c.mean(axis=0)
    vol = 0.0
    for q in [(0, 3, 2, 1), (4, 5, 6, 7), (0, 1, 5, 4), (1, 2, 6, 5), (2, 3, 7, 6), (3, 0, 4, 7)]:
        for tri in ((q[0], q[1], q[2]), (q[0], q[2], q[3])):
            a, b, d = c[tri[0]] - centre, c[tri[1]] - centre, c[tri[2]] - centre
            vol += abs(float(np.dot(a, np.cross(b, d)))) / 6.0
    return vol


def joint_or_point(name):
    if name is None:
        return None
    if name.startswith("@"):
        return np.array(PTS[name[1:]])
    return np.array(PIV[name])


# ===================================================================================================
# Build
# ===================================================================================================
meshes = [o for o in bpy.context.scene.objects if o.type == "MESH" and len(o.data.polygons) > 0]
if len(meshes) != 1:
    raise SystemExit("expected one creature mesh, found %s" % [o.name for o in meshes])
src = meshes[0]
SUMMARY["objects"] = [src.name]
manifest = stage_textures(src)
glow_slots = {k for k, s in enumerate(src.material_slots) if is_glow(s.material)}
if len(glow_slots) != 1:
    warn("expected one fullbright material, found %d" % len(glow_slots))

# artist's point empties (A's .blend has them; build space, so they turn with the mesh)
EMPTY = {}
turn = Matrix.Rotation(math.radians(OPT["yaw"]), 4, "Z")
for o in bpy.data.objects:
    if o.type == "EMPTY":
        EMPTY[o.name] = turn @ o.matrix_world.translation

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
    lod0.data.transform(turn)
lod0.data.update()
SUMMARY["yaw_deg"] = OPT["yaw"]
lo, hi = world_bounds(lod0)
if abs(lo.z) > 0.05:
    warn("lowest point is at z = %.2f m; moved by %.2f m so the feet stand on z = 0" % (lo.z, -lo.z))
    lod0.data.transform(Matrix.Translation((0, 0, -lo.z)))
    lod0.data.update()
    lo, hi = world_bounds(lod0)
height = hi.z
H = height
SUMMARY["bounds"] = {"min": [round(c, 3) for c in lo], "max": [round(c, 3) for c in hi]}
SUMMARY["height_m"] = round(height, 3)
SUMMARY["depth_forward_m"] = round(-lo.y, 2)
reach = max(abs(lo.x), abs(hi.x), abs(lo.y), abs(hi.y))
if reach > 20:
    warn("a part reaches %.1f m from the origin in plan view (keep within 20, grid margin 25)" % reach)

# ---- skin -------------------------------------------------------------------------------------------
if OPT["weights"]:
    with open(OPT["weights"], "r", encoding="utf-8") as fh:
        dump = json.load(fh)
    kd = KDTree(len(dump["co"]))
    for k, co in enumerate(dump["co"]):
        kd.insert(turn @ Vector(co), k)
    kd.balance()
    for vg in list(lod0.vertex_groups):
        lod0.vertex_groups.remove(vg)
    groups = {name: lod0.vertex_groups.new(name=name) for name in dump["groups"]}
    worst = 0.0
    for v in lod0.data.vertices:
        _co, k, dist = kd.find(v.co)
        worst = max(worst, dist)
        for name, wgt in dump["weights"][k].items():
            groups[name].add([v.index], wgt, "REPLACE")
    SUMMARY["weights_from"] = OPT["weights"]
    SUMMARY["weights_worst_match_m"] = round(worst, 5)
    if worst > 0.01:
        warn("weights matched by position, worst distance %.4f m" % worst)
for vg in [vg for vg in lod0.vertex_groups if vg.name.lower() not in ALL_BONES]:
    warn("vertex group '%s' is not a bone of SKELETON.md; dropped" % vg.name)
    lod0.vertex_groups.remove(vg)
if "bricho" not in lod0.vertex_groups:
    lod0.vertex_groups.new(name="bricho")

skin = Skin(lod0)
SUMMARY["pieces"] = int(len(np.unique(skin.piece)))
if OPT["weightfix"] == "1":
    SUMMARY["weight_fixes"] = fix_weights(skin)
    for line in SUMMARY["weight_fixes"]:
        say(line)
skin.write()
SUMMARY["skin"] = "bones: %s" % sorted(vg.name for vg in lod0.vertex_groups)
SUMMARY["vertices_by_bone_count"] = {str(k): int(((skin.W > 0).sum(axis=1) == k).sum()) for k in range(1, 5)}

# glow vertices and main bone of every LOD0 vertex (for the hulls and the far LODs)
mesh0 = lod0.data

# ---- the furnace opened up (D): creature_cfg.WOUNDS ---------------------------------------------------
if CFG.WOUNDS.get(TAG) and OPT["wounds"] == "1":
    import mesh_fixes  # noqa: E402
    names_now = np.array([skin.names[k] for k in skin.main()])
    trunk_bones = ("hrudnik", "zebra", "bricho", "ded_rib_l", "ded_rib_r")
    affected = np.isin(names_now, trunk_bones)
    # the trunk's hide: the largest piece of those bones
    pids, cnt = np.unique(skin.piece[affected], return_counts=True)
    hide_piece = skin.piece == pids[np.argmax(cnt)]
    loops = [l for l in mesh_fixes.boundary_loops(mesh0, hide_piece, glow_slots) if len(l) >= 20]
    body_centre = skin.P[hide_piece].mean(axis=0)
    SUMMARY["wounds"] = []
    for which, widen, heighten, reach_, depth_ in CFG.WOUNDS[TAG]:
        if not loops:
            warn("no opening found in the trunk for the wound '%s'" % which)
            continue
        biggest = max(loops, key=len)
        # "front": the largest opening (the furnace). "others": every other one (the split in the back, the flank holes).
        for loop in ([biggest] if which == "front" else [l for l in loops if l is not biggest]):
            newP, winfo = mesh_fixes.open_wound(skin.P, loop, affected, body_centre, widen, heighten, reach_, depth_)
            skin.P = newP
            winfo["which"] = which
            SUMMARY["wounds"].append(winfo)
            say("wound opened (%s): %s" % (which, winfo))
    mesh0.vertices.foreach_set("co", skin.P.ravel())
    mesh0.update()
glow_vert = np.zeros(skin.n, dtype=bool)
for poly in mesh0.polygons:
    if poly.material_index in glow_slots:
        glow_vert[list(poly.vertices)] = True
main_bone = np.array([skin.names[k] for k in skin.main()])

# ---- more points, measured on the mesh ---------------------------------------------------------------
P0 = skin.P
# Sticks (A's telegraph pole and its wires, chain links) are not body: a piece over 6 m long and under
# 0.9 m thick is kept out of the measured points and out of the hulls.
stick = np.zeros(skin.n, dtype=bool)
for pid in np.unique(skin.piece):
    members = skin.piece == pid
    q = P0[members]
    if len(q) < 4:
        continue
    c = q - q.mean(axis=0)
    _u, sv, _vt = np.linalg.svd(c, full_matrices=False)
    ext = np.sort(np.ptp(c @ _vt.T, axis=0))[::-1]
    if ext[0] > 6.0 and ext[1] < 0.9:
        stick[members] = True
SUMMARY["stick_vertices"] = int(stick.sum())


def lowest_of(bones):
    sel = np.isin(main_bone, bones) & ~stick
    if not sel.any():
        return None
    k = np.nonzero(sel)[0][np.argmin(P0[sel, 2])]
    return Vector(P0[k])


def farthest_of(bones, origin):
    sel = np.isin(main_bone, bones) & ~stick
    if not sel.any():
        return None
    d = np.linalg.norm(P0[sel] - np.array(origin), axis=1)
    return Vector(P0[np.nonzero(sel)[0][np.argmax(d)]])


for s_, en in (("l", "l"), ("p", "r")):
    claw = lowest_of([s_ + "ruka"])
    if claw is not None:
        PTS["claw_" + en] = claw
    toe = np.isin(main_bone, [s_ + "prsty"]) & ~stick
    if toe.any():
        k = np.nonzero(toe)[0][np.argmin(P0[toe, 1])]       # the foremost toe or claw tip
        PTS["toe_" + en] = Vector((P0[k][0], P0[k][1], 0.0))
if "ded_hang1" in PIV:
    PTS["deadhead_tip"] = farthest_of(["ded_hang1"], PIV["ded_hang1"])
if "ded_tail3" in PIV:
    PTS["tail_tip"] = farthest_of(["ded_tail3"], PIV["ded_tail3"])
jaw_tip = farthest_of(["ded_jaw"], PIV.get("ded_jaw", PIV["hlava"]))
if jaw_tip is not None:
    PTS["jaw_tip"] = jaw_tip
head_sel = main_bone == "hlava"
PTS["head"] = Vector(0.5 * (np.percentile(P0[head_sel], 5, axis=0) + np.percentile(P0[head_sel], 95, axis=0)))

# ---- the joint file of this round ---------------------------------------------------------------------
rest_out = json.loads(json.dumps(REST))
rest_out.setdefault("anim", {}).update(CFG.ANIM[TAG])
low_claw = min(PTS["claw_l"].z if "claw_l" in PTS else 99.0, PTS["claw_r"].z if "claw_r" in PTS else 99.0)
for en in ("l", "r"):
    if "claw_" + en in PTS and PTS["claw_" + en].z < 0.08 * H:
        rest_out["points"]["claw_" + en] = to_file(PTS["claw_" + en])
if "hand_clearance" in rest_out["anim"] and low_claw > 0.08 * H:
    del rest_out["anim"]["hand_clearance"]
# the fall: on his chest (gen_ded_anim.py, "The fall"). Lifted by the forward depth of the trunk, but not
# by more than leaves the furnace point a twelfth of his height above the ground when he lies.
trunk_sel = np.isin(main_bone, ["hrudnik", "zebra", "bricho", "ded_rib_l", "ded_rib_r"]) & ~stick
trunk_depth = float(-P0[trunk_sel, 1].min())
f_fwd = float(-PTS["furnace"].y) if "furnace" in PTS else 0.5 * trunk_depth
rest_out["anim"]["fall_crumple"] = 1
rest_out["anim"]["fall_depth"] = round(min(trunk_depth, f_fwd + 0.085 * H), 2)
SUMMARY["fall"] = {"trunk_forward_depth_m": round(trunk_depth, 2), "lift_m": rest_out["anim"]["fall_depth"]}
rest_out["note"] = REST.get("note", "") + " Finishing round: anim settings and claw points added by finish_model.py."
rest_path = os.path.join(OUT_DIR, "dedushka_rest.json")
with open(rest_path, "w", encoding="utf-8") as fh:
    json.dump(rest_out, fh, indent=1, sort_keys=True)
SUMMARY["rest_json"] = rest_path

# ---- visual LODs ---------------------------------------------------------------------------------------
lods = [lod0]
far_info = []
if OPT["far"] == "hand":
    import far_lod  # noqa: E402
    far_objs, far_info, far_manifest = far_lod.build(
        lod0=lod0, skin=skin, glow_slots=glow_slots, root=root, height=H, tag=TAG, out_dir=OUT_DIR,
        texdir=OPT["texdir"], manifest=manifest, cfg=CFG.FAR[TAG], lod_collection=lod_collection, say=say, warn=warn,
        furnace=PTS.get("furnace"), glow_grow=CFG.FAR_GLOW_GROW[TAG], glow_gain=CFG.FAR_GLOW_GAIN[TAG],
        drop_bones=CFG.FAR_SKIN_DROP[TAG], glow_votes=CFG.FAR_GLOW_VOTES[TAG])
    lods += far_objs
    manifest += far_manifest
elif OPT["far"] == "auto":
    tris0 = tri_count(lod0.data)
    for k, (frac, share, cap) in enumerate([(0.70, 0.50, None), (0.95, 0.20, None), (1.15, 0.03, 480)], start=1):
        target = tris0 * share
        if cap:
            target = min(target, cap)
        col = lod_collection(root, "lod%d" % k, round(frac * height, 2), {"lodnoshadow": "1"} if k < 3 else None)
        obj = lod0.copy()
        obj.data = lod0.data.copy()
        obj.name = "dedushka_lod%d" % k
        col.objects.link(obj)
        mod = obj.modifiers.new("lod", "DECIMATE")
        mod.ratio = min(1.0, target / tris0)
        mod.use_collapse_triangulate = True
        bpy.context.view_layer.update()
        deps = bpy.context.evaluated_depsgraph_get()
        me = bpy.data.meshes.new_from_object(obj.evaluated_get(deps), preserve_all_data_layers=True, depsgraph=deps)
        obj.modifiers.clear()
        obj.data = me
        lods.append(obj)

# Mip anchor: inside the pelvis.
r = PIV["bricho"]
guesses = [r, r + Vector((0, 0, 1)), r + Vector((0, 0, -1)), r + Vector((0, 1, 0)), r + Vector((0, -1, 0))]
cx, cy = (lo.x + hi.x) * 0.5, (lo.y + hi.y) * 0.5
for zf in (0.45, 0.5, 0.4, 0.55, 0.6):
    for dy in (0.0, 2.0, -2.0, 4.0):
        guesses.append(Vector((cx, cy + dy, zf * height)))
anchor, hits = inside_point(lod0, guesses)
SUMMARY["mip_anchor_at"] = [round(c, 2) for c in anchor]
SUMMARY["mip_anchor_rays_hit"] = "%d of 14" % hits
SUMMARY["lod_detail"] = []
for k, obj in enumerate(lods):
    slots = {s for s, m in enumerate(obj.material_slots) if is_glow(m.material)}
    SUMMARY["lod_detail"].append(finish_lod(obj, slots, anchor if k == 0 else None))
corners = len({(l.vertex_index, tuple(round(c, 4) for c in n.vector), tuple(round(c, 4) for c in uv.uv))
               for l, n, uv in zip(lod0.data.loops, lod0.data.corner_normals, lod0.data.uv_layers.active.data)})
SUMMARY["lod0_split_vertices_estimate"] = corners
if corners > 32767:
    warn("LOD0 has about %d vertices after normal and UV splits; the engine limit is 32767" % corners)

# ---- collision and hit shapes -------------------------------------------------------------------------
floor = max(6.0, CFG.GEO_FLOOR * H)
swing = CFG.GEO_SWING_MARGIN[TAG]
SWINGING = ("biceps", "loket", "ruka", "ded_tail", "ded_hang", "ded_jaw", "stehno")
fire_hulls, geo_hulls, hull_report = [], [], []
region_of = {h: reg for reg, hs in CFG.REGIONS.items() for h in hs}
for name, owner, bones, a_from, a_to, opt in CFG.HULLS[TAG]:
    sel = np.isin(main_bone, bones) & ~stick
    if sel.sum() < 6:
        warn("hull %s: no vertices on %s" % (name, bones))
        continue
    pts = P0[sel]
    axis = None
    if a_from is not None and a_to is not None:
        axis = joint_or_point(a_to) - joint_or_point(a_from)
        if np.linalg.norm(axis) < 0.5:
            axis = None
    min_side = opt.get("min_side", 0.0)
    entries = []
    if opt.get("split_front"):
        # the chest: a furnace piece in front (the fullbright bowl and the rib gates before it), the rest behind
        g = sel & (glow_vert | np.isin(main_bone, ["ded_rib_l", "ded_rib_r"]))
        gl = sel & glow_vert
        v_split = float(np.median(P0[gl, 1])) if gl.any() else float(np.median(pts[:, 1]))
        front = P0[g & (P0[:, 1] <= v_split)]
        back = pts[pts[:, 1] >= v_split]
        entries.append((opt["split_front"], fit_box(front, None, 1.0, min_side)))
        entries.append((name, fit_box(back, None, 1.5, min_side)))
        SUMMARY["furnace_split_y_file_z"] = round(v_split, 2)
    else:
        entries.append((name, fit_box(pts, axis, 0.5 if opt.get("flat") else 1.5, min_side)))
    for hname, corners_ in entries:
        if corners_ is None:
            warn("hull %s: nothing to wrap" % hname)
            continue
        corners_ = [np.array([c_[0], c_[1], max(c_[2], 0.0)]) for c_ in corners_]
        sels = [owner, "ded_hull_" + hname]
        if hname in region_of:
            sels.append(region_of[hname])
        fire_hulls.append((corners_, sels))
        c = np.array(corners_)
        row = {"hull": hname, "bone": owner, "volume_m3": round(box_volume(corners_), 1),
               "low_m": round(float(c[:, 2].min()), 2), "high_m": round(float(c[:, 2].max()), 2), "in_collision": False}
        if not opt.get("fire"):
            # the collision copy: only what is above the floor (swinging parts a little higher still)
            cut = floor + (swing if any(owner.endswith(s) or owner.startswith(s) for s in SWINGING) else 0.0)
            src_pts = front if hname == opt.get("split_front") else (back if opt.get("split_front") else pts)
            keep = src_pts[src_pts[:, 2] >= cut]
            gbox = fit_box(keep, axis if hname == name else None, 1.5, min_side) if len(keep) >= 6 else None
            if gbox is not None:
                g_ = np.array(gbox)
                if g_[:, 2].min() < cut:            # a tilted box can dip under the cut at a corner: lift those corners
                    for k in range(8):
                        if gbox[k][2] < cut:
                            gbox[k] = np.array([gbox[k][0], gbox[k][1], cut])
                geo_hulls.append((gbox, [owner, "ded_hull_" + hname]))
                row["in_collision"] = True
                row["collision_low_m"] = round(float(np.array(gbox)[:, 2].min()), 2)
        hull_report.append(row)
SUMMARY["hulls"] = hull_report
SUMMARY["geometry_floor_m"] = round(floor, 2)


def half_diagonal(hulls):
    c = np.concatenate([np.array(h[0]) for h in hulls])
    return 0.5 * float(np.linalg.norm(c.max(axis=0) - c.min(axis=0))), 0.5 * (c.max(axis=0) + c.min(axis=0))


# The engine's "geometry radius" is half the diagonal of the Geometry LOD's box. The AI's "is it worth
# firing" gate and the damage scale both read it (VehicleAICombat.cpp, Target.cpp, Dammage.cpp), and
# the combat feature's numbers were measured on the radius of the build before this one. So the whole
# collision shape is drawn in about its centre until its radius is that one again: who fires at him
# and what a round costs stay as tested.
r_now, centre = half_diagonal(geo_hulls)
r_want = CFG.GEO_RADIUS[TAG]
SUMMARY["geometry_radius_as_fitted_m"] = round(r_now, 3)
if r_now > r_want:
    f = r_want / r_now
    geo_hulls = [([centre + f * (np.array(c_) - centre) for c_ in h[0]], h[1]) for h in geo_hulls]
r_now, _c = half_diagonal(geo_hulls)
SUMMARY["geometry_radius_m"] = round(r_now, 3)
SUMMARY["geometry_lowest_m"] = round(float(min(np.array(h[0])[:, 2].min() for h in geo_hulls)), 2)
say("collision shape: %d boxes, radius %.2f m (fitted %.2f), lowest point %.2f m; hit shape: %d boxes"
    % (len(geo_hulls), r_now, SUMMARY["geometry_radius_as_fitted_m"], SUMMARY["geometry_lowest_m"], len(fire_hulls)))

# What the combat feature needs to tell the place of a hit on THIS model (gen_config.py writes them into
# the creature's class; models-tests\tools\patch_zone.py is the rule that reads them).
by_name = {h[1][1][len("ded_hull_"):]: np.array(h[0]) for h in fire_hulls}
combat = {"legs_up_m": round(max(PIV["lstehno"].z, PIV["pstehno"].z) + 0.5, 2)}
if "head" in by_name:
    hc = np.array(PTS["head"])
    far_c = float(np.linalg.norm(by_name["head"] - hc, axis=1).max())
    combat.update({"head_point": "ded_head", "head_r_m": round(0.75 * far_c, 2),
                   "head_right_fwd_up": [round(-float(hc[0]), 2), round(-float(hc[1]), 2), round(float(hc[2]), 2)]})
if "furnace" in by_name:
    fb = by_name["furnace"]
    combat["furnace_r_m"] = round(0.75 * 0.5 * float(np.hypot(np.ptp(fb[:, 0]), np.ptp(fb[:, 2]))), 2)
SUMMARY["combat"] = combat
say("hit regions: %s" % combat)

geo = lod_collection(root, "Geometry", "Geometry", {"autocenter": "0"})
geo["p3d_mass"] = MASS_KG
geo_obj = helper_object("dedushka_geometry", geo, hulls=geo_hulls)
fire = lod_collection(root, "FireGeometry", "FireGeometry")
fire_obj = helper_object("dedushka_firegeometry", fire, hulls=fire_hulls)

# ---- memory points --------------------------------------------------------------------------------------
mem = lod_collection(root, "Memory", "Memory")
furnace = PTS.get("furnace", (PIV["hrudnik"] + PIV["krk"]) * 0.5)
mouth = PTS.get("mouth", PIV["hlava"])
eye = EMPTY.get("pilot", (PIV["hlava"] + PTS.get("head_top", PIV["hlava"]) + mouth) / 3.0)
hand_r, hand_l = PTS.get("hand_r", PIV["pruka"]), PTS.get("hand_l", PIV["lruka"])
foot_l, foot_r = PTS.get("foot_l", PIV["lchodidlo"]), PTS.get("foot_r", PIV["pchodidlo"])
back = PIV["hrudnik"] + Vector((0, 4.0, 3.0))


def sole_bone(side, p):
    # a sole point ahead of the toe joint lies on the toes (SKELETON.md, section 3)
    return side + ("prsty" if p.y < PIV[side + "prsty"].y - 1e-6 else "chodidlo")


furnace_dir = EMPTY.get("ded_furnace_dir")
if furnace_dir is None or (furnace_dir - furnace).length < 0.2:
    furnace_dir = furnace + Vector((0, -1, 0))
else:
    furnace_dir = furnace + (furnace_dir - furnace).normalized()
pts = [
    (furnace, ["zamerny", "hrudnik"]),
    (PIV["bricho"] + Vector((0, 0, 6)), ["osa mireni", "bricho"]),
    (PIV["krk"], ["osa otaceni", "hrudnik"]),
    (eye, ["pilot", "hlava"]),
    (Vector((foot_l.x, foot_l.y, 0.05)), ["stopaL", sole_bone("l", foot_l)]),
    (Vector((foot_r.x, foot_r.y, 0.05)), ["stopaP", sole_bone("p", foot_r)]),
    (hand_r, ["zbran"]), (hand_r + Vector((0, -2, 0)), ["zbran"]),
    (back, ["roura"]), (back + Vector((0, 0, 2)), ["roura"]),
    (hand_r, ["pruka", "granat"]), (hand_l, ["lruka", "granat2"]),
    (furnace, ["ded_furnace", "hrudnik"]), (furnace_dir, ["ded_furnace_dir", "hrudnik"]),
    (mouth, ["ded_mouth", "hlava"]),
    (furnace + Vector((0, 0, -1)), ["doplnovani", "hrudnik"]), (furnace + Vector((0, 0, -2)), ["doplnovani2", "hrudnik"]),
    # this round: points the scripts can ask for by name (kjPoint)
    (PTS["head"], ["ded_head", "hlava"]),
    (PTS.get("head_top", PIV["hlava"]), ["ded_head_top", "hlava"]),
    (hand_l, ["ded_hand_l", "lruka"]), (hand_r, ["ded_hand_r", "pruka"]),
    (Vector((foot_l.x, foot_l.y, 0.05)), ["ded_sole_l", sole_bone("l", foot_l)]),
    (Vector((foot_r.x, foot_r.y, 0.05)), ["ded_sole_r", sole_bone("p", foot_r)]),
    (PIV["lholen"], ["ded_knee_l", "lholen"]), (PIV["pholen"], ["ded_knee_r", "pholen"]),
]
for en, side in (("l", "l"), ("r", "p")):
    if "heel_" + en in PTS:
        hp = PTS["heel_" + en]
        pts.append((Vector((hp.x, hp.y, 0.05)), ["ded_heel_" + en, sole_bone(side, hp)]))
    if "toe_" + en in PTS:
        pts.append((Vector((PTS["toe_" + en].x, PTS["toe_" + en].y, 0.05)), ["ded_toe_" + en, side + "prsty"]))
    if "claw_" + en in PTS:
        pts.append((PTS["claw_" + en], ["ded_claw_" + en, side + "ruka"]))
if "jaw_tip" in PTS:
    pts.append((PTS["jaw_tip"], ["ded_jaw_tip", "ded_jaw"]))
if "tail_tip" in PTS:
    pts.append((PTS["tail_tip"], ["ded_tail_tip", "ded_tail3"]))
if "deadhead_tip" in PTS:
    pts.append((PTS["deadhead_tip"], ["ded_deadhead", "ded_hang1"]))
# the eyes: given in the settings (file space), then moved onto the nearest face of the head
bvh0 = BVHTree.FromPolygons([v.co for v in mesh0.vertices], [tuple(p.vertices) for p in mesh0.polygons])
for en, guess in (CFG.EYES.get(TAG) or {}).items():
    g = eye + Vector(guess["from_pilot"]) if "from_pilot" in guess else from_file(guess["at"])
    hit = bvh0.find_nearest(g)
    at = hit[0] if hit[0] is not None else g
    pts.append((at, ["ded_eye_" + en, "hlava"]))
for bone in ALL_BONES:
    if bone in PIV and bone not in ("lzadek", "pzadek", "roura", "zbran"):
        pts.append((PIV[bone], [bone] if bone != "bricho" else ["bricho"]))
helper_object("dedushka_memory", mem, points=pts)
mem_report = []
for co, names in pts:
    hit = bvh0.find_nearest(Vector(co))
    mem_report.append({"point": names[0], "bone": names[1] if len(names) > 1 else names[0], "at_file": to_file(Vector(co)),
                       "to_surface_m": round(hit[3], 2) if hit[0] is not None else None})
SUMMARY["memory"] = mem_report

land = lod_collection(root, "LandContact", "LandContact")
helper_object("dedushka_landcontact", land, points=[((0, 0, 0), [])])
hit_col = lod_collection(root, "HitPoints", "HitPoints")
hp = [(PTS["head"], ["hlava"]), (0.5 * (PIV["hrudnik"] + furnace), ["hrudnik", "telo"]), (PIV["bricho"], ["bricho", "telo"])]
for s_ in ("l", "p"):
    hp += [(0.5 * (PIV[s_ + "biceps"] + PIV[s_ + "loket"]), [s_ + "biceps", "ruce"]),
           (0.5 * (PIV[s_ + "loket"] + PIV[s_ + "ruka"]), [s_ + "loket", "ruce"]),
           (0.5 * (PIV[s_ + "stehno"] + PIV[s_ + "holen"]), [s_ + "stehno", "nohy"]),
           (0.5 * (PIV[s_ + "holen"] + PIV[s_ + "chodidlo"]), [s_ + "holen", "nohy"]),
           (PIV[s_ + "chodidlo"], [s_ + "chodidlo", "nohy"])]
helper_object("dedushka_hitpoints", hit_col, points=hp)

# ---- export ---------------------------------------------------------------------------------------------
out_p3d = os.path.join(OUT_DIR, "dedushka.p3d")
result = export_p3d(out_p3d, root)
WARN.extend(result.get("warnings") or [])
SUMMARY["p3d"] = out_p3d
SUMMARY["lods"] = result["lods"]
SUMMARY["textures"] = manifest
SUMMARY["far"] = far_info
long_tex = [t for l in result["lods"] for t in l["textures"] if len(t) > 31]
if long_tex:
    raise SystemExit("texture names over 31 characters: %s" % sorted(set(long_tex)))
with open(os.path.join(OUT_DIR, "dedushka_export.json"), "w", encoding="utf-8") as fh:
    json.dump(SUMMARY, fh, indent=1, default=str)
if OPT["blend"]:
    bpy.ops.wm.save_as_mainfile(filepath=os.path.abspath(OPT["blend"]), check_existing=False, compress=True)
say("%s, wrote %s (%d bytes), %d textures, %d warnings" % (SUMMARY["mode"], out_p3d, result["bytes"], len(manifest), len(WARN)))
LOG.close()
