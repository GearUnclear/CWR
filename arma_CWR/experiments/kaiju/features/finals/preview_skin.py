"""Poses a built Dedushka P3D with an RTM clip, without the game, and shows where the mesh stretches.

  python preview_skin.py <dedushka.p3d> <clip.rtm> <out.png> [--phases 8] [--title text]

For each of N phases of the clip it skins LOD0 the way the engine does (a vertex lands at the weighted
sum of M_bone * v_rest over the bone selections it is in, weights normalised) and draws the body from
the side (his right) and from the front, flat shaded. A triangle is painted

  red     when one of its edges is more than 1.5 times its rest length, or under two thirds of it
  orange  from 1.25 times (or under four fifths)

and the report lists, per phase, how many triangles are red, the worst edge, and which pairs of bones
the red triangles span (so "the tear is between lstehno and bricho" can be said without the game).
It is a check of the weights against the clip, not of the engine: the game frames are the proof.
"""
import argparse
import math
import sys

import numpy as np
from PIL import Image, ImageDraw

sys.path.insert(0, r"D:\Arma_CWA_worktrees\kaiju\arma_CWR\apps\tools\BlenderAddon\io_import_p3d")
import mlod_writer as mw  # noqa: E402
import rtm_io  # noqa: E402


def load_model(path):
    lod = mw.read_mlod(open(path, "rb").read())[1][0]
    pts = np.array(lod.points, dtype=np.float64)
    tris = []
    for f in lod.faces:
        c = [x[0] for x in f.corners]
        for k in range(1, len(c) - 1):
            tris.append((c[0], c[k], c[k + 1]))
    weights = {}
    for sel in lod.selections:
        if sel.points:
            weights[sel.name.lower()] = dict(sel.points)
    return pts, np.array(tris, dtype=np.int64), weights


def clip_matrices(anim, phase):
    """{bone: (3x3 R, t)} at ``phase`` in 0..1, linear between keys like the engine."""
    frames = anim.frames
    times = [f.time for f in frames]
    if phase <= times[0]:
        a, b, k = frames[0], frames[0], 0.0
    else:
        a, b, k = frames[-1], frames[0], 0.0
        for i in range(len(frames) - 1):
            if times[i] <= phase <= times[i + 1]:
                a, b = frames[i], frames[i + 1]
                k = (phase - times[i]) / max(1e-9, times[i + 1] - times[i])
                break
    out = {}
    for bi, name in enumerate(anim.bone_names):
        ma = np.array(a.matrices[bi], dtype=np.float64)
        mb = np.array(b.matrices[bi], dtype=np.float64)
        m = ma * (1 - k) + mb * k
        rot = m[:9].reshape(3, 3).T      # stored as columns aside, up, dir
        out[name.lower()] = (rot, m[9:12])
    return out


def skin(pts, weights, mats):
    acc = np.zeros_like(pts)
    wsum = np.zeros(len(pts))
    for bone, sel in weights.items():
        if bone not in mats:
            continue
        idx = np.fromiter(sel.keys(), dtype=np.int64)
        w = np.fromiter(sel.values(), dtype=np.float64)
        rot, t = mats[bone]
        acc[idx] += (pts[idx] @ rot.T + t) * w[:, None]
        wsum[idx] += w
    out = pts.copy()
    ok = wsum > 1e-6
    out[ok] = acc[ok] / wsum[ok][:, None]
    return out, int((~ok).sum())


def edge_lengths(p, tris):
    a, b, c = p[tris[:, 0]], p[tris[:, 1]], p[tris[:, 2]]
    return np.stack([np.linalg.norm(b - a, axis=1), np.linalg.norm(c - b, axis=1), np.linalg.norm(a - c, axis=1)], axis=1)


