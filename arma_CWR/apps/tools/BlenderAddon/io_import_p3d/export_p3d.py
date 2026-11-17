"""Blender exporter for OFP-era MLOD P3D (SP3X LODs), the format the engine
loads at runtime. The byte layout lives in mlod_writer.py (no bpy needed).

Conventions (see also the README next to the creature notes):

* One collection per LOD. The resolution comes from, in order: the collection
  custom property ``p3d_resolution`` (number or name), ``cwr_lod_resolution`` on
  the collection or on all of its direct mesh objects (what the importer
  writes), or the collection name ("0.15", "1.0", "1e13", "Geometry",
  "Memory", "LandContact", "HitPoints", "FireGeometry", "ViewGeometry",
  "ViewPilot", ...). A trailing Blender ".001" suffix is ignored.
  Collections nested inside a LOD collection that are not LODs themselves
  (for example the importer's "*_Proxies" collections) belong to that LOD.
* Every mesh object in a LOD collection is merged into that LOD. Vertices
  become points; polygons become triangles and quads (n-gons are split).
* Axis mapping is the inverse of the importer: Blender (x, y, z) -> game
  (x, z, y). That swap is a mirror, so every face's corner order is reversed
  on the way out (and on the way in by the importer): front faces in Blender
  stay front faces in the engine. Check it against a stock model, never a
  round trip alone, which cannot see a winding error made on both sides.
* Vertex groups become named selections (weights use the O2 byte encoding).
  A face is in a selection when all of its vertices are.
* Texture per face: material custom property ``p3d_texture``, else the
  importer's ``cwr_texture``, else the first Image Texture node's image
  (its ``p3d_texture`` property, else its file name with the extension
  changed to .paa and the exporter's texture prefix in front).
* Named properties: custom properties ``p3d_prop_<name>`` on the LOD
  collection or its objects, plus the string ``cwr_<name>`` properties the
  importer writes. Model-wide ones (autocenter, class, map) belong on the
  Geometry LOD, which the engine searches first.
* ``#Mass#`` (Geometry LOD only): a float POINT attribute ``p3d_mass``, else a
  total ``p3d_mass`` on the collection or object (or the operator's Mass
  field) spread evenly over the Geometry points.
* Proxies: mesh objects or empties carrying ``p3d_proxy`` / ``cwr_proxy_path``
  or named ``proxy:...``. A mesh proxy keeps its triangle; an empty gets a
  triangle built from its location, local +Y (forward) and +Z (up).
"""

import os
import re

import bpy
from bpy.props import BoolProperty, FloatProperty, StringProperty
from bpy_extras.io_utils import ExportHelper
from mathutils import Vector

from . import mlod_writer as mw

# Importer bookkeeping keys that are not P3D named properties.
_IMPORTER_KEYS = {"lod_index", "lod_resolution", "format", "center_offset", "bounding_center", "proxy_path",
                  "proxy_selection", "has_noshadow_faces", "nolight_face_count", "texture", "surface"}

_BLENDER_SUFFIX = re.compile(r"\.\d{3,}$")


def _to_game(v):
    """Blender (x, y, z) -> game (x, y, z): the importer maps game (x, y, z) to (x, z, y)."""
    return (v[0], v[2], v[1])


def _vec3_prop(idblock, key):
    val = idblock.get(key) if idblock is not None else None
    if val is None:
        return None
    try:
        vals = [float(x) for x in val]
    except (TypeError, ValueError):
        return None
    return tuple(vals) if len(vals) == 3 else None


def _inherited_vec3(obj, key):
    o = obj
    while o is not None:
        val = _vec3_prop(o, key)
        if val is not None:
            return val
        o = o.parent
    return None


