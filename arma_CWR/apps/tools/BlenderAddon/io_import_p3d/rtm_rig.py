"""Stock man skeleton for RTM work: hierarchy, joint pivots, rest layout.

RTM files carry no skeleton: each bone matrix is absolute (rest file space to
posed file space), so the engine needs no hierarchy and no joint positions.
An animator does, to rotate a forearm about the elbow. The pivots are
derived from the stock RTMs themselves: for a child bone c and its parent p,
the joint is the rest-space point that both matrices move to the same place
in every pose, ``M_p(f) x = M_c(f) x``. ``derive_rest_joints`` solves that in
least squares over all rigid stock phases and reports the residual (a real
joint has a residual of a few millimetres).

The derived numbers come from Bohemia data, so they are written to a local
JSON file (``save_rest_json``) instead of being kept in this module.

Coordinates here are RTM file space (X, Y up, Z; stock rest faces +X, the
man's left is +Z). No bpy import.
"""

import json
import math
import os

try:
    from .rtm_io import STOCK_MAN_BONES, mat_det, mat_orthonormal_error, read_rtm
except ImportError:  # plain-Python use (tests) outside the add-on package
    from rtm_io import STOCK_MAN_BONES, mat_det, mat_orthonormal_error, read_rtm

# Parent of each stock bone. bricho (belly) is the root; the pelvis halves hang
# from it. pzadek and lzadek move as one in every stock RTM, as do the shoulders
# with the chest, so their pivots are weakly defined (see derive_rest_joints).
STOCK_PARENTS = {
    "bricho": None,
    "zebra": "bricho",
    "hrudnik": "zebra",
    "krk": "hrudnik",
    "hlava": "krk",
    "lrameno": "hrudnik",
    "prameno": "hrudnik",
    "lbiceps": "lrameno",
    "pbiceps": "prameno",
    "lloket": "lbiceps",
    "ploket": "pbiceps",
    "lruka": "lloket",
    "pruka": "ploket",
    "lzadek": "bricho",
    "pzadek": "bricho",
    "lstehno": "lzadek",
    "pstehno": "pzadek",
    "lholen": "lstehno",
    "pholen": "pstehno",
    "lchodidlo": "lholen",
    "pchodidlo": "pholen",
    "lprsty": "lchodidlo",
    "pprsty": "pchodidlo",
    "zbran": "pruka",  # weapon: no fixed joint in the stock data
    "roura": "hrudnik",  # launcher on the back: no fixed joint either
}

# Bones without a meaningful fixed joint to their parent.
FREE_BONES = ("zbran", "roura")

# Which bones the skin uses (the creature has vertices on these 23).
SKIN_BONES = tuple(b for b in STOCK_MAN_BONES if b not in FREE_BONES)


def bones_parent_first(bones=STOCK_MAN_BONES, parents=None):
    parents = parents or STOCK_PARENTS
    out = []
    pending = list(bones)
    while pending:
        progressed = False
        for b in list(pending):
            p = parents.get(b)
            if p is None or p in out or p not in bones:
                out.append(b)
                pending.remove(b)
                progressed = True
        if not progressed:
            raise ValueError(f"cycle in parents: {pending}")
    return out


def _solve3(a, b):
    """Solve a 3x3 system by Gaussian elimination with partial pivoting."""
    m = [list(a[i]) + [b[i]] for i in range(3)]
    for c in range(3):
        piv = max(range(c, 3), key=lambda r: abs(m[r][c]))
        if abs(m[piv][c]) < 1e-18:
            raise ValueError("singular system")
        m[c], m[piv] = m[piv], m[c]
        for r in range(3):
            if r != c:
                f = m[r][c] / m[c][c]
                for k in range(c, 4):
                    m[r][k] -= f * m[c][k]
    return [m[i][3] / m[i][i] for i in range(3)]


def _rigid(m, tol):
    return mat_det(m) > 0 and mat_orthonormal_error(m) < tol


