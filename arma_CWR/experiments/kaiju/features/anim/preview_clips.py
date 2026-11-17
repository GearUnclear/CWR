"""Offline look at the generated clips: strips of posed frames, no game needed.

  python preview_clips.py <anim dir> <rest json> <out dir> [--proxy <build_proxy.py>] [--only DedWalk,DedRoar]

Reads the RTM files the generator wrote and poses the body exactly as the
engine does (a straight blend of the two keys around a time, one rigid
matrix per bone). With --proxy the proxy's own boxes are drawn (read as text
from build_proxy.py, which is never run); without it, a stick figure from
the joint file. One PNG per clip: rows are views (side, front, three
quarter), columns are moments spread over the clip.
"""
import argparse
import ast
import json
import math
import os
import re
import sys

import numpy as np
from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.normpath(os.path.join(HERE, "..", "..", "..", "..", "apps", "tools", "BlenderAddon", "io_import_p3d")))
sys.path.insert(0, HERE)
import rtm_io  # noqa: E402
import ded_rig as R  # noqa: E402

BOX_FACES = [(0, 3, 2, 1), (4, 5, 6, 7), (0, 1, 5, 4), (1, 2, 6, 5), (2, 3, 7, 6), (3, 0, 4, 7)]
COLOURS = {"ded_body": (118, 100, 80), "ded_limb": (82, 70, 60), "ded_foot": (50, 47, 47), "MARKER": (220, 212, 190)}


def proxy_boxes(path):
    """LOD0 boxes of the proxy as (bone, 8 corners in file space, colour)."""
    src = open(path, encoding="utf-8").read()
    env = {"ALL": (0, 1, 2, 3)}
    out = []
    def grab(name):
        m = re.search(r"^%s = \[(.*?)^\]" % name, src, re.S | re.M)
        return eval("[" + m.group(1) + "]", {"__builtins__": {}}, env)  # literals only
    center, left = grab("CENTER"), grab("LEFT")
    parts = list(center) + list(left) + [("p" + b[1:], (-x1, -x0), ys, zs, tex, lods) for b, (x0, x1), ys, zs, tex, lods in left]
    for bone, (x0, x1), (y0, y1), (z0, z1), tex, lods in parts:
        if 0 not in lods:
            continue
        corners = [(x0, y0, z0), (x1, y0, z0), (x1, y1, z0), (x0, y1, z0), (x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1)]
        out.append((bone, [np.array([c[0], c[2], c[1]]) for c in corners], COLOURS.get(tex, (100, 90, 80))))
    return out


def load_clip(path):
    anim = rtm_io.read_rtm(path)
    names = [n.lower() for n in anim.bone_names]
    frames = []
    for fr in anim.frames:
        mats = {}
        for n, m in zip(names, fr.matrices):
            a = np.eye(4)
            a[:3, 0], a[:3, 1], a[:3, 2], a[:3, 3] = m[0:3], m[3:6], m[6:9], m[9:12]
            mats[n] = a
        frames.append((fr.time, mats))
    return anim, frames


def at(frames, p):
    for i in range(len(frames) - 1):
        t0, t1 = frames[i][0], frames[i + 1][0]
        if t0 <= p <= t1:
            w = (p - t0) / (t1 - t0) if t1 > t0 else 0.0
            return {b: (1 - w) * frames[i][1][b] + w * frames[i + 1][1][b] for b in frames[i][1]}
    return frames[-1][1]


def view_matrix(yaw_deg):
    """Camera looking horizontally; yaw 0 = from his right side (he faces left on the page)."""
    a = math.radians(yaw_deg)
    # screen x axis and depth axis in file space (x left, y up, z back)
    sx = np.array([math.sin(a), 0.0, -math.cos(a)])     # yaw 0: he faces right on the page
    depth = np.array([math.cos(a), 0.0, math.sin(a)])
    return sx, depth


