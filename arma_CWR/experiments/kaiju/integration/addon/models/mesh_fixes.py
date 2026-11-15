"""Mesh changes of the finishing round that keep every UV (called by finish_model.py).

open_wound: widens an opening of the trunk (D's furnace slot, the split in its back) by drawing the
trunk's surface away from the middle of the opening, in the plane of the opening, with a smooth
fall-off. Everything of the trunk inside the fall-off moves by the same rule (hide, the bowl behind
the opening, the ribs across it, the lip, the coals), so nothing comes apart, and no vertex, face or
UV is added or removed: the bowl's fire is stretched a little, the hide beside the wound is drawn
together. What D is does not change; how much fire shows does.
"""
import numpy as np


def _smooth(t):
    t = np.clip(t, 0.0, 1.0)
    return t * t * (3.0 - 2.0 * t)


def boundary_loops(mesh, verts_mask, skip_materials=()):
    """Open edge loops among the faces whose vertices are all in verts_mask (faces of skip_materials,
    the fullbright ones, do not count: a bowl welded to the rim of a hole does not close the hole)."""
    count = {}
    for p in mesh.polygons:
        vs = list(p.vertices)
        if p.material_index in skip_materials or not all(verts_mask[v] for v in vs):
            continue
        for k in range(len(vs)):
            a, b = vs[k], vs[(k + 1) % len(vs)]
            key = (min(a, b), max(a, b))
            count[key] = count.get(key, 0) + 1
    adj = {}
    for (a, b), n in count.items():
        if n == 1:
            adj.setdefault(a, []).append(b)
            adj.setdefault(b, []).append(a)
    seen, loops = set(), []
    for s in adj:
        if s in seen:
            continue
        stack, loop = [s], []
        while stack:
            v = stack.pop()
            if v in seen:
                continue
            seen.add(v)
            loop.append(v)
            stack.extend(adj[v])
        loops.append(np.array(loop))
    return loops


def open_wound(P, loop, affected, body_centre, widen, heighten, reach=2.0, depth=(6.0, 10.0)):
    """Returns new positions. loop: vertex indices of the opening's rim. affected: bool mask of the
    vertices that may move. widen / heighten: the share by which the opening grows across and along.
    reach: the fall-off ends at this many half-sizes from the middle. depth: behind the opening's
    plane the pull is whole down to depth[0] metres and gone by depth[1]."""
    rim = P[loop]
    c = rim.mean(axis=0)
    _u, _s, vt = np.linalg.svd(rim - c, full_matrices=False)
    n = vt[2]
    if np.dot(n, c - body_centre) < 0:
        n = -n
    up = np.array([0.0, 0.0, 1.0])
    e_b = up - np.dot(up, n) * n
    e_b /= np.linalg.norm(e_b)
    e_a = np.cross(n, e_b)
    a_rim, b_rim = (rim - c) @ e_a, (rim - c) @ e_b
    ax, by = 0.5 * (a_rim.max() - a_rim.min()), 0.5 * (b_rim.max() - b_rim.min())
    # centre the frame on the middle of the rim's box, not its centroid
    c = c + 0.5 * (a_rim.max() + a_rim.min()) * e_a + 0.5 * (b_rim.max() + b_rim.min()) * e_b
    q = P - c
    a, b, d = q @ e_a, q @ e_b, q @ n
    r = np.sqrt((a / ax) ** 2 + (b / by) ** 2)
    f = 1.0 - _smooth((r - 1.0) / (reach - 1.0))
    g = _smooth((d + depth[1]) / (depth[1] - depth[0]))
    w = f * g * affected
    a2 = a * (1.0 + widen * w)
    b2 = b * (1.0 + heighten * w)
    out = c + np.outer(a2, e_a) + np.outer(b2, e_b) + np.outer(d, n)
    moved = np.linalg.norm(out - P, axis=1)
    info = {"centre_file": [round(float(c[0]), 2), round(float(c[2]), 2), round(float(c[1]), 2)], "half_width_m": round(float(ax), 2),
            "half_height_m": round(float(by), 2), "widen": widen, "heighten": heighten, "vertices_moved": int((moved > 0.01).sum()),
            "furthest_m": round(float(moved.max()), 2), "rim_vertices": int(len(loop))}
    return out, info