def solve_pivot(pairs, prior=(0.0, 0.0, 0.0), reg=1e-9):
    """Least-squares fixed point of pairs [(M_parent, M_child)] of 12-float matrices.

    Minimises sum |M_p x - M_c x|^2 + lam * |x - prior|^2 with lam a tiny
    fraction (reg) of the mean diagonal of A^T A. It only matters when parent
    and child always move together (then the pivot is undefined and x falls
    back to the prior). Returns
    (x, rms residual in metres, condition hint = smallest pivot of A^T A / N).
    """
    ata = [[0.0] * 3 for _ in range(3)]
    atb = [0.0] * 3
    n = 0
    for mp, mc in pairs:
        # A = R_p - R_c (3x3, columns from the file layout), b = t_c - t_p
        a = [[mp[3 * j + i] - mc[3 * j + i] for j in range(3)] for i in range(3)]
        b = [mc[9 + i] - mp[9 + i] for i in range(3)]
        for r in range(3):
            for c in range(3):
                ata[r][c] += sum(a[k][r] * a[k][c] for k in range(3))
            atb[r] += sum(a[k][r] * b[k] for k in range(3))
        n += 1
    if n == 0:
        return list(prior), float("nan"), 0.0
    # relative regulariser: negligible bias for a real joint, prior wins when A^T A ~ 0
    lam = reg * (ata[0][0] + ata[1][1] + ata[2][2]) / 3 + 1e-12 * n
    lhs = [[ata[r][c] + (lam if r == c else 0.0) for c in range(3)] for r in range(3)]
    rhs = [atb[r] + lam * prior[r] for r in range(3)]
    x = _solve3(lhs, rhs)
    sq = 0.0
    for mp, mc in pairs:
        for i in range(3):
            d = sum((mp[3 * j + i] - mc[3 * j + i]) * x[j] for j in range(3)) - (mc[9 + i] - mp[9 + i])
            sq += d * d
    rms = math.sqrt(sq / (3 * n))
    # crude conditioning: smallest diagonal after elimination is not exposed by
    # _solve3, so report the smallest Gershgorin lower bound per frame instead
    cond = min(ata[i][i] - sum(abs(ata[i][j]) for j in range(3) if j != i) for i in range(3)) / n
    return x, rms, cond


def derive_rest_joints(anims, rigid_tol=1e-3, parents=None):
    """Derive joint pivots (rest file space) from RtmAnimation objects.

    Only phases where both matrices of a pair are rigid rotations (det > 0,
    orthonormal within ``rigid_tol``) are used. Returns a dict ready for
    ``save_rest_json``.
    """
    parents = parents or STOCK_PARENTS
    pairs = {b: [] for b in parents if parents[b]}
    files = 0
    for anim in anims:
        idx = {n.lower(): i for i, n in enumerate(anim.bone_names)}
        if not all(b in idx for b in STOCK_MAN_BONES):
            continue
        files += 1
        for fr in anim.frames:
            for child, parent in parents.items():
                if not parent:
                    continue
                mp = fr.matrices[idx[parent]]
                mc = fr.matrices[idx[child]]
                if _rigid(mp, rigid_tol) and _rigid(mc, rigid_tol):
                    pairs[child].append((mp, mc))
    pivots = {}
    residual = {}
    counts = {}
    for child in bones_parent_first(tuple(parents), parents):
        parent = parents[child]
        if not parent:
            continue
        prior = pivots.get(parent, (0.0, 0.0, 0.0))
        x, rms, _ = solve_pivot(pairs[child], prior=prior)
        pivots[child] = [round(v, 5) for v in x]
        residual[child] = round(rms, 5)
        counts[child] = len(pairs[child])
    return {
        "format": "ud-rtm-rest/1",
        "space": "RTM file space: X, Y up, Z; stock rest faces +X, left is +Z",
        "note": "derived from stock RTMs by rtm_rig.derive_rest_joints; Bohemia-derived, keep local",
        "files": files,
        "pivots": pivots,
        "residual_m": residual,
        "pairs_used": counts,
        "parents": {b: p for b, p in parents.items()},
    }


def derive_rest_joints_from_dir(folder, pattern_ext=".rtm"):
    anims = []
    for name in sorted(os.listdir(folder)):
        if name.lower().endswith(pattern_ext):
            try:
                anims.append(read_rtm(os.path.join(folder, name)))
            except Exception:  # noqa: BLE001 - skip unreadable files
                continue
    return derive_rest_joints(anims)