def render(draw, p, tris, bad, box, view):
    """view 'side': look along +x at his right side (screen x = -z, forward is left).
    view 'front': look along +z from in front of him (file space is left handed: x = his left, y up,
    z = back, so his left, +x, is on the viewer's right)."""
    x0, y0, w, h, scale = box
    if view == "side":
        sx, depth = -p[:, 2], -p[:, 0]
    else:
        sx, depth = p[:, 0], -p[:, 2]
    px = x0 + w * 0.5 + sx * scale
    py = y0 + h - 12 - p[:, 1] * scale
    a, b, c = p[tris[:, 0]], p[tris[:, 1]], p[tris[:, 2]]
    n = np.cross(b - a, c - a)
    ln = np.linalg.norm(n, axis=1)
    ln[ln < 1e-9] = 1.0
    n = n / ln[:, None]
    light = np.array([0.35, 0.8, -0.5])
    light = light / np.linalg.norm(light)
    shade = 0.35 + 0.65 * np.abs(n @ light)
    d = (depth[tris[:, 0]] + depth[tris[:, 1]] + depth[tris[:, 2]]) / 3.0
    order = np.argsort(d)       # far first
    for i in order:
        t = tris[i]
        s = shade[i]
        if bad[i] == 2:
            col = (int(255 * s), int(40 * s), int(40 * s))
        elif bad[i] == 1:
            col = (int(255 * s), int(160 * s), int(40 * s))
        else:
            col = (int(150 * s), int(145 * s), int(135 * s))
        draw.polygon([(px[t[0]], py[t[0]]), (px[t[1]], py[t[1]]), (px[t[2]], py[t[2]])], fill=col)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("p3d")
    ap.add_argument("rtm")
    ap.add_argument("out")
    ap.add_argument("--phases", type=int, default=8)
    ap.add_argument("--title", default="")
    args = ap.parse_args()
    pts, tris, weights = load_model(args.p3d)
    anim = rtm_io.read_rtm(args.rtm)
    rest = edge_lengths(pts, tris)
    rest[rest < 1e-6] = 1e-6
    # main bone of each vertex, for the report
    main_bone = {}
    for bone, sel in weights.items():
        for i, w in sel.items():
            if w > main_bone.get(i, ("", 0.0))[1]:
                main_bone[i] = (bone, w)
    cell_w, cell_h = 520, 430
    sheet = Image.new("RGB", (cell_w * args.phases, cell_h * 2 + 26), (24, 26, 30))
    draw = ImageDraw.Draw(sheet)
    scale = (cell_h - 30) / 46.0
    lines = ["%s on %s" % (args.rtm, args.p3d), "bones in the clip: %d, selections in LOD0: %d" % (len(anim.bone_names), len(weights))]
    missing = sorted(b for b in weights if b not in {n.lower() for n in anim.bone_names})
    if missing:
        lines.append("selections with no bone in the clip (their vertices fall back to the others, or stay): %s" % missing)
    worst_all = 0.0
    for k in range(args.phases):
        phase = k / args.phases
        p, unbound = skin(pts, weights, clip_matrices(anim, phase))
        ratio = edge_lengths(p, tris) / rest
        hi = ratio.max(axis=1)
        lo = ratio.min(axis=1)
        bad = np.zeros(len(tris), dtype=np.int8)
        bad[(hi > 1.25) | (lo < 0.8)] = 1
        bad[(hi > 1.5) | (lo < 2.0 / 3.0)] = 2
        pairs = {}
        for i in np.nonzero(bad == 2)[0]:
            bones = sorted({main_bone.get(int(v), ("?", 0))[0] for v in tris[i]})
            key = "+".join(bones)
            c = p[tris[i]].mean(axis=0)
            e = pairs.setdefault(key, [0, 0.0, c])
            e[0] += 1
            if hi[i] > e[1]:
                e[1] = hi[i]
                e[2] = c
        worst_all = max(worst_all, float(hi.max()))
        lines.append("phase %.3f: red %d, orange %d of %d triangles, worst edge %.2f x rest, lowest point y %.2f m, unbound vertices %d"
                     % (phase, int((bad == 2).sum()), int((bad == 1).sum()), len(tris), float(hi.max()), float(p[:, 1].min()), unbound))
        for key, (n, worst, c) in sorted(pairs.items(), key=lambda kv: -kv[1][0])[:6]:
            lines.append("      %-28s %4d red, worst %.2f x, about x %+.1f (left +) y %.1f (up) z %+.1f (back +)" % (key, n, worst, c[0], c[1], c[2]))
        render(draw, p, tris, bad, (cell_w * k, 0, cell_w, cell_h, scale), "side")
        render(draw, p, tris, bad, (cell_w * k, cell_h, cell_w, cell_h, scale), "front")
        draw.text((cell_w * k + 6, 4), "phase %.2f  side (his right)" % phase, fill=(230, 230, 230))
        draw.text((cell_w * k + 6, cell_h + 4), "front", fill=(230, 230, 230))
    draw.text((6, cell_h * 2 + 6), "%s   red: an edge over 1.5 x or under 0.67 x its rest length; orange: 1.25 x / 0.8 x. Worst edge %.2f x."
              % (args.title or args.rtm, worst_all), fill=(230, 230, 230))
    sheet.save(args.out)
    with open(args.out.rsplit(".", 1)[0] + ".txt", "w", encoding="utf-8") as fh:
        fh.write("\n".join(lines) + "\n")
    print("\n".join(lines))


if __name__ == "__main__":
    main()
