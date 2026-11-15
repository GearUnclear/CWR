"""Numbers for the seams seen in motion, from a built P3D and its clips (no game, no Blender).

  python seam_metrics.py <dedushka.p3d> <anim dir> [--tag a|d] [--out report.txt]

For every clip, at 48 phases:
  hands in legs   vertices of each hand (claws included) that are INSIDE a leg tube or the pelvis
                  (winding number), and how deep the deepest one is
  hands in ground the lowest point of each hand
  roots           how far the innermost ring of each limb tube moves against the bone it is pushed
                  into (the pelvis for a thigh, the shoulder for an upper arm, the pelvis for the tail):
                  a root that moves is a root that comes out of the body
  collision       the lowest point of the Geometry LOD (men and tanks must pass under it)
"""
import argparse
import os

import numpy as np

import p3d_pose as pp
from comp_report import components

CLIPS = ["ded_idle", "ded_walk", "ded_turnl", "ded_turnr", "ded_roar", "ded_beamrise", "ded_beamhold", "ded_beamlower",
         "ded_stomp", "ded_swat", "ded_flinch", "ded_dormant"]
ROOTS = [("lstehno", "bricho"), ("pstehno", "bricho"), ("lbiceps", "lrameno"), ("pbiceps", "prameno"), ("ded_tail1", "bricho")]