def collection_resolution(col):
    """LOD resolution for a collection, or None if it is not a LOD collection."""
    for key in ("p3d_resolution", "cwr_lod_resolution"):
        if key in col:
            val = col[key]
            res = mw.parse_resolution(val if isinstance(val, (int, float)) else str(val))
            if res is None:
                raise mw.MlodError("collection '%s': %s = %r is not a LOD resolution" % (col.name, key, val))
            return res
    vals = [float(o["cwr_lod_resolution"]) for o in col.objects if o.type == "MESH" and "cwr_lod_resolution" in o]
    if vals:
        if max(vals) - min(vals) > 1e-3 * max(abs(v) for v in vals):
            raise mw.MlodError("collection '%s' holds objects from different imported LODs %s" % (col.name, vals))
        return vals[0]
    name = col.name
    res = mw.parse_resolution(name)
    if res is None and _BLENDER_SUFFIX.search(name):
        res = mw.parse_resolution(_BLENDER_SUFFIX.sub("", name))
    return res


def find_lod_collections(root):
    """[(collection, resolution)] for every LOD collection under ``root`` (itself included)."""
    found = []

    def walk(col):
        res = collection_resolution(col)
        if res is not None:
            found.append((col, res))
            return
        for child in col.children:
            walk(child)

    walk(root)
    return found


def _lod_objects(col, lod_cols):
    """Objects of a LOD collection, including non-LOD child collections."""
    out = []
    seen = set()

    def walk(c):
        for o in c.objects:
            if o.name not in seen:
                seen.add(o.name)
                out.append(o)
        for ch in c.children:
            if ch not in lod_cols:
                walk(ch)

    walk(col)
    return out


def _proxy_info(obj):
    """(selection_name or None, proxy_path or None) when obj is a proxy, else None."""
    sel = obj.get("cwr_proxy_selection") or obj.get("p3d_proxy_selection")
    path = obj.get("p3d_proxy") or obj.get("cwr_proxy_path")
    name = _BLENDER_SUFFIX.sub("", obj.name)
    if sel:
        return str(sel), None
    if path:
        return None, str(path)
    if name.lower().startswith("proxy:"):
        return name, None
    return None


def _texture_for_material(mat, prefix, cache, warnings):
    if mat is None:
        return ""
    if mat.name in cache:
        return cache[mat.name]
    tex = mat.get("p3d_texture")
    if tex is None:
        tex = mat.get("cwr_texture")
    if tex is None and mat.use_nodes and mat.node_tree:
        for node in mat.node_tree.nodes:
            if node.type == "TEX_IMAGE" and node.image:
                img = node.image
                tex = img.get("p3d_texture")
                if tex is None:
                    base = os.path.basename(bpy.path.abspath(img.filepath)) if img.filepath else img.name
                    base = os.path.splitext(base)[0] + ".paa"
                    tex = prefix + base
                    if "\\" not in tex:
                        warnings.append("material '%s': texture '%s' has no folder; set a texture prefix or "
                                        "p3d_texture" % (mat.name, tex))
                break
    tex = str(tex or "").replace("/", "\\").lstrip("\\")
    cache[mat.name] = tex
    return tex


def _p3d_properties(idblock, include_importer):
    props = []
    for key in idblock.keys():
        val = idblock[key]
        if key.startswith("p3d_prop_"):
            props.append((key[len("p3d_prop_"):], str(val)))
        elif include_importer and key.startswith("cwr_") and isinstance(val, str):
            name = key[4:]
            if name in _IMPORTER_KEYS or name.startswith("centroid_"):
                continue
            props.append((name, val))
    return props


def _float_prop(idblock, key):
    if idblock is None or key not in idblock:
        return None
    try:
        return float(idblock[key])
    except (TypeError, ValueError):
        return None


class _LodBuilder:
    def __init__(self, resolution):
        self.lod = mw.MlodLod(resolution=resolution)
        self.lod.point_flags = []
        self._normal_index = {}
        self.mass_points = []  # per point mass from attributes, None where unknown

    def add_point(self, p, flags=0, mass=None):
        self.lod.points.append(tuple(float(c) for c in p))
        self.lod.point_flags.append(int(flags) & 0xFFFFFFFF)
        self.mass_points.append(mass)
        return len(self.lod.points) - 1

    def add_normal(self, n):
        key = (round(n[0], 5), round(n[1], 5), round(n[2], 5))
        idx = self._normal_index.get(key)
        if idx is None:
            idx = len(self.lod.normals)
            self.lod.normals.append(key)
            self._normal_index[key] = idx
        return idx

    def add_face(self, engine_corners, texture="", flags=0):
        """engine_corners: [(point, normal, u, v)] in engine/Blender front-facing order."""
        self.lod.faces.append(mw.MlodFace(corners=mw.file_order(engine_corners), texture=texture,
                                          flags=int(flags) & 0xFFFFFFFF))
        return len(self.lod.faces) - 1

    def select(self, name, point, weight):
        if mw.encode_weight(weight) == 0:
            return
        sel = self.lod.get_or_add_selection(name)
        sel.points[point] = max(weight, sel.points.get(point, 0.0))


