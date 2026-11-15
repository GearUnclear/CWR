"""Far LODs of a Dedushka creature, made for their distance (called by finish_model.py, Blender 4.3).

Not a decimation of LOD0. For each far LOD:

1. The body is taken apart into the groups that move together (trunk, head, each arm, each leg, tail,
   the hanging head). Small pieces (teeth, tatters, chain, drool) and sticks (the pole, wires) are left
   out; claws are set aside.
2. Each group is rebuilt as ONE closed skin over everything in it (a voxel remesh at a size chosen
   for the distance): holes are shut, ribs merge with the chest, tubes pushed into each other become
   one surface. That skin is then reduced to its share of the triangle budget.
3. Claws come back as three-sided blades, so the hands and feet keep their outline.
4. Every vertex takes its bones from the nearest face of LOD0 in its own group (two bones at most;
   ribs, jaw and hanging rags ride with the bone they hang from, so nothing tears at range).
5. A face is fullbright when what LOD0 shows there, seen from outside, is the furnace: the furnace
   becomes a flat bright face behind dark bars.
6. New small sheets are laid out (hide and glow) and painted from LOD0 by looking at it from outside
   along each face's normal, so the paint is what a camera at that distance sees.
"""
import math
import os

import bmesh
import bpy
import numpy as np
from mathutils import Vector
from mathutils.bvhtree import BVHTree

GROUPS = [
    ("trunk", ("bricho", "zebra", "hrudnik", "ded_rib_l", "ded_rib_r", "lrameno", "prameno", "lzadek", "pzadek")),
    ("head", ("krk", "hlava", "ded_jaw", "ded_hang2")),
    ("arm_l", ("lbiceps", "lloket", "lruka")),
    ("arm_r", ("pbiceps", "ploket", "pruka")),
    ("leg_l", ("lstehno", "lholen", "lchodidlo", "lprsty")),
    ("leg_r", ("pstehno", "pholen", "pchodidlo", "pprsty")),
    ("tail", ("ded_tail1", "ded_tail2", "ded_tail3")),
    ("hang", ("ded_hang1",)),
]
# far LODs: these bones ride with the bone they hang from
MERGE_BONE = {"ded_rib_l": "hrudnik", "ded_rib_r": "hrudnik", "ded_hang2": "krk", "ded_jaw": "hlava",
              "lrameno": "hrudnik", "prameno": "hrudnik", "lzadek": "bricho", "pzadek": "bricho"}
CLAW_BONES = ("lruka", "pruka", "lprsty", "pprsty")


def _mesh_object(name, verts, faces, collection=None):
    me = bpy.data.meshes.new(name)
    me.from_pydata([tuple(v) for v in verts], [], [tuple(int(i) for i in f) for f in faces])
    me.update()
    ob = bpy.data.objects.new(name, me)
    (collection or bpy.context.scene.collection).objects.link(ob)
    return ob


def _evaluated_mesh(ob):
    bpy.context.view_layer.update()
    deps = bpy.context.evaluated_depsgraph_get()
    return bpy.data.meshes.new_from_object(ob.evaluated_get(deps), depsgraph=deps)