def winding(q, tri):
    """Winding number of the closed triangle soup `tri` (m, 3, 3) round each point of q (n, 3)."""
    a = tri[None, :, 0, :] - q[:, None, :]
    b = tri[None, :, 1, :] - q[:, None, :]
    c = tri[None, :, 2, :] - q[:, None, :]
    la, lb, lc = np.linalg.norm(a, axis=2), np.linalg.norm(b, axis=2), np.linalg.norm(c, axis=2)
    num = np.einsum("nmk,nmk->nm", a, np.cross(b, c))
    den = la * lb * lc + np.einsum("nmk,nmk->nm", a, b) * lc + np.einsum("nmk,nmk->nm", b, c) * la + np.einsum("nmk,nmk->nm", c, a) * lb
    return np.abs(np.arctan2(num, den).sum(axis=1) / (2.0 * np.pi))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("p3d")
    ap.add_argument("anim")
    ap.add_argument("--out", default="")
    ap.add_argument("--phases", type=int, default=48)
    a = ap.parse_args()
    model = pp.load(a.p3d)
    lod = model.visual[0]
    comp = components(len(lod.points), lod.tris)
    tri_comp = comp[lod.tris[:, 0]]
    pose0 = pp.Pose(lod, pp.read_clip(os.path.join(a.anim, "ded_idle.rtm")))
    main = np.array(pose0.main_bone())
    bone_ix = {b: k for k, b in enumerate(pose0.bones)}

    def pieces_of(bones, share=0.15):
        out = []
        for c in np.unique(comp):
            members = comp == c
            if np.isin(main[members], bones).mean() >= share and members.sum() >= 40:
                out.append(c)
        return out

    legs = {s: pieces_of([s + "stehno", s + "holen"]) for s in "lp"}
    trunk = pieces_of(["bricho"], 0.1)
    hands = {s: np.nonzero(main == s + "ruka")[0] for s in "lp"}
    geo = model.special.get("geometry")
    rest = {}
    rp = os.path.join(os.path.dirname(a.p3d), "dedushka_rest.json")
    if os.path.isfile(rp):
        import json
        rest = json.load(open(rp, encoding="utf-8"))
    lines = ["%s" % a.p3d, "leg tubes: %s, trunk pieces: %s, hand vertices: %s" % (
        {s: len(v) for s, v in legs.items()}, len(trunk), {s: len(v) for s, v in hands.items()})]
    # innermost ring of each root: the vertices of the child's tube that carry the most parent weight
    roots = {}
    for child, parent in ROOTS:
        if child not in bone_ix or child not in lod.sel:
            continue
        tube = pieces_of([child], 0.1)
        members = np.isin(comp, tube)
        cw = np.array([lod.sel[child].get(i, 0.0) for i in range(len(lod.points))])
        pw = np.array([lod.sel.get(parent, {}).get(i, 0.0) for i in range(len(lod.points))])
        cand = np.nonzero(members & ((cw + pw) > 0.5))[0]
        if len(cand) == 0:
            continue
        # the root end: the ten vertices of the tube furthest back along the line from the tube's
        # own middle to its joint (the joint file beside the model gives the joint)
        centre = lod.points[members].mean(axis=0)
        joint = np.array(rest["pivots"][child]) if child in rest.get("pivots", {}) else lod.points[cand].mean(axis=0)
        d = joint - centre
        d /= max(1e-9, np.linalg.norm(d))
        order = cand[np.argsort(-((lod.points[cand] - joint) @ d))]
        roots[child] = (parent, order[:10])
    worst = {}
    for stem in CLIPS:
        path = os.path.join(a.anim, stem + ".rtm")
        if not os.path.isfile(path):
            continue
        clip = pp.read_clip(path)
        pose = pp.Pose(lod, clip)
        gpose = pp.Pose(geo, clip) if geo is not None else None
        row = {"in_leg": {"l": 0, "p": 0}, "depth": {"l": 0.0, "p": 0.0}, "low": {"l": 9e9, "p": 9e9}, "geo_low": 9e9,
               "root": {c: 0.0 for c in roots}, "at": {"l": 0.0, "p": 0.0}}
        for k in range(a.phases):
            ph = k / a.phases
            p = pose.at(ph)
            mats = pp.clip_matrices(clip, ph)
            for s in "lp":
                h = p[hands[s]]
                row["low"][s] = min(row["low"][s], float(h[:, 1].min()))
                inside = np.zeros(len(h), dtype=bool)
                deep = 0.0
                for c in legs["l"] + legs["p"] + trunk:
                    tri = p[lod.tris[tri_comp == c]]
                    # quick reject by box
                    lo, hi = tri.reshape(-1, 3).min(axis=0), tri.reshape(-1, 3).max(axis=0)
                    cand = np.all((h > lo) & (h < hi), axis=1)
                    if not cand.any():
                        continue
                    w = winding(h[cand], tri)
                    ins = w > 0.5
                    if ins.any():
                        idx = np.nonzero(cand)[0][ins]
                        inside[idx] = True
                        cen = tri.mean(axis=1)
                        d = np.linalg.norm(h[idx][:, None, :] - cen[None, :, :], axis=2).min(axis=1)
                        deep = max(deep, float(d.max()))
                n = int(inside.sum())
                if n > row["in_leg"][s]:
                    row["in_leg"][s] = n
                    row["at"][s] = ph
                row["depth"][s] = max(row["depth"][s], deep)
            for child, (parent, idx) in roots.items():
                if parent not in mats:
                    continue
                rot, t = mats[parent]
                ride = lod.points[idx] @ rot.T + t
                row["root"][child] = max(row["root"][child], float(np.linalg.norm(p[idx] - ride, axis=1).max()))
            if gpose is not None:
                row["geo_low"] = min(row["geo_low"], float(gpose.at(ph)[:, 1].min()))
        worst[stem] = row
        lines.append("%-14s hands in legs or pelvis: L %3d (deepest %.2f m, phase %.2f)  R %3d (deepest %.2f m, phase %.2f) | lowest hand point L %.2f R %.2f m | "
                     "collision lowest %.2f m | roots move: %s" % (
                         stem, row["in_leg"]["l"], row["depth"]["l"], row["at"]["l"], row["in_leg"]["p"], row["depth"]["p"], row["at"]["p"],
                         row["low"]["l"], row["low"]["p"], row["geo_low"],
                         ", ".join("%s %.2f" % (c, v) for c, v in row["root"].items())))
    text = "\n".join(lines)
    print(text)
    if a.out:
        with open(a.out, "w", encoding="utf-8") as fh:
            fh.write(text + "\n")


if __name__ == "__main__":
    main()