def _evaluated_mesh(obj, depsgraph, depsgraph_names, apply_modifiers, warnings):
    """(mesh, owner_to_clear or None). Armature modifiers are switched off by
    export_p3d before evaluation, so the rest pose is exported."""
    if not apply_modifiers or not any(m.show_viewport for m in obj.modifiers):
        return obj.data, None
    if obj.name not in depsgraph_names:
        warnings.append("'%s' is not in the view layer (excluded collection?); modifiers were not applied" % obj.name)
        return obj.data, None
    eval_obj = obj.evaluated_get(depsgraph)
    return eval_obj.to_mesh(), eval_obj


def _corner_normals(mesh):
    try:
        return [tuple(cn.vector) for cn in mesh.corner_normals]
    except AttributeError:  # Blender < 4.1
        mesh.calc_normals_split()
        return [tuple(l.normal) for l in mesh.loops]


def _face_flag_attribute(obj, mesh):
    attr = mesh.attributes.get("p3d_face_flags")
    if attr is None and obj.get("cwr_format") == "MLOD":
        # The importer's cwr_face_flags hold file flags only for MLOD sources;
        # for ODOL sources they are engine runtime flags and are not written.
        attr = mesh.attributes.get("cwr_face_flags")
    if attr is not None and attr.domain == "FACE" and attr.data_type == "INT":
        return [a.value for a in attr.data]
    return None


def _point_attribute(mesh, name, data_type):
    attr = mesh.attributes.get(name)
    if attr is not None and attr.domain == "POINT" and attr.data_type == data_type:
        return [a.value for a in attr.data]
    return None


def _add_mesh_object(builder, obj, mesh, offset, opts, tex_cache, warnings, proxy_sel_name=None, skip_groups=()):
    mw_mat = obj.matrix_world
    nrm_mat = mw_mat.to_3x3().inverted_safe().transposed()
    mirrored = mw_mat.to_3x3().determinant() < 0

    pflags = _point_attribute(mesh, "p3d_point_flags", "INT") or [0] * len(mesh.vertices)
    pmass = _point_attribute(mesh, "p3d_mass", "FLOAT")
    base = len(builder.lod.points)
    for v in mesh.vertices:
        g = _to_game(mw_mat @ v.co)
        builder.add_point((g[0] + offset[0], g[1] + offset[1], g[2] + offset[2]),
                          pflags[v.index], pmass[v.index] if pmass else None)

    uv_layer = mesh.uv_layers.active
    normals = _corner_normals(mesh) if mesh.polygons else []
    fflags = _face_flag_attribute(obj, mesh)
    slots = obj.material_slots

    def corner(li):
        vi = mesh.loops[li].vertex_index
        n = nrm_mat @ Vector(normals[li])
        if n.length > 1e-12:
            n.normalize()
        else:
            n = Vector((0.0, 0.0, 1.0))
        u, v = (uv_layer.data[li].uv if uv_layer else (0.0, 0.0))
        return (base + vi, builder.add_normal(_to_game(n)), float(u), float(1.0 - v))

    first_face = len(builder.lod.faces)
    tri_cache = None
    for poly in mesh.polygons:
        mat = slots[poly.material_index].material if poly.material_index < len(slots) else None
        texture = _texture_for_material(mat, opts["texture_prefix"], tex_cache, warnings)
        flags = fflags[poly.index] if fflags else 0
        if poly.loop_total <= 4:
            loops_list = [list(poly.loop_indices)]
        else:
            if tri_cache is None:
                mesh.calc_loop_triangles()
                tri_cache = {}
                for lt in mesh.loop_triangles:
                    tri_cache.setdefault(lt.polygon_index, []).append(list(lt.loops))
            loops_list = tri_cache.get(poly.index, [])
        for loops in loops_list:
            corners = [corner(li) for li in loops]
            # The axis swap in _to_game is a mirror, so a Blender front face
            # (counter-clockwise loops) comes out clockwise unless its corners
            # are reversed; a negative-scale object mirrors once more. Proxy
            # triangles keep their order: the engine reads their placement
            # from the point order, not the winding.
            if (proxy_sel_name is None) != mirrored:
                corners.reverse()
            builder.add_face(corners, texture, flags)

    # vertex groups -> selections
    names = {vg.index: vg.name for vg in obj.vertex_groups}
    for v in mesh.vertices:
        for g in v.groups:
            name = names.get(g.group)
            if name is None or name.lower() in skip_groups:
                continue  # a proxy object of this LOD already defines it
            builder.select(name, base + v.index, g.weight)
    if proxy_sel_name:
        sel = builder.lod.get_or_add_selection(proxy_sel_name)
        for i in range(len(mesh.vertices)):
            sel.points[base + i] = 1.0
        sel.faces.update(range(first_face, len(builder.lod.faces)))
    return len(mesh.vertices)