def _remesh(verts, faces, voxel, target_tris, name, crumb=0.04):
    """One closed skin over the soup, then reduced to target_tris. Returns (verts, tris) arrays."""
    ob = _mesh_object(name + "_src", verts, faces)
    # The skin needs closed pieces to know inside from outside. D's limb tubes are open at both ends
    # (they slide inside the trunk), and a sheet of hide has no inside at all: weld what touches, cap
    # every open loop, turn the faces outward, and give whatever is still open a wall to stand on.
    bm = bmesh.new()
    bm.from_mesh(ob.data)
    bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=0.002)
    edges = [e for e in bm.edges if e.is_boundary]
    if edges:
        bmesh.ops.holes_fill(bm, edges=edges, sides=0)
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    still_open = sum(1 for e in bm.edges if e.is_boundary)
    bm.to_mesh(ob.data)
    bm.free()
    if still_open:
        wall = ob.modifiers.new("wall", "SOLIDIFY")
        wall.thickness = 1.6 * voxel
        wall.offset = -1.0
    mod = ob.modifiers.new("skin", "REMESH")
    mod.mode = "VOXEL"
    mod.voxel_size = voxel
    mod.adaptivity = 0.0
    me = _evaluated_mesh(ob)
    bpy.data.objects.remove(ob)
    if len(me.polygons) == 0:
        bpy.data.meshes.remove(me)
        return None
    # crumbs: a skin can come out with small loose islands (a rib end, a fold of hide). Each costs
    # triangles the reduction cannot take away, so only islands with a fair share of the faces stay.
    bm = bmesh.new()
    bm.from_mesh(me)
    bm.faces.ensure_lookup_table()
    seen, islands = set(), []
    for f0 in bm.faces:
        if f0.index in seen:
            continue
        stack, isl = [f0], []
        seen.add(f0.index)
        while stack:
            f_ = stack.pop()
            isl.append(f_)
            for e in f_.edges:
                for g in e.link_faces:
                    if g.index not in seen:
                        seen.add(g.index)
                        stack.append(g)
        islands.append(isl)
    if len(islands) > 1:
        most = max(len(i_) for i_ in islands)
        drop = [f_ for isl in islands if len(isl) < max(crumb * most, 12) for f_ in isl]
        if drop:
            bmesh.ops.delete(bm, geom=drop, context="FACES")
    bm.to_mesh(me)
    bm.free()
    ob2 = bpy.data.objects.new(name + "_skin", me)
    bpy.context.scene.collection.objects.link(ob2)
    tri = ob2.modifiers.new("tri", "TRIANGULATE")
    n_now = sum(len(p.vertices) - 2 for p in me.polygons)
    dec = ob2.modifiers.new("less", "DECIMATE")
    dec.decimate_type = "COLLAPSE"
    dec.ratio = min(1.0, target_tris / max(1, n_now))
    dec.use_collapse_triangulate = True
    me2 = _evaluated_mesh(ob2)
    bpy.data.objects.remove(ob2)
    bpy.data.meshes.remove(me)
    v = np.array([p.co[:] for p in me2.vertices], dtype=np.float64)
    f = []
    for p in me2.polygons:
        idx = list(p.vertices)
        for k in range(1, len(idx) - 1):
            f.append((idx[0], idx[k], idx[k + 1]))
    bpy.data.meshes.remove(me2)
    return v, np.array(f, dtype=np.int64).reshape(-1, 3)


def _blade(pts):
    """A claw piece -> a three-sided blade (4 vertices, 4 triangles with its base): base round the thick end, tip at the far end."""
    c = pts.mean(axis=0)
    _u, _s, vt = np.linalg.svd(pts - c, full_matrices=False)
    axis = vt[0]
    t = (pts - c) @ axis
    lo, hi = t.min(), t.max()
    # the thick end is the base
    near = pts[t < lo + 0.25 * (hi - lo)]
    far = pts[t > hi - 0.25 * (hi - lo)]
    r_near = np.linalg.norm((near - near.mean(axis=0)) - np.outer((near - near.mean(axis=0)) @ axis, axis), axis=1).mean()
    r_far = np.linalg.norm((far - far.mean(axis=0)) - np.outer((far - far.mean(axis=0)) @ axis, axis), axis=1).mean()
    if r_far > r_near:
        axis, lo, hi, near, far, r_near = -axis, -hi, -lo, far, near, r_far
        t = -t
    base_c = near.mean(axis=0)
    tip = pts[np.argmax(t)]
    u = np.cross(axis, np.array([0.0, 0.0, 1.0]))
    if np.linalg.norm(u) < 0.2:
        u = np.cross(axis, np.array([1.0, 0.0, 0.0]))
    u /= np.linalg.norm(u)
    w = np.cross(axis, u)
    r = max(0.18, 1.5 * r_near)
    base = [base_c + r * (math.cos(a) * u + math.sin(a) * w) for a in (0.0, 2.094, 4.189)]
    verts = np.array(base + [tip])
    tris = np.array([(0, 1, 3), (1, 2, 3), (2, 0, 3), (0, 2, 1)])
    # outward
    cen = verts.mean(axis=0)
    for k in range(len(tris)):
        a, b, c3 = verts[tris[k]]
        if np.dot(np.cross(b - a, c3 - a), a - cen) < 0:
            tris[k] = tris[k][::-1]
    return verts, tris