def save_rest_json(rest, path):
    with open(path, "w", encoding="utf-8") as f:
        json.dump(rest, f, indent=1, sort_keys=True)


def load_rest_json(path):
    with open(path, encoding="utf-8") as f:
        rest = json.load(f)
    if rest.get("format") != "ud-rtm-rest/1":
        raise ValueError(f"{path}: not a ud-rtm-rest/1 file")
    return rest


def man_foot_contacts(rest):
    """Ground contact points of each foot (ankle and toe joint) for rtm_engine.estimate_step."""
    pv = rest["pivots"]
    return {s: [(s + "chodidlo", pv[s + "chodidlo"]), (s + "prsty", pv[s + "prsty"])] for s in "lp"}


def _sub(a, b):
    return [a[i] - b[i] for i in range(3)]


def _add(a, b):
    return [a[i] + b[i] for i in range(3)]


def _norm(v):
    n = math.sqrt(sum(c * c for c in v))
    return [c / n for c in v] if n > 1e-12 else [0.0, 1.0, 0.0]


def _mul(v, s):
    return [c * s for c in v]


def rest_layout(rest):
    """Bone heads and tails in rest file space, for building an editable rig.

    head = the joint with the parent (bricho: the hip centre, the mean of the
    pelvis pivots); tail = the main child's joint, or a short stub for leaf
    bones. Only rotations about these heads depend on them: the deformation
    the engine sees is the absolute matrix, whatever the rest layout.
    """
    pv = rest["pivots"]
    hip = _mul(_add(pv["lzadek"], pv["pzadek"]), 0.5)
    heads = dict(pv)
    heads["bricho"] = hip
    # The pelvis halves always move together; start both at the hip centre.
    heads["lzadek"] = hip
    heads["pzadek"] = hip
    # Shoulders barely move against the chest: start them at the neck base.
    neck_base = pv["krk"]
    heads["lrameno"] = neck_base
    heads["prameno"] = neck_base
    heads["zbran"] = pv["pruka"]
    heads["roura"] = pv["hrudnik"]
    forward = [1.0, 0.0, 0.0]
    up = [0.0, 1.0, 0.0]
    tails = {
        "bricho": pv["zebra"], "zebra": pv["hrudnik"], "hrudnik": pv["krk"], "krk": pv["hlava"],
        "lrameno": pv["lbiceps"], "prameno": pv["pbiceps"],
        "lbiceps": pv["lloket"], "pbiceps": pv["ploket"],
        "lloket": pv["lruka"], "ploket": pv["pruka"],
        "lzadek": pv["lstehno"], "pzadek": pv["pstehno"],
        "lstehno": pv["lholen"], "pstehno": pv["pholen"],
        "lholen": pv["lchodidlo"], "pholen": pv["pchodidlo"],
        "lchodidlo": pv["lprsty"], "pchodidlo": pv["pprsty"],
    }
    tails["hlava"] = _add(heads["hlava"], _mul(_norm(_sub(heads["hlava"], heads["krk"])), 0.2))
    for s in "lp":
        tails[s + "ruka"] = _add(heads[s + "ruka"], _mul(_norm(_sub(heads[s + "ruka"], heads[s + "loket"])), 0.12))
        tails[s + "prsty"] = _add(heads[s + "prsty"], _mul(forward, 0.07))
    tails["zbran"] = _add(heads["zbran"], _mul(forward, 0.3))
    tails["roura"] = _add(heads["roura"], _mul(up, 0.3))
    out = {}
    for b in STOCK_MAN_BONES:
        h = heads[b]
        t = tails[b]
        if math.dist(h, t) < 1e-3:
            t = _add(h, _mul(up, 0.05))
        # Roll reference: limbs and spine bend about the man's side axis, so keep
        # the bone's Z towards the rest forward (+X); bones that point forward use up.
        d = _norm(_sub(t, h))
        roll_ref = up if abs(d[0]) > 0.7 else forward
        out[b] = {"head": list(h), "tail": list(t), "parent": STOCK_PARENTS[b], "roll_ref": roll_ref}
    return out