def _add_empty_proxy(builder, obj, offset, sel_name):
    m = obj.matrix_world
    origin = m.to_translation()
    fwd = (m.to_3x3() @ Vector((0.0, 1.0, 0.0))).normalized()
    up = (m.to_3x3() @ Vector((0.0, 0.0, 1.0))).normalized()
    # Engine (LODShape::Load): p0-p1 is the shortest edge (direction), p0-p2 the
    # second (up). Sides 0.1 and 0.2 keep that order (squared 0.01, 0.04, 0.05)
    # while staying small enough not to grow the bounding sphere.
    pts = [origin, origin + fwd * 0.1, origin + up * 0.2]
    idx = []
    for p in pts:
        g = _to_game(p)
        idx.append(builder.add_point((g[0] + offset[0], g[1] + offset[1], g[2] + offset[2])))
    n = builder.add_normal(_to_game((fwd.cross(up)).normalized()))
    fi = builder.add_face([(i, n, 0.0, 0.0) for i in idx])
    sel = builder.lod.get_or_add_selection(sel_name)
    for i in idx:
        sel.points[i] = 1.0
    sel.faces.add(fi)


def build_lods(root, opts, warnings):
    """Collect LOD collections under ``root`` and build MlodLod objects."""
    context = bpy.context
    lod_entries = find_lod_collections(root)
    if not lod_entries:
        raise mw.MlodError("no LOD collections found under '%s'" % root.name)
    lod_cols = {c for c, _ in lod_entries}
    depsgraph = context.evaluated_depsgraph_get()
    depsgraph_names = {o.name for o in depsgraph.objects}
    tex_cache = {}
    lods = []
    for col, res in lod_entries:
        builder = _LodBuilder(res)
        props = _p3d_properties(col, include_importer=False)
        proxy_ids = {}
        objects = [o for o in _lod_objects(col, lod_cols) if not o.get("p3d_ignore")]
        if not opts["include_hidden"]:
            objects = [o for o in objects if o.visible_get()]
        # Proxy objects first, so their selection names are known when the
        # main meshes' vertex groups are read.
        jobs = []
        for obj in objects:
            proxy = _proxy_info(obj)
            sel_name = None
            if proxy is not None:
                sel_name, path = proxy
                if sel_name is None:
                    proxy_ids[path.lower()] = proxy_ids.get(path.lower(), 0) + 1
                    pid = int(obj.get("p3d_proxy_id", proxy_ids[path.lower()]))
                    sel_name = "proxy:%s.%02d" % (path, pid)
            jobs.append((sel_name is None, obj, sel_name))
        jobs.sort(key=lambda j: j[0])
        proxy_sels = {j[2].lower() for j in jobs if j[2]}
        # Keep the vertex group order of the main meshes as the selection order;
        # selections that end up empty are dropped below.
        for _, obj, sel_name in jobs:
            if sel_name is None and obj.type == "MESH":
                for vg in obj.vertex_groups:
                    builder.lod.get_or_add_selection(vg.name)
        for _, obj, sel_name in jobs:
            offset = (0.0, 0.0, 0.0)
            if opts["restore_import_origin"]:
                for key in ("cwr_center_offset", "cwr_bounding_center"):
                    off = _inherited_vec3(obj, key)
                    if off:
                        offset = (offset[0] + off[0], offset[1] + off[1], offset[2] + off[2])
            if obj.type == "MESH":
                mesh, owner = _evaluated_mesh(obj, depsgraph, depsgraph_names, opts["apply_modifiers"], warnings)
                try:
                    if sel_name and (len(mesh.vertices) != 3 or len(mesh.polygons) != 1):
                        warnings.append("proxy '%s' (%s) should be a single triangle" % (obj.name, sel_name))
                    _add_mesh_object(builder, obj, mesh, offset, opts, tex_cache, warnings, sel_name,
                                     skip_groups=proxy_sels)
                finally:
                    if owner is not None:
                        owner.to_mesh_clear()
                for p in _p3d_properties(obj, include_importer=True):
                    props.append(p)
            elif obj.type == "EMPTY" and sel_name:
                _add_empty_proxy(builder, obj, offset, sel_name)
        lod = builder.lod
        lod.selections = [s for s in lod.selections if s.points]
        # Named properties, first occurrence wins (collection before objects).
        seen = set()
        for name, value in props:
            if name.lower() not in seen:
                seen.add(name.lower())
                lod.properties.append((name, value))
        # Faces of each selection: all corners selected (proxy objects set their own face).
        for sel in lod.selections:
            if sel.name.lower() not in proxy_sels:
                sel.faces = mw.faces_fully_in(lod, set(sel.points))
        if opts["auto_components"] and mw.wants_components(res) and lod.faces:
            n = mw.auto_components(lod)
            if n:
                warnings.append("LOD %s: added %d ComponentNN selections" % (mw.lod_name(res), n))
        if mw.is_spec(res, mw.GEOMETRY_SPEC) and lod.points:
            if all(m is not None for m in builder.mass_points):
                lod.mass = [float(m) for m in builder.mass_points]
            else:
                total = opts["mass"] if opts["mass"] > 0 else None
                if total is None:
                    total = _float_prop(col, "p3d_mass")
                if total is None:
                    for o in objects:
                        total = _float_prop(o, "p3d_mass")
                        if total is not None:
                            break
                if total is not None and total > 0:
                    each = total / len(lod.points)
                    lod.mass = [each] * len(lod.points)
                else:
                    warnings.append("Geometry LOD has no mass (set p3d_mass); the object will weigh nothing")
        if not lod.points:
            warnings.append("LOD %s from '%s' is empty" % (mw.lod_name(res), col.name))
        lods.append(lod)
    return lods