def _pixels(img):
    w, h = img.size
    buf = np.empty(w * h * 4, dtype=np.float32)
    img.pixels.foreach_get(buf)
    return buf.reshape(h, w, 4)[:, :, :3]


def _save_png(path, rgb):
    h, w, _ = rgb.shape
    img = bpy.data.images.new(os.path.basename(path), w, h, alpha=False)
    buf = np.ones((h, w, 4), dtype=np.float32)
    buf[:, :, :3] = np.clip(rgb, 0.0, 1.0)
    img.pixels.foreach_set(buf.ravel())
    img.filepath_raw = path
    img.file_format = "PNG"
    img.save()
    bpy.data.images.remove(img)


def build(lod0, skin, glow_slots, root, height, tag, out_dir, texdir, manifest, cfg, lod_collection, say, warn, furnace=None,
          glow_grow=(0, 0, 0), glow_gain=1.0, drop_bones=(), glow_votes=2):
    mesh0 = lod0.data
    mesh0.calc_loop_triangles()
    P = skin.P
    names = list(skin.names)
    main = np.array([names[k] for k in skin.main()])
    # ---- LOD0 as triangles, with what the bake needs -------------------------------------------------
    lt = mesh0.loop_triangles
    T = np.array([t.vertices[:] for t in lt], dtype=np.int64)
    TL = np.array([t.loops[:] for t in lt], dtype=np.int64)
    Tmat = np.array([t.material_index for t in lt], dtype=np.int64)
    uv_all = np.empty(len(mesh0.loops) * 2, dtype=np.float32)
    mesh0.uv_layers.active.data.foreach_get("uv", uv_all)
    uv_all = uv_all.reshape(-1, 2)
    Tuv = uv_all[TL]                                  # (nt, 3, 2)
    Tglow = np.isin(Tmat, list(glow_slots))
    sheets = {}
    for k, slot in enumerate(lod0.material_slots):
        stem = slot.material.get("ud_stem")
        path = os.path.join(texdir, stem + ".png") if texdir and stem else None
        if path and os.path.isfile(path):
            img = bpy.data.images.load(path)
            sheets[k] = _pixels(img).copy()
            bpy.data.images.remove(img)
        else:
            warn("far LODs: no sheet for material %s in %s; painted grey" % (slot.material.name, texdir))
            sheets[k] = np.full((4, 4, 3), 0.4, dtype=np.float32)
    # ---- pieces and groups ---------------------------------------------------------------------------
    piece = skin.piece
    group_of_bone = {b: g for g, bones in GROUPS for b in bones}
    piece_ids = np.unique(piece)
    info = {}
    for pid in piece_ids:
        members = np.nonzero(piece == pid)[0]
        q = P[members]
        c = q - q.mean(axis=0)
        ext = np.sort(np.ptp(c @ np.linalg.svd(c, full_matrices=False)[2].T, axis=0))[::-1] if len(q) >= 3 else np.zeros(3)
        bones, counts = np.unique(main[members], return_counts=True)
        top = bones[np.argmax(counts)]
        tris = np.nonzero(piece[T[:, 0]] == pid)[0]
        info[pid] = {"verts": members, "tris": tris, "ext": ext, "bone": top, "group": group_of_bone.get(top, "trunk"),
                     "glow": bool(Tglow[tris].mean() > 0.5) if len(tris) else False,
                     "stick": bool(ext[0] > 6.0 and ext[1] < 0.9),
                     "claw": bool(top in CLAW_BONES and ext[0] > 1.2 and ext[0] > 2.2 * max(ext[1], 0.05) and len(members) < 80)}
    # ---- per-group search trees over LOD0 (everything but sticks: small parts are painted on) --------
    trees = {}
    for g, _b in GROUPS:
        tri_idx = np.concatenate([info[p]["tris"] for p in piece_ids if info[p]["group"] == g and not info[p]["stick"]] or [np.zeros(0, dtype=np.int64)])
        if len(tri_idx) == 0:
            continue
        trees[g] = (BVHTree.FromPolygons([tuple(v) for v in P], [tuple(int(i) for i in T[k]) for k in tri_idx]), tri_idx)
    W = skin.W
    out_objs, out_info, out_manifest = [], [], []
    for li, (frac, voxel, budget, hide_px, glow_px, min_piece) in enumerate(cfg, start=1):
        claws = [p for p in piece_ids if info[p]["claw"] and info[p]["ext"][0] >= (1.2 if li == 1 else (2.0 if li == 2 else 3.0))]
        claw_tris = 4 * len(claws)
        body_budget = max(120, budget - claw_tris)
        # ---- 1, 2: one closed skin per group ------------------------------------------------------------
        soups = {}
        for g, _b in GROUPS:
            keep = [p for p in piece_ids if info[p]["group"] == g and not info[p]["stick"] and not info[p]["claw"]
                    and info[p]["bone"] not in drop_bones
                    and (info[p]["ext"][0] >= min_piece or len(info[p]["tris"]) > 300)]
            if not keep:
                continue
            tri_idx = np.concatenate([info[p]["tris"] for p in keep])
            used = np.unique(T[tri_idx])
            remap = -np.ones(len(P), dtype=np.int64)
            remap[used] = np.arange(len(used))
            area = 0.5 * np.linalg.norm(np.cross(P[T[tri_idx, 1]] - P[T[tri_idx, 0]], P[T[tri_idx, 2]] - P[T[tri_idx, 0]]), axis=1).sum()
            soups[g] = (P[used], remap[T[tri_idx]], area)
        total_w = sum(a ** 0.7 for _v, _f, a in soups.values())
        scale = 1.0
        for _try in range(5):
            vs, fs, gs = [], [], []
            base = 0
            for g, (v, f, area) in soups.items():
                share = max(20 if li == len(cfg) else 48, int(round(scale * body_budget * (area ** 0.7) / total_w)))
                res = _remesh(v, f, voxel, share, "far%d_%s" % (li, g), 0.10 if li == len(cfg) else 0.04)
                if res is None:
                    warn("far LOD %d: group %s gave no skin at %.2f m voxels" % (li, g, voxel))
                    continue
                rv, rf = res
                vs.append(rv)
                fs.append(rf + base)
                gs += [g] * len(rv)
                base += len(rv)
            got = sum(len(f_) for f_ in fs)
            if got <= body_budget * 1.03:
                break
            scale *= 0.96 * body_budget / got
        # ---- 3: claws as blades ---------------------------------------------------------------------------
        for p in claws:
            bv, bt = _blade(P[info[p]["verts"]])
            vs.append(bv)
            fs.append(bt + base)
            gs += [info[p]["group"]] * len(bv)
            base += len(bv)
        V = np.concatenate(vs)
        F = np.concatenate(fs)
        G = np.array(gs)
        # trim to the budget if the skins overshot (the last LOD must stay under the shadow limit)
        col = lod_collection(root, "lod%d" % li, round(frac * height, 2), {"lodnoshadow": "1"} if li < len(cfg) else None)
        ob = _mesh_object("dedushka_lod%d" % li, V, F, col)
        me = ob.data
        # ---- 4: bones --------------------------------------------------------------------------------------
        bone_w = {}
        for vi in range(len(V)):
            g = G[vi]
            if g not in trees:
                g = "trunk"
            tree, tri_idx = trees[g]
            loc, _n, fi, _d = tree.find_nearest(Vector(V[vi]))
            if loc is None:
                continue
            tv = T[tri_idx[fi]]
            a, b, c = P[tv[0]], P[tv[1]], P[tv[2]]
            # barycentric of loc
            v0, v1, v2 = b - a, c - a, np.array(loc) - a
            d00, d01, d11, d20, d21 = v0 @ v0, v0 @ v1, v1 @ v1, v2 @ v0, v2 @ v1
            den = d00 * d11 - d01 * d01
            if abs(den) < 1e-12:
                bw = (1.0, 0.0, 0.0)
            else:
                bv_ = (d11 * d20 - d01 * d21) / den
                bw_ = (d00 * d21 - d01 * d20) / den
                bw = (1.0 - bv_ - bw_, bv_, bw_)
            w = bw[0] * W[tv[0]] + bw[1] * W[tv[1]] + bw[2] * W[tv[2]]
            acc = {}
            for k in np.nonzero(w > 0.01)[0]:
                nm = MERGE_BONE.get(names[k], names[k])
                acc[nm] = acc.get(nm, 0.0) + float(w[k])
            top = sorted(acc.items(), key=lambda kv: -kv[1])[:2]
            s = sum(x for _n2, x in top)
            for nm, x in top:
                if x / s >= 0.08:
                    bone_w.setdefault(nm, []).append((vi, x / s))
        for nm, lst in bone_w.items():
            vg = ob.vertex_groups.new(name=nm)
            for vi, x in lst:
                vg.add([vi], x, "REPLACE")
        if "bricho" not in ob.vertex_groups:
            ob.vertex_groups.new(name="bricho")
        # ---- 5: which faces are furnace -----------------------------------------------------------------------
        me.calc_loop_triangles()
        for p in me.polygons:
            p.use_smooth = True
        me.update()
        vn = np.array([v.normal[:] for v in me.vertices], dtype=np.float64)
        off = max(0.8, 2.5 * voxel)

        def look(pos, nrm, g):
            """What LOD0 shows at this point of the far skin, seen from outside: (triangle index, location) or None."""
            tree, tri_idx = trees[g if g in trees else "trunk"]
            o = Vector(pos + off * nrm)
            loc, _n, fi, _d = tree.ray_cast(o, Vector(-nrm), 2.0 * off + 1.5)
            if loc is None:
                loc, _n, fi, _d = tree.find_nearest(Vector(pos))
                if loc is None:
                    return None
            return int(tri_idx[fi]), np.array(loc)

        face_glow = np.zeros(len(F), dtype=bool)
        for fi in range(len(F)):
            a, b, c = V[F[fi, 0]], V[F[fi, 1]], V[F[fi, 2]]
            n = np.cross(b - a, c - a)
            ln = np.linalg.norm(n)
            if ln < 1e-9:
                continue
            n /= ln
            cen = (a + b + c) / 3.0
            votes = 0
            samples = [cen, 0.6 * a + 0.2 * b + 0.2 * c, 0.2 * a + 0.6 * b + 0.2 * c, 0.2 * a + 0.2 * b + 0.6 * c]
            for s_ in samples:
                hit = look(s_, n, G[F[fi, 0]])
                if hit is not None and Tglow[hit[0]]:
                    votes += 1
            face_glow[fi] = votes >= glow_votes
        for _ring in range(int(glow_grow[li - 1]) if li - 1 < len(glow_grow) else 0):
            hot_v = np.zeros(len(V), dtype=bool)
            hot_v[F[face_glow].ravel()] = True
            face_glow |= hot_v[F].any(axis=1) & (G[F[:, 0]] == "trunk")
        # ---- materials -------------------------------------------------------------------------------------------
        stems = ("d%s_f%dh" % (tag, li), "d%s_f%dg" % (tag, li))
        for k, stem in enumerate(stems):
            mat = bpy.data.materials.new(stem + ("_glow" if k else ""))
            mat["p3d_texture"] = "uddedushka\\%s.paa" % stem
            if k:
                mat["p3d_fullbright"] = 1
            me.materials.append(mat)
        me.polygons.foreach_set("material_index", face_glow.astype(np.int32))
        me.update()
        # ---- 6: lay out and paint -----------------------------------------------------------------------------------
        uvl = me.uv_layers.new(name="UVMap")
        bpy.context.view_layer.objects.active = ob
        for o_ in bpy.context.view_layer.objects:
            o_.select_set(o_ is ob)
        for slot in (0, 1):
            if not (face_glow == bool(slot)).any():
                continue
            bpy.ops.object.mode_set(mode="EDIT")
            bm = bmesh.from_edit_mesh(me)
            for f_ in bm.faces:
                f_.select = (f_.material_index == slot)
            bmesh.update_edit_mesh(me)
            px = glow_px if slot else hide_px
            bpy.ops.uv.smart_project(angle_limit=math.radians(72.0), island_margin=3.0 / px, area_weight=0.0, correct_aspect=True,
                                     scale_to_bounds=False)
            bpy.ops.object.mode_set(mode="OBJECT")
        uv = np.empty(len(me.loops) * 2, dtype=np.float32)
        me.uv_layers.active.data.foreach_get("uv", uv)
        uv = uv.reshape(-1, 2)
        loop_start = np.array([p.loop_start for p in me.polygons])
        poly_v = np.array([p.vertices[:] for p in me.polygons])
        counts = {}
        for slot in (0, 1):
            faces = np.nonzero(face_glow == bool(slot))[0]
            px = glow_px if slot else hide_px
            img = np.zeros((px, px, 3), dtype=np.float32)
            done = np.zeros((px, px), dtype=bool)
            # texels, by triangle
            tx, ty, tpos, tnrm, tg = [], [], [], [], []
            for fi in faces:
                l0 = loop_start[fi]
                tri_uv = uv[l0:l0 + 3].astype(np.float64) * px
                vi = poly_v[fi]
                x0, x1 = int(math.floor(tri_uv[:, 0].min())) - 1, int(math.ceil(tri_uv[:, 0].max())) + 1
                y0, y1 = int(math.floor(tri_uv[:, 1].min())) - 1, int(math.ceil(tri_uv[:, 1].max())) + 1
                x0, y0, x1, y1 = max(x0, 0), max(y0, 0), min(x1, px - 1), min(y1, px - 1)
                if x1 < x0 or y1 < y0:
                    continue
                gx, gy = np.meshgrid(np.arange(x0, x1 + 1), np.arange(y0, y1 + 1))
                pxs = np.stack([gx.ravel() + 0.5, gy.ravel() + 0.5], axis=1)
                a, b, c = tri_uv
                m = np.array([[b[0] - a[0], c[0] - a[0]], [b[1] - a[1], c[1] - a[1]]])
                det = m[0, 0] * m[1, 1] - m[0, 1] * m[1, 0]
                if abs(det) < 1e-9:
                    continue
                r = pxs - a
                wb = (r[:, 0] * m[1, 1] - r[:, 1] * m[0, 1]) / det
                wc = (r[:, 1] * m[0, 0] - r[:, 0] * m[1, 0]) / det
                wa = 1.0 - wb - wc
                eps = 1.2 / max(1.0, math.sqrt(abs(det)))          # a texel of slack past the edges
                ins = (wa >= -eps) & (wb >= -eps) & (wc >= -eps)
                if not ins.any():
                    continue
                wa, wb, wc = np.clip(wa[ins], 0, 1), np.clip(wb[ins], 0, 1), np.clip(wc[ins], 0, 1)
                s = wa + wb + wc
                wa, wb, wc = wa / s, wb / s, wc / s
                pos = wa[:, None] * V[vi[0]] + wb[:, None] * V[vi[1]] + wc[:, None] * V[vi[2]]
                nrm = wa[:, None] * vn[vi[0]] + wb[:, None] * vn[vi[1]] + wc[:, None] * vn[vi[2]]
                ln = np.linalg.norm(nrm, axis=1)
                ln[ln < 1e-9] = 1.0
                nrm /= ln[:, None]
                tx.append(gx.ravel()[ins])
                ty.append(gy.ravel()[ins])
                tpos.append(pos)
                tnrm.append(nrm)
                tg += [G[vi[0]]] * int(ins.sum())
            if not tx:
                continue
            tx, ty = np.concatenate(tx), np.concatenate(ty)
            tpos, tnrm = np.concatenate(tpos), np.concatenate(tnrm)
            hit_tri = -np.ones(len(tx), dtype=np.int64)
            hit_loc = np.zeros((len(tx), 3))
            for k in range(len(tx)):
                hit = look(tpos[k], tnrm[k], tg[k])
                if hit is not None:
                    hit_tri[k] = hit[0]
                    hit_loc[k] = hit[1]
            ok = hit_tri >= 0
            ht = hit_tri[ok]
            a, b, c = P[T[ht, 0]], P[T[ht, 1]], P[T[ht, 2]]
            v0, v1, v2 = b - a, c - a, hit_loc[ok] - a
            d00, d01, d11 = (v0 * v0).sum(1), (v0 * v1).sum(1), (v1 * v1).sum(1)
            d20, d21 = (v2 * v0).sum(1), (v2 * v1).sum(1)
            den = d00 * d11 - d01 * d01
            den[np.abs(den) < 1e-12] = 1e-12
            bb = np.clip((d11 * d20 - d01 * d21) / den, 0, 1)
            cc = np.clip((d00 * d21 - d01 * d20) / den, 0, 1)
            aa = np.clip(1.0 - bb - cc, 0, 1)
            suv = aa[:, None] * Tuv[ht, 0] + bb[:, None] * Tuv[ht, 1] + cc[:, None] * Tuv[ht, 2]
            colour = np.zeros((len(ht), 3), dtype=np.float32)
            for mslot, sheet in sheets.items():
                sel = Tmat[ht] == mslot
                if not sel.any():
                    continue
                sh, sw, _ = sheet.shape
                ix = np.clip((suv[sel, 0] % 1.0) * sw, 0, sw - 1).astype(np.int64)
                iy = np.clip((suv[sel, 1] % 1.0) * sh, 0, sh - 1).astype(np.int64)
                colour[sel] = sheet[iy, ix]
            src_glow = Tglow[ht]
            if slot:
                # on the fullbright face: fire at its own value (lifted a little, it is small at range),
                # whatever stands before the fire as a dark bar
                colour[src_glow] = np.clip(colour[src_glow] * glow_gain, 0, 1)
                # (first try: the bars at 0.38 of their paint. In game at dusk from 1.85 km A's rib gates,
                # whose paint carries yellow furnace light, then read as olive bars brighter than the
                # fire between them. They are now a dark warm brown: a silhouette before the fire.)
                colour[~src_glow] = colour[~src_glow] * 0.2 * np.array([1.0, 0.72, 0.5], dtype=np.float32)
            else:
                # on the lit face a patch of fire would turn black at night: keep it as warm paint, dimmed
                colour[src_glow] *= 0.55
            img[ty[ok], tx[ok]] = colour
            done[ty[ok], tx[ok]] = True
            mean = img[done].mean(axis=0) if done.any() else np.array([0.3, 0.25, 0.2])
            # spread the edge colour into the gutters, then fill what is left with the mean
            for _ in range(6):
                if done.all():
                    break
                acc = np.zeros_like(img)
                cnt = np.zeros((px, px), dtype=np.float32)
                for dy, dx in ((0, 1), (0, -1), (1, 0), (-1, 0)):
                    sh_img = np.roll(img, (dy, dx), axis=(0, 1))
                    sh_done = np.roll(done, (dy, dx), axis=(0, 1))
                    acc += sh_img * sh_done[:, :, None]
                    cnt += sh_done
                grow = (~done) & (cnt > 0)
                img[grow] = acc[grow] / cnt[grow][:, None]
                done |= grow
            img[~done] = mean
            path = os.path.join(out_dir, stems[slot] + ".png")
            _save_png(path, img)
            out_manifest.append({"png": path, "paa": stems[slot] + ".paa", "material": me.materials[slot].name, "size": [px, px],
                                 "fullbright": bool(slot), "far_lod": li})
            counts["glow" if slot else "hide"] = {"faces": int(len(faces)), "texels": int(len(tx)), "sheet_px": px}
        if not face_glow.any():
            warn("far LOD %d has no furnace face" % li)
        out_objs.append(ob)
        row = {"lod": li, "resolution": round(frac * height, 2), "triangles": int(len(F)), "vertices": int(len(V)), "voxel_m": voxel,
               "claw_blades": len(claws), "furnace_faces": int(face_glow.sum()), "sheets": counts,
               "bones": sorted(vg.name for vg in ob.vertex_groups)}
        out_info.append(row)
        say("far LOD %d (resolution %.1f): %d triangles, %d vertices, %d claw blades, %d furnace faces, sheets %s"
            % (li, frac * height, len(F), len(V), len(claws), int(face_glow.sum()), counts))
    return out_objs, out_info, out_manifest