def draw(draw_ctx, mats, boxes, rig, origin, scale, yaw, shift=(0.0, 0.0, 0.0)):
    sx, depth = view_matrix(yaw)
    light = R.unit(np.array([0.3, 0.8, -0.5]))
    polys = []
    def proj(p):
        p = p + np.array(shift)
        return (origin[0] + scale * float(np.dot(p, sx)), origin[1] - scale * float(p[1]))
    if boxes:
        for bone, corners, colour in boxes:
            m = mats[bone]
            pts = [m[:3, :3] @ c + m[:3, 3] for c in corners]
            centre = sum(pts) / 8.0
            for f in BOX_FACES:
                quad = [pts[i] for i in f]
                normal = R.unit(np.cross(quad[1] - quad[0], quad[2] - quad[0]))
                if float(np.dot(normal, quad[0] - centre)) < 0:
                    normal = -normal
                if float(np.dot(normal, depth)) >= 0:        # facing away from the camera
                    continue
                shade = 0.45 + 0.55 * max(0.0, float(np.dot(normal, light)))
                col = tuple(int(c * shade) for c in colour)
                z = float(np.dot(sum(quad) / 4.0, depth))
                polys.append((z, [proj(q) for q in quad], col))
        for z, quad, col in sorted(polys, key=lambda t: -t[0]):
            draw_ctx.polygon(quad, fill=col, outline=(20, 20, 20))
    else:
        for b in R.BONES:
            p = rig.parents[b]
            if p is None or not rig.has.get(b, True) and b not in R.STOCK_BONES:
                continue
            a = R.apply(mats[p], rig.piv[p])
            c = R.apply(mats[b], rig.piv[b])
            draw_ctx.line([proj(a), proj(c)], fill=(60, 50, 40), width=3)
        for name in ("foot_l", "foot_p", "hand_l", "hand_p", "head_top"):
            c = rig.point(mats, name)
            owner = {"foot_l": "lchodidlo", "foot_p": "pchodidlo", "hand_l": "lruka", "hand_p": "pruka", "head_top": "hlava"}[name]
            draw_ctx.line([proj(R.apply(mats[owner], rig.piv[owner])), proj(c)], fill=(120, 40, 30), width=3)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("anim_dir")
    ap.add_argument("rest")
    ap.add_argument("out_dir")
    ap.add_argument("--proxy", default="")
    ap.add_argument("--only", default="")
    ap.add_argument("--cols", type=int, default=10)
    args = ap.parse_args()
    os.makedirs(args.out_dir, exist_ok=True)
    rig = R.load_rig(args.rest)
    boxes = proxy_boxes(args.proxy) if args.proxy else None
    timing = json.load(open(os.path.join(args.anim_dir, "ded_anim_timing.json"), encoding="utf-8"))
    only = [s for s in args.only.split(",") if s]
    h = rig.height
    scale = 150.0 / h                          # pixels per metre
    cell_w, cell_h = int(scale * h * 1.25), int(scale * h * 1.22)
    views = [("side", 0.0), ("front", 90.0), ("three quarter", 55.0)]
    for clip in timing["clips"]:
        if only and clip["move"] not in only:
            continue
        stem = clip["file"].split("\\")[-1]
        anim, frames = load_clip(os.path.join(args.anim_dir, stem))
        cols = args.cols
        img = Image.new("RGB", (cell_w * cols, cell_h * len(views) + 24), (188, 200, 210))
        d = ImageDraw.Draw(img)
        d.text((6, 4), "%s  %.1f s  %s  step %.2f m" % (clip["move"], clip["secs"], "loop" if clip["looped"] else "one shot",
                                                         clip["step_m"]), fill=(0, 0, 0))
        for r, (label, yaw) in enumerate(views):
            ground = 24 + cell_h * (r + 1) - int(0.06 * cell_h)
            d.line([(0, ground), (img.width, ground)], fill=(90, 100, 90), width=2)
            for c in range(cols):
                p = c / cols if clip["looped"] else c / (cols - 1)
                mats = at(frames, p)
                draw(d, mats, boxes, rig, (cell_w * c + cell_w // 2, ground), scale, yaw)
                if r == 0:
                    d.text((cell_w * c + 4, 24 + 2), "%.2f s" % (p * clip["secs"]), fill=(0, 0, 0))
            d.text((4, ground + 4), label, fill=(0, 0, 0))
        img.save(os.path.join(args.out_dir, "preview_%s.png" % clip["move"]))
        # The walk again, unrolled: the body advances by the step, so planted feet must stay put.
        if clip["step_m"]:
            n = 14
            span = clip["step_m"] * 1.3 + h * 0.9
            half = 0.5 * scale
            img = Image.new("RGB", (int(half * span), int(cell_h * 0.62 * n)), (188, 200, 210))
            d = ImageDraw.Draw(img)
            for i in range(n):
                p = 1.3 * i / (n - 1)
                mats = at(frames, p % 1.0)
                ground = int(cell_h * 0.62 * (i + 1)) - 8
                d.line([(0, ground), (img.width, ground)], fill=(90, 100, 90), width=1)
                # side view from his right: he walks to the right of the page
                draw(d, mats, boxes, rig, (int(half * h * 0.45), ground), half, 0.0,
                     shift=(0.0, 0.0, -clip["step_m"] * p))
                d.text((4, ground - 12), "%.2f s" % (p * clip["secs"]), fill=(0, 0, 0))
            img.save(os.path.join(args.out_dir, "preview_%s_unrolled.png" % clip["move"]))
    print("previews in", args.out_dir)


if __name__ == "__main__":
    main()