def export_p3d(filepath, collection=None, *, restore_import_origin=True, apply_modifiers=True,
               include_hidden=True, auto_components=True, texture_prefix="", mass=0.0, verify=True):
    """Export LOD collections to an MLOD P3D. Headless entry point.

    collection: a bpy Collection, a collection name, or None for the whole scene.
    Returns a dict with per-LOD counts and the list of warnings.
    """
    if collection is None:
        root = bpy.context.scene.collection
    elif isinstance(collection, str):
        root = bpy.data.collections.get(collection)
        if root is None:
            raise mw.MlodError("collection '%s' not found" % collection)
    else:
        root = collection
    if texture_prefix:
        texture_prefix = texture_prefix.replace("/", "\\").lstrip("\\")
        if not texture_prefix.endswith("\\"):
            texture_prefix += "\\"
    opts = {
        "restore_import_origin": restore_import_origin,
        "apply_modifiers": apply_modifiers,
        "include_hidden": include_hidden,
        "auto_components": auto_components,
        "texture_prefix": texture_prefix,
        "mass": float(mass or 0.0),
    }
    warnings = []
    # Export the rest pose: switch armature modifiers off while meshes are evaluated.
    disabled = []
    if apply_modifiers:
        for obj in root.all_objects:
            for m in getattr(obj, "modifiers", ()):
                if m.type == "ARMATURE" and m.show_viewport:
                    m.show_viewport = False
                    disabled.append(m)
        if disabled:
            bpy.context.view_layer.update()
    try:
        lods = build_lods(root, opts, warnings)
    finally:
        for m in disabled:
            m.show_viewport = True
        if disabled:
            bpy.context.view_layer.update()
    data = mw.write_mlod(lods)
    if verify:
        _ver, _lods, read_warnings = mw.read_mlod(data, strict=True)
        warnings.extend(read_warnings)
    os.makedirs(os.path.dirname(os.path.abspath(filepath)), exist_ok=True)
    with open(filepath, "wb") as f:
        f.write(data)
    summary = {
        "path": filepath,
        "bytes": len(data),
        "lods": [
            {
                "resolution": l.resolution,
                "name": mw.lod_name(l.resolution),
                "points": len(l.points),
                "faces": len(l.faces),
                "selections": [s.name for s in l.selections],
                "textures": sorted({f.texture for f in l.faces}),
                "properties": list(l.properties),
                "mass": sum(l.mass) if l.mass else None,
            }
            for l in sorted(lods, key=lambda x: x.resolution)
        ],
        "warnings": warnings,
    }
    for w in warnings:
        print("[P3D export] warning: %s" % w)
    print("[P3D export] wrote %s (%d LODs, %d bytes)" % (filepath, len(lods), len(data)))
    return summary


class ExportP3D(bpy.types.Operator, ExportHelper):
    """Export LOD collections as an OFP-era MLOD P3D (SP3X)"""

    bl_idname = "export_scene.p3d"
    bl_label = "Export P3D"
    bl_options = {"REGISTER", "PRESET"}

    filename_ext = ".p3d"
    filter_glob: StringProperty(default="*.p3d", options={"HIDDEN"})

    collection: StringProperty(
        name="Collection",
        description="Root collection holding the LOD collections (empty = whole scene)",
        default="",
    )
    restore_import_origin: BoolProperty(
        name="Restore Imported Origin",
        description="Undo the importer's re-centring (cwr_center_offset, cwr_bounding_center) so coordinates "
        "return to the source file's model space",
        default=True,
    )
    apply_modifiers: BoolProperty(name="Apply Modifiers", description="Armature modifiers are always skipped",
                                  default=True)
    include_hidden: BoolProperty(name="Include Hidden", description="Export hidden and excluded LOD objects too",
                                 default=True)
    auto_components: BoolProperty(
        name="Auto Components",
        description="Add ComponentNN selections to geometry LODs that have none (one per closed piece)",
        default=True,
    )
    texture_prefix: StringProperty(
        name="Texture Prefix",
        description="Folder put in front of image file names without p3d_texture, e.g. chupa\\",
        default="",
    )
    mass: FloatProperty(name="Mass (kg)", description="Total Geometry LOD mass; 0 = use p3d_mass properties",
                        default=0.0, min=0.0)

    def invoke(self, context, event):
        lc = context.view_layer.active_layer_collection
        if not self.collection and lc and lc.collection is not context.scene.collection:
            self.collection = lc.collection.name
        return ExportHelper.invoke(self, context, event)

    def execute(self, context):
        try:
            summary = export_p3d(
                self.filepath,
                self.collection or None,
                restore_import_origin=self.restore_import_origin,
                apply_modifiers=self.apply_modifiers,
                include_hidden=self.include_hidden,
                auto_components=self.auto_components,
                texture_prefix=self.texture_prefix,
                mass=self.mass,
            )
        except mw.MlodError as e:
            self.report({"ERROR"}, str(e))
            return {"CANCELLED"}
        for w in summary["warnings"][:10]:
            self.report({"WARNING"}, w)
        self.report({"INFO"}, "Exported %d LODs to %s" % (len(summary["lods"]), self.filepath))
        return {"FINISHED"}
