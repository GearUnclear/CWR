r"""Makes every model and texture of the texture-resolution test from nothing (plain Python).

  python gen_texres.py <out_dir> [--base <dir with the pipeline proxy dedushka.p3d>] [--after-layout <json>]

It reads the pipeline's proxy model (the blocky 40 m biped, copied to
kaiju-notes\features\texres\base) with the pure Python MLOD reader, keeps its
skeleton, selections and special LODs as they are, and rewrites only the
faces of the first visual LOD. The coarser visual LODs are dropped, so every
frame at every distance shows the same faces and the same textures.

Models written to <out_dir>\models:

  dedushka.p3d     CHART. Front faces of the shins, thighs, toes, hands, belly,
                   ribs, chest and head, and the underside of the pelvis:
                   * on the creature's right (the viewer's left): six patches of
                     one piece of hide, authored at 16, 32, 64, 128, 256 and
                     512 texels per metre, low to high, left to right, top row
                     first. Each patch has a ruler band of line pairs on top.
                   * on the creature's left: the MIP TELL, one 4096 sheet mapped
                     at 256 texels per metre whose mip levels are flat colours
                     (red 256/m, yellow 128, green 64, cyan 32, blue 16,
                     magenta 8, white 4). Fullbright, so the colour on screen
                     is the level the GPU sampled: the texel density that the
                     screen can show at that spot.
  dedushka_b.p3d   BEFORE. The whole body in one 1024 sheet, like art candidate A
                   as first shown (13.8 texels per metre there).
  dedushka_a.p3d   AFTER. The body split over 4096 sheets by height band, as
                   TEXTURE-BUDGET.md asks.
  dedushka_s.p3d   SECTIONS. The AFTER model with its faces shuffled, so nearly
                   every face starts a new section: the worst case for draw calls.
  dedushka_m.p3d   AFTER plus one hidden "mip anchor" face per sheet (see
                   LodBuilder.add_mip_anchors): the data-only cure for the engine
                   loading too coarse a level on a giant. The CHART model carries
                   the same anchors from the second chart run on.
  txp_<id>.p3d     one 8 m panel per texture format and size (the limits test).

Textures go to <out_dir>\tex as PNG (the build script converts them with
PoseidonTools, the artists' tool) or straight to <out_dir>\paa when only
paa_py.py can make them. texres_gen.json lists everything.
"""
import json
import math
import os
import random
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
ENGINE_ROOT = os.path.normpath(os.path.join(HERE, "..", "..", "..", ".."))
sys.path.insert(0, os.path.join(ENGINE_ROOT, "apps", "tools", "BlenderAddon", "io_import_p3d"))
sys.path.insert(0, HERE)
import mlod_writer as mw  # noqa: E402
import paa_py  # noqa: E402

FACE_NOLIGHT = 0x1
CREATURE = "uddedushka"
PANELS = "kjtexres"

# ----------------------------------------------------------------------------------------------
# Tuning block
# ----------------------------------------------------------------------------------------------
TILE_M = 4.0                       # the hide tile repeats every 4 m
TRUTH_TPM = 512                    # the ground truth tile is painted at 512 texels per metre
DENSITIES = [16, 32, 64, 128, 256, 512]
TINT = {16: (1.00, 0.80, 0.80), 32: (1.00, 0.90, 0.74), 64: (1.00, 1.00, 0.76),
        128: (0.82, 1.00, 0.82), 256: (0.80, 0.95, 1.00), 512: (0.90, 0.85, 1.00)}
TELL_TPM = 256                     # the mip tell sheet is mapped at this density
TELL_SIZE = 4096
TELL_COLOURS = [(255, 0, 0), (255, 255, 0), (0, 255, 0), (0, 255, 255), (0, 0, 255), (255, 0, 255),
                (255, 255, 255), (0, 0, 0), (0, 0, 0), (0, 0, 0), (0, 0, 0)]
BEFORE_SHEET = 1024
# AFTER layout: sheets by body group, texels per metre by the height of a face's centre.
# Overridden by --after-layout once the measurements are in.
AFTER_LAYOUT = {
    "sheet": 4096,
    "groups": {
        "legs_l": ["lholen", "lchodidlo", "lprsty", "lstehno"],
        "legs_r": ["pholen", "pchodidlo", "pprsty", "pstehno"],
        "torso": ["bricho", "zebra", "hrudnik", "lzadek", "pzadek"],
        "arms_head": ["lrameno", "prameno", "lbiceps", "pbiceps", "lloket", "ploket", "lruka", "pruka", "krk", "hlava"],
    },
    # (height of the face centre above the ground up to, texels per metre)
    "bands": [[3.0, 192], [12.0, 128], [24.0, 96], [100.0, 64]],
    "top_faces": 24,       # faces that look up: never seen from the ground
    "ground_faces": 8,     # soles
}
PAD = 8                            # texels of overscan around every atlas rectangle
SEED = 20261111


# ----------------------------------------------------------------------------------------------
# The hide: one seamless ground truth tile, then honest reductions of it
# ----------------------------------------------------------------------------------------------
def fbm(n, rng, beta):
    """Periodic noise with a 1/f^beta amplitude spectrum, normalised to mean 0, std 1."""
    f = np.fft.fftfreq(n)
    fx, fy = np.meshgrid(f, f)
    r = np.sqrt(fx * fx + fy * fy)
    r[0, 0] = 1.0
    spec = (rng.standard_normal((n, n)) + 1j * rng.standard_normal((n, n))) / (r ** beta)
    spec[0, 0] = 0
    out = np.fft.ifft2(spec).real
    return (out - out.mean()) / out.std()


def voronoi_edges(n, cells, rng):
    """Periodic jittered-grid Voronoi. Returns (d2 - d1) in pixels and a per-cell random value."""
    cs = n / cells
    jx = rng.random((cells, cells))
    jy = rng.random((cells, cells))
    val = rng.random((cells, cells))
    ys, xs = np.mgrid[0:n, 0:n].astype(np.float32)
    cx = np.floor(xs / cs).astype(np.int32)
    cy = np.floor(ys / cs).astype(np.int32)
    d1 = np.full((n, n), 1e9, dtype=np.float32)
    d2 = np.full((n, n), 1e9, dtype=np.float32)
    v1 = np.zeros((n, n), dtype=np.float32)
    for oy in (-1, 0, 1):
        for ox in (-1, 0, 1):
            gx = cx + ox
            gy = cy + oy
            px = (gx + jx[gy % cells, gx % cells]) * cs
            py = (gy + jy[gy % cells, gx % cells]) * cs
            d = np.sqrt((px - xs) ** 2 + (py - ys) ** 2)
            closer = d < d1
            d2 = np.where(closer, d1, np.minimum(d2, d))
            v1 = np.where(closer, val[gy % cells, gx % cells], v1)
            d1 = np.where(closer, d, d1)
    return d2 - d1, v1


def smoothstep(a, b, x):
    t = np.clip((x - a) / (b - a), 0.0, 1.0)
    return t * t * (3 - 2 * t)


def make_truth(rng):
    """The hide at 512 texels per metre over one 4 m tile: float32 (n, n, 3) in 0..1."""
    n = int(TILE_M * TRUTH_TPM)
    edge_big, val_big = voronoi_edges(n, 14, rng)        # plates about 29 cm across
    edge_small, val_small = voronoi_edges(n, 56, rng)    # scales about 7 cm across
    mottle = fbm(n, rng, 1.6)
    grain = fbm(n, rng, 0.35)
    base = np.array([0.42, 0.36, 0.30], dtype=np.float32)
    tone = 0.80 + 0.16 * mottle[..., None] + 0.20 * (val_big[..., None] - 0.5) + 0.10 * (val_small[..., None] - 0.5)
    img = base * tone
    # cracks between plates (about 1.6 cm wide) and between scales (about 0.5 cm)
    crack_big = 1.0 - smoothstep(2.0, 8.0, edge_big)
    crack_small = 1.0 - smoothstep(0.6, 2.6, edge_small)
    img *= (1.0 - 0.72 * crack_big[..., None]) * (1.0 - 0.45 * crack_small[..., None])
    # a pale rim just inside each scale edge, then pores down to one texel
    rim = smoothstep(2.6, 5.0, edge_small) * (1.0 - smoothstep(5.0, 9.0, edge_small))
    img *= 1.0 + 0.10 * rim[..., None]
    img *= 1.0 + 0.09 * grain[..., None]
    # a few rust-red scabs so colour detail is tested too
    scab = smoothstep(1.1, 1.6, fbm(n, rng, 2.2))
    img = img * (1 - 0.6 * scab[..., None]) + np.array([0.42, 0.12, 0.08], dtype=np.float32) * 0.6 * scab[..., None]
    return np.clip(img, 0.0, 1.0).astype(np.float32)


def add_ruler(truth):
    """A 0.5 m band on top of the tile: line pairs with periods of 2, 4, 8 ... 128 truth texels.

    Period in millimetres: 3.9, 7.8, 15.6, 31, 62.5, 125, 250. A density of N texels per
    metre can carry periods down to 2000 / N mm, so the 512 patch shows all seven groups
    and the 16 patch only the last one. Top row: vertical stripes. Second row: horizontal.
    """
    img = truth.copy()
    n = img.shape[0]
    band = int(0.25 * TRUTH_TPM)        # each row 25 cm tall
    group = int(0.25 * TRUTH_TPM)       # each group 25 cm wide
    for gi, period in enumerate([128, 64, 32, 16, 8, 4, 2]):
        x0 = gi * group
        xs = np.arange(group)
        ys = np.arange(band)
        vert = ((xs // (period // 2)) % 2).astype(np.float32)             # stripes that run up and down
        horz = ((ys // (period // 2)) % 2).astype(np.float32)
        img[0:band, x0:x0 + group, :] = (0.08 + 0.84 * vert)[None, :, None]
        img[band:2 * band, x0:x0 + group, :] = (0.08 + 0.84 * horz)[:, None, None]
        img[0:2 * band, x0:x0 + 2, :] = (0.9, 0.1, 0.1)                   # red divider between groups
    img[2 * band:2 * band + 4, 0:7 * group, :] = (0.9, 0.1, 0.1)
    return img


def reduce_tile(truth, tpm):
    """The same hide authored at ``tpm`` texels per metre: an area average of the truth."""
    size = max(4, int(round(TILE_M * tpm)))
    im = Image.fromarray((truth * 255.0 + 0.5).astype(np.uint8), "RGB")
    if size == im.size[0]:
        return np.asarray(im)
    return np.asarray(im.resize((size, size), Image.BOX if size < im.size[0] else Image.LANCZOS))


def save_png(arr, path):
    Image.fromarray(arr, "RGB").save(path, compress_level=1)


# ----------------------------------------------------------------------------------------------
# Model helpers
# ----------------------------------------------------------------------------------------------
BONES = ("pchodidlo", "lchodidlo", "pprsty", "lprsty", "lholen", "pholen", "pstehno", "lstehno",
         "pzadek", "lzadek", "bricho", "zebra", "hrudnik", "krk", "prameno", "lrameno", "hlava",
         "pbiceps", "lbiceps", "ploket", "lloket", "pruka", "lruka")
# Normal index in the proxy -> face name. Game space: X the creature's left, Y up, Z back.
SIDES = ["bottom", "top", "front", "left", "back", "right"]


class Box:
    def __init__(self, bone, faces, points):
        self.bone = bone
        self.faces = faces            # six MlodFace, order = SIDES
        xs = [p[0] for p in points]
        ys = [p[1] for p in points]
        zs = [p[2] for p in points]
        self.lo = (min(xs), min(ys), min(zs))
        self.hi = (max(xs), max(ys), max(zs))


def load_base(path):
    with open(path, "rb") as fh:
        _ver, lods, _warn = mw.read_mlod(fh.read())
    return lods


def boxes_of(lod):
    """The proxy's first visual LOD is boxes of six quads each, then one glow quad."""
    boxes = []
    extra = []
    i = 0
    while i + 6 <= len(lod.faces):
        group = lod.faces[i:i + 6]
        pts = sorted({c[0] for f in group for c in f.corners})
        if len(pts) != 8:
            break
        bone = None
        for sel in lod.selections:
            if sel.name in BONES and all(p in sel.points for p in pts):
                bone = sel.name
                break
        if bone is None:
            raise SystemExit("box at face %d belongs to no bone" % i)
        boxes.append(Box(bone, group, [lod.points[p] for p in pts]))
        i += 6
    extra = lod.faces[i:]
    return boxes, extra


def face_frame(side, box):
    """Origin corner and the u (viewer's right) and v (down) axes of one box face, seen from outside.

    Returns (origin xyz, u axis unit, v axis unit, width m, height m).
    """
    (x0, y0, z0), (x1, y1, z1) = box.lo, box.hi
    if side == "front":      # normal -Z, the viewer looks along +Z: right is +X
        return (x0, y1, z0), (1, 0, 0), (0, -1, 0), x1 - x0, y1 - y0
    if side == "back":       # normal +Z: right is -X
        return (x1, y1, z1), (-1, 0, 0), (0, -1, 0), x1 - x0, y1 - y0
    if side == "left":       # normal +X, the viewer looks along -X: right is +Z
        return (x1, y1, z0), (0, 0, 1), (0, -1, 0), z1 - z0, y1 - y0
    if side == "right":      # normal -X: right is -Z
        return (x0, y1, z1), (0, 0, -1), (0, -1, 0), z1 - z0, y1 - y0
    if side == "top":        # seen from above with the front at the bottom of the picture
        return (x0, y1, z1), (1, 0, 0), (0, 0, -1), x1 - x0, z1 - z0
    # bottom, seen from below with the creature's front at the top of the picture
    return (x0, y0, z0), (1, 0, 0), (0, 0, 1), x1 - x0, z1 - z0


class LodBuilder:
    """Rewrites the faces of one visual LOD, keeping its points and selections."""

    def __init__(self, base_lod):
        self.src = base_lod
        self.lod = mw.MlodLod(base_lod.resolution)
        self.lod.points = list(base_lod.points)
        self.lod.point_flags = list(base_lod.point_flags) if base_lod.point_flags else None
        self.lod.normals = list(base_lod.normals)
        self.lod.properties = list(base_lod.properties)
        self.lod.mass = None
        self.lod.selections = [mw.MlodSelection(s.name, dict(s.points), set()) for s in base_lod.selections]
        self.faces = []      # (texture, MlodFace)
        self.ground_offset = 0.0

    def add_point(self, p, bone):
        self.lod.points.append(tuple(float(c) for c in p))
        if self.lod.point_flags is not None:
            self.lod.point_flags.append(0)
        idx = len(self.lod.points) - 1
        self.lod.selection(bone).points[idx] = 1.0
        return idx

    def quad(self, box, side, s0, s1, t0, t1, texture, uv_of, flags=0):
        """One rectangle of a box face. s runs along the viewer's right, t down, both 0..1.

        ``uv_of(a, b)`` maps metres from the face's top left corner to (u, v).
        """
        origin, ua, va, w, h = face_frame(side, box)
        nrm = SIDES.index(side)
        corners = []
        # Engine (front facing) order, checked against the proxy's own faces: counter-clockwise
        # seen from outside in game space, which mlod_writer.file_order then reverses.
        for s, t in ((s0, t1), (s1, t1), (s1, t0), (s0, t0)):
            a, b = s * w, t * h
            p = tuple(origin[k] + ua[k] * a + va[k] * b for k in range(3))
            u, v = uv_of(a, b)
            corners.append((self.add_point(p, box.bone), nrm, float(u), float(v)))
        self.faces.append((texture, mw.MlodFace(self.order(corners, side), texture, flags)))

    def order(self, corners, side):
        """Wind the quad the way the proxy's own face on that side is wound (file order)."""
        ref = self.ref[side]
        pts = [self.lod.points[c[0]] for c in corners]
        if winding_sign(pts, SIDE_NORMAL[side]) == ref:
            return list(corners)
        return list(reversed(corners))

    def keep(self, face):
        self.faces.append((face.texture, face))

    def add_mip_anchors(self, skip=()):
        """One hidden 3 cm triangle per texture whose three corners share ONE UV point.

        The engine keeps loaded only the mip levels that Shape::RecalculateAreas and
        PrepareTexture (Shape.cpp:1800-1838, DrawPoly.cpp:20-118) think the texture needs,
        from the largest ratio of world area to UV area of any face that uses it and the
        distance from the camera to the model ORIGIN. A face with no UV area makes that
        ratio as large as it can be (maxAOT), so the engine keeps the full size level
        loaded at any distance and the GPU's own filtering picks what the screen needs.
        The triangle sits inside the pelvis where nothing can see it.
        """
        bone = next(n for n in ("lzadek", "bricho", "zebra", "hrudnik") if self.lod.selection(n) is not None)
        seen = {}
        for tex, face in self.faces:
            if tex and tex not in seen and tex not in skip:
                seen[tex] = (face.corners[0][2], face.corners[0][3])
        k = 0
        for tex, (u, v) in seen.items():
            base = (0.6 + 0.05 * k, 20.5 - self.ground_offset, 0.0)
            pts = [base, (base[0] + 0.03, base[1], base[2]), (base[0], base[1] + 0.03, base[2])]
            corners = [(self.add_point(pt, bone), 0, float(u), float(v)) for pt in pts]
            self.faces.append((tex, mw.MlodFace(corners, tex, 0)))
            k += 1
        return k

    def finish(self, sort_by_texture=True, shuffle=None):
        faces = list(self.faces)
        if shuffle is not None:
            shuffle.shuffle(faces)
        elif sort_by_texture:
            faces.sort(key=lambda tf: tf[0])          # stable: one run of faces per texture
        self.lod.faces = [f for _t, f in faces]
        for sel in self.lod.selections:
            pts = sel.points
            sel.faces = {i for i, f in enumerate(self.lod.faces) if all(c[0] in pts for c in f.corners)}
        return self.lod


SIDE_NORMAL = {"bottom": (0, -1, 0), "top": (0, 1, 0), "front": (0, 0, -1), "left": (1, 0, 0),
               "back": (0, 0, 1), "right": (-1, 0, 0)}


def winding_sign(pts, normal):
    a = [pts[1][k] - pts[0][k] for k in range(3)]
    b = [pts[2][k] - pts[0][k] for k in range(3)]
    c = (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])
    return 1 if sum(c[k] * normal[k] for k in range(3)) > 0 else -1


def reference_windings(lod, boxes):
    """File-order winding sign of the proxy's own face on each side (it draws correctly in game)."""
    ref = {}
    box = boxes[0]
    for side, face in zip(SIDES, box.faces):
        pts = [lod.points[c[0]] for c in face.corners]
        ref[side] = winding_sign(pts, SIDE_NORMAL[side])
    return ref


def special_lods(lods):
    return [l for l in lods if l.resolution >= 900]


def write_model(path, visual, lods):
    out = [visual] + special_lods(lods)
    size = mw.write_mlod_file(path, out)
    texs = sorted({f.texture for f in visual.faces if f.texture})
    runs = 0
    prev = None
    for f in visual.faces:
        if f.texture != prev:
            runs += 1
            prev = f.texture
    return {"bytes": size, "points": len(visual.points), "faces": len(visual.faces), "textures": texs,
            "texture_runs": runs}


# ----------------------------------------------------------------------------------------------
# CHART model
# ----------------------------------------------------------------------------------------------
# bone -> (side, columns, rows) for the patch faces on the creature's right; the same face of
# the mirrored bone (or the other half of a centre box) carries the mip tell.
# (patch bone, tell bone, side, columns, rows, patch s range, tell s range); s runs 0..1 along
# the viewer's right. The pelvis underside is mostly inside the thighs, so only its inner
# 2.2 m strip (between the legs) is used.
CHART_PAIRS = [
    ("pprsty", "lprsty", "front", 3, 2, (0.0, 1.0), (0.0, 1.0)),
    ("pholen", "lholen", "front", 2, 3, (0.0, 1.0), (0.0, 1.0)),
    ("pstehno", "lstehno", "front", 2, 3, (0.0, 1.0), (0.0, 1.0)),
    ("pruka", "lruka", "front", 2, 3, (0.0, 1.0), (0.0, 1.0)),
    ("pzadek", "lzadek", "bottom", 1, 6, (6.3 / 8.5, 1.0), (0.0, 2.2 / 8.5)),
]
CHART_CENTRE = [("bricho", "front", 3, 2), ("zebra", "front", 3, 2), ("hrudnik", "front", 3, 2), ("hlava", "front", 2, 3)]


def tex_name(addon, stem):
    name = "%s\\%s.paa" % (addon, stem)
    if len(name) > 31:
        raise SystemExit("texture name too long: %s" % name)
    return name


def build_chart(lods, path, anchors=True):
    base = lods[0]
    boxes, extra = boxes_of(base)
    lb = LodBuilder(base)
    lb.ref = reference_windings(base, boxes)
    blank = tex_name(CREATURE, "tr_blank")
    tell = tex_name(CREATURE, "tr_tell")
    layout = []
    tell_scale = TELL_TPM / float(TELL_SIZE)       # UV per metre

    def tell_uv(a, b):
        return a * tell_scale, b * tell_scale

    def patch_uv(a0, b0):
        return lambda a, b: ((a - a0) / TILE_M, (b - b0) / TILE_M)

    def patches(box, side, s_from, s_to, cols, rows):
        origin, ua, va, w, h = face_frame(side, box)
        k = 0
        for r in range(rows):
            for c in range(cols):
                s0 = s_from + (s_to - s_from) * c / cols
                s1 = s_from + (s_to - s_from) * (c + 1) / cols
                t0, t1 = r / rows, (r + 1) / rows
                d = DENSITIES[k]
                k += 1
                lb.quad(box, side, s0, s1, t0, t1, tex_name(CREATURE, "tr_h%03d" % d), patch_uv(s0 * w, t0 * h))
                sm, tm = 0.5 * (s0 + s1) * w, 0.5 * (t0 + t1) * h
                layout.append({"bone": box.bone, "side": side, "density": d, "col": c, "row": r,
                               "size_m": [round((s1 - s0) * w, 3), round((t1 - t0) * h, 3)],
                               "centre_game_xyz": [round(origin[k] + ua[k] * sm + va[k] * tm, 3) for k in range(3)]})

    pair_patch = {p[0]: p for p in CHART_PAIRS}
    pair_tell = {p[1]: p for p in CHART_PAIRS}
    centre = {c[0]: c for c in CHART_CENTRE}
    seen_head = False
    for box in boxes:
        if box.bone == "hlava":
            if seen_head:
                continue                       # the snout: dropped, it hides the head's front face
            seen_head = True
        for side in SIDES:
            done = False
            if box.bone in pair_patch and pair_patch[box.bone][2] == side:
                _p, _t, _s, cols, rows, (pa, pb), _tr = pair_patch[box.bone]
                patches(box, side, pa, pb, cols, rows)
                if pa > 0.0:
                    lb.quad(box, side, 0.0, pa, 0.0, 1.0, blank, lambda a, b: (0.5, 0.5))
                if pb < 1.0:
                    lb.quad(box, side, pb, 1.0, 0.0, 1.0, blank, lambda a, b: (0.5, 0.5))
                done = True
            elif box.bone in pair_tell and pair_tell[box.bone][2] == side:
                (ta, tb) = pair_tell[box.bone][6]
                lb.quad(box, side, ta, tb, 0.0, 1.0, tell, tell_uv, FACE_NOLIGHT)
                if ta > 0.0:
                    lb.quad(box, side, 0.0, ta, 0.0, 1.0, blank, lambda a, b: (0.5, 0.5))
                if tb < 1.0:
                    lb.quad(box, side, tb, 1.0, 0.0, 1.0, blank, lambda a, b: (0.5, 0.5))
                done = True
            elif box.bone in centre and centre[box.bone][1] == side:
                _b, _s, cols, rows = centre[box.bone]
                # the viewer's left half (the creature's right) carries the patches
                s_mid = 0.5
                if box.bone == "hrudnik":
                    s_mid = (-3.5 - box.lo[0]) / (box.hi[0] - box.lo[0])    # stay clear of the glow patch
                patches(box, side, 0.0, s_mid, cols, rows)
                lb.quad(box, side, 1.0 - s_mid, 1.0, 0.0, 1.0, tell, tell_uv, FACE_NOLIGHT)
                if box.bone == "hrudnik":
                    lb.quad(box, side, s_mid, 1.0 - s_mid, 0.0, 1.0, blank, lambda a, b: (0.5, 0.5))
                done = True
            if not done:
                lb.quad(box, side, 0.0, 1.0, 0.0, 1.0, blank, lambda a, b: (0.5, 0.5))
    for f in extra:
        lb.keep(f)
    info_anchors = lb.add_mip_anchors(skip=(blank,) + tuple(f.texture for f in extra)) if anchors else 0
    visual = lb.finish()
    info = write_model(path, visual, lods)
    info["layout"] = layout
    info["mip_anchors"] = info_anchors
    return info


# ----------------------------------------------------------------------------------------------
# BEFORE and AFTER models: every box face a rectangle of its own in an atlas
# ----------------------------------------------------------------------------------------------
def shelf_pack(rects, size):
    """rects: [(id, w, h)]. Returns {id: (x, y)} or None when they do not fit."""
    order = sorted(rects, key=lambda r: (-r[2], -r[1]))
    x = y = 0
    row_h = 0
    out = {}
    for rid, w, h in order:
        if w > size or h > size:
            return None
        if x + w > size:
            x = 0
            y += row_h
            row_h = 0
        if y + h > size:
            return None
        out[rid] = (x, y)
        x += w
        row_h = max(row_h, h)
    return out


def face_density(box, side, layout):
    if side == "top":
        return layout["top_faces"]
    if side == "bottom" and box.lo[1] < 0.5:
        return layout["ground_faces"]
    centre_h = box.lo[1] if side == "bottom" else 0.5 * (box.lo[1] + box.hi[1])
    for limit, tpm in layout["bands"]:
        if centre_h <= limit:
            return tpm
    return layout["bands"][-1][1]


def build_atlas_model(lods, path, stem, groups, sheet, density_of, tiles, rng, shuffle=None, scale_to_fit=True, anchors=False):
    """groups: {name: [bones]} one sheet each. Returns (model info, sheet images, report)."""
    base = lods[0]
    boxes, extra = boxes_of(base)
    lb = LodBuilder(base)
    lb.ref = reference_windings(base, boxes)
    bone_group = {b: g for g, bones in groups.items() for b in bones}
    sheets = {}
    report = {}
    names = sorted(groups)
    for gi, g in enumerate(names):
        members = [(bi, box, side) for bi, box in enumerate(boxes) for side in SIDES if bone_group.get(box.bone) == g]
        scale = 1.0
        while True:
            rects = []
            dens = {}
            for bi, box, side in members:
                _o, _u, _v, w, h = face_frame(side, box)
                d = density_of(box, side) * scale
                dens[(bi, side)] = d
                rects.append(((bi, side), int(math.ceil(w * d)) + 2 * PAD, int(math.ceil(h * d)) + 2 * PAD))
            placed = shelf_pack(rects, sheet)
            if placed is not None or not scale_to_fit:
                break
            scale *= 0.97
        if placed is None:
            raise SystemExit("group %s does not fit a %d sheet" % (g, sheet))
        img = np.zeros((sheet, sheet, 3), dtype=np.uint8)
        img[:] = (90, 78, 66)
        used = 0
        texture = tex_name(CREATURE, "%s%d" % (stem, gi))
        for (key, rw, rh) in rects:
            bi, side = key
            box = boxes[bi]
            x, y = placed[key]
            d = dens[key]
            tile = tiles(d)
            ox = int(rng.integers(0, tile.shape[1]))
            oy = int(rng.integers(0, tile.shape[0]))
            ys = (np.arange(rh) + oy) % tile.shape[0]
            xs = (np.arange(rw) + ox) % tile.shape[1]
            img[y:y + rh, x:x + rw] = tile[np.ix_(ys, xs)]
            used += (rw - 2 * PAD) * (rh - 2 * PAD)
            _o, _u, _v, w, h = face_frame(side, box)

            def uv_of(a, b, x=x, y=y, d=d):
                return (x + PAD + a * d) / sheet, (y + PAD + b * d) / sheet
            lb.quad(box, side, 0.0, 1.0, 0.0, 1.0, texture, uv_of)
        sheets[texture] = img
        ds = sorted(set(round(v, 1) for v in dens.values()))
        report[g] = {"texture": texture, "sheet": sheet, "scale": round(scale, 3), "fill": round(used / float(sheet * sheet), 3),
                     "texels_per_metre": ds}
    for f in extra:
        lb.keep(f)
    n_anchor = lb.add_mip_anchors(skip=tuple(f.texture for f in extra)) if anchors else 0
    visual = lb.finish(shuffle=shuffle)
    info = write_model(path, visual, lods)
    info["mip_anchors"] = n_anchor
    return info, sheets, report


# ----------------------------------------------------------------------------------------------
# Limits panels
# ----------------------------------------------------------------------------------------------
PANEL_M = 8.0
# id, pixel size, file kind ("paa:<format>" through PoseidonTools, "raw:<format>" through paa_py,
# or a loose file extension that the model names directly)
PANEL_SPECS = [
    ("d1_1024", 1024, "paa:DXT1"),
    ("d1_2048", 2048, "paa:DXT1"),
    ("d1_4096", 4096, "paa:DXT1"),
    ("d5_2048", 2048, "paa:DXT5"),
    ("a8_1024", 1024, "paa:ARGB8888"),
    ("png_2048", 2048, "png"),
    ("png_4096", 4096, "png"),
    ("tga_1024", 1024, "tga"),
    ("jpg_2048", 2048, "jpg"),
    ("d1m_4096", 4096, "raw:DXT1"),      # 4096 with eleven authored levels, to see how many the engine keeps
    # The last two are expected to be refused (checked offline with PoseidonTools, which links
    # the engine's own reader): they come last so a bad reaction cannot cost the other frames.
    ("a4_1024", 1024, "paa:ARGB4444"),   # the tool writes it raw, the reader wants it LZW packed
    ("d1_8192w", 8192, "raw:DXT1"),      # 8192 x 1024: fits the size field, over the reader's 4096 limit
]
# Not built at all, because PoseidonTools writes a broken file (the 24 bit mip size wraps to 0):
#   DXT5 4096 (top mip 16 777 216 bytes), ARGB8888 2048 (the same size). Offline evidence in NOTES.md.


def font(px):
    for name in ("arialbd.ttf", "arial.ttf", "DejaVuSans-Bold.ttf"):
        try:
            return ImageFont.truetype(name, px)
        except OSError:
            continue
    return ImageFont.load_default()


def limits_chart(size, label, alpha=False):
    """A square chart: label, one, two and four texel checkers, hairlines, colour ramps."""
    img = np.zeros((size, size, 3), dtype=np.uint8)
    img[:] = (120, 120, 120)
    q = size // 4
    ys, xs = np.mgrid[0:q, 0:q]
    for i, cell in enumerate((1, 2, 4)):
        chk = (((xs // cell) + (ys // cell)) % 2).astype(np.uint8) * 255
        img[size - q:size, i * q:(i + 1) * q] = chk[..., None]
    # last quarter of the bottom row: one texel lines, vertical then horizontal
    half = q // 2
    img[size - q:size, 3 * q:3 * q + half] = ((xs[:, :half] % 2) * 255)[..., None]
    img[size - q:size, 3 * q + half:4 * q] = ((ys[:, :half] % 2) * 255)[..., None]
    # smooth ramps (banding shows a 16 bit upload): grey, red, green, blue
    ramp = np.linspace(0, 255, size).astype(np.uint8)
    band = q // 4
    for i, ch in enumerate(((1, 1, 1), (1, 0, 0), (0, 1, 0), (0, 0, 1))):
        row = np.zeros((band, size, 3), dtype=np.uint8)
        for k in range(3):
            row[..., k] = ramp[None, :] * ch[k]
        img[size - q - (i + 1) * band:size - q - i * band] = row
    im = Image.fromarray(img, "RGB")
    dr = ImageDraw.Draw(im)
    f = font(size // 7)
    dr.text((size // 30, size // 30), label, fill=(255, 235, 60), font=f, stroke_width=max(1, size // 300), stroke_fill=(0, 0, 0))
    f2 = font(size // 16)
    dr.text((size // 30, size // 30 + size // 6), "%d x %d" % (size, size), fill=(255, 255, 255), font=f2,
            stroke_width=max(1, size // 500), stroke_fill=(0, 0, 0))
    dr.text((size // 30, size // 30 + size // 4), "checkers: 1, 2, 4 texels; lines: 1 texel", fill=(230, 230, 230),
            font=font(size // 28), stroke_width=max(1, size // 700), stroke_fill=(0, 0, 0))
    if alpha:
        a = np.full((size, size), 255, dtype=np.uint8)
        a[size // 2 - q // 2:size // 2 + q // 2, :] = np.linspace(0, 255, size).astype(np.uint8)[None, :]
        im = im.convert("RGBA")
        im.putalpha(Image.fromarray(a, "L"))
    return im


def build_panel(path, texture):
    """One upright 8 m square, 0.5 m off the ground, drawn from both sides, fullbright."""
    lod = mw.MlodLod(1.0)
    lod.normals.append((0.0, 0.0, -1.0))
    h = PANEL_M / 2
    pts = [(-h, 0.5, 0.0), (h, 0.5, 0.0), (h, 0.5 + PANEL_M, 0.0), (-h, 0.5 + PANEL_M, 0.0)]
    uvs = [(0.0, 1.0), (1.0, 1.0), (1.0, 0.0), (0.0, 0.0)]
    idx = []
    for p in pts:
        lod.points.append(p)
        idx.append(len(lod.points) - 1)
    corners = [(i, 0, u, v) for i, (u, v) in zip(idx, uvs)]
    lod.faces.append(mw.MlodFace(mw.file_order(corners), texture, FACE_NOLIGHT))
    lod.faces.append(mw.MlodFace(list(corners), texture, FACE_NOLIGHT))
    lod.properties.append(("lodnoshadow", "1"))
    return mw.write_mlod_file(path, [lod])


def panel_config(ids):
    """config.cpp of the kjtexres addon: one static class per panel (pattern of the kjbeam addon)."""
    units = ", ".join('"KJTxp_%s"' % i for i in ids)
    lines = [
        "// kjtexres: texture limits panels for the texture-resolution test. Written by gen_texres.py.",
        "// One 8 m fullbright square per texture format and size. Nothing here is a weapon.",
        "#define private   0", "#define protected 1", "#define public    2", "",
        "class CfgPatches", "{", "    class kjtexres", "    {",
        "        units[] = {%s};" % units,
        "        weapons[] = {};", "        requiredVersion = 1.30;", "        requiredAddons[] = {};", "    };", "};", "",
        "class CfgVehicles", "{", "    class All {};", "    class Static : All {};", "    class Building : Static {};",
        "    class NonStrategic : Building {};", "    class KJTxpBase : NonStrategic", "    {",
        "        scope = private;", '        simulation = "house";', '        destrType = "DestructNo";',
        "        armor = 1000000;", "        accuracy = 0;", "        cost = 0;", "        mapSize = 8;",
        '        vehicleClass = "Objects";', '        icon = "Unknown_object";', "    };",
    ]
    for i in ids:
        lines += ["    class KJTxp_%s : KJTxpBase" % i, "    {", "        scope = public;",
                  '        model = "\\kjtexres\\txp_%s";' % i, '        displayName = "texture panel %s";' % i, "    };"]
    lines += ["};", ""]
    return "\n".join(lines)


# ----------------------------------------------------------------------------------------------
def main():
    argv = sys.argv[1:]
    if not argv:
        raise SystemExit(__doc__)
    out = os.path.abspath(argv[0])
    base_dir = r"D:\Arma_CWA_worktrees\kaiju-notes\features\texres\base\uddedushka"
    layout = AFTER_LAYOUT
    i = 1
    while i < len(argv):
        if argv[i] == "--base":
            base_dir = argv[i + 1]
            i += 1
        elif argv[i] == "--after-layout":
            with open(argv[i + 1], "r", encoding="utf-8") as fh:
                layout = json.load(fh)
            i += 1
        else:
            raise SystemExit("unknown option %s" % argv[i])
        i += 1
    for sub in ("models", "tex", "paa", "tex_kjtexres", "paa_kjtexres"):
        os.makedirs(os.path.join(out, sub), exist_ok=True)
    rng = np.random.default_rng(SEED)
    summary = {"creature_png": [], "creature_paa": [], "panel_png": [], "panel_paa": [], "panel_loose": [], "models": {}}

    print("[texres] painting the hide at %d texels per metre" % TRUTH_TPM, flush=True)
    truth = make_truth(rng)
    chart_truth = add_ruler(truth)
    tile_cache = {}

    def tiles(tpm):
        key = round(tpm, 2)
        if key not in tile_cache:
            tile_cache[key] = reduce_tile(truth, tpm)
        return tile_cache[key]

    # chart tiles, one per density, tinted
    for d in DENSITIES:
        arr = reduce_tile(chart_truth, d).astype(np.float32) * np.array(TINT[d], dtype=np.float32)
        p = os.path.join(out, "tex", "tr_h%03d.png" % d)
        save_png(np.clip(arr, 0, 255).astype(np.uint8), p)
        summary["creature_png"].append({"png": p, "paa": "tr_h%03d.paa" % d, "format": "DXT1", "size": arr.shape[0]})
    p = os.path.join(out, "tex", "tr_blank.png")
    Image.new("RGB", (8, 8), (70, 70, 74)).save(p)
    summary["creature_png"].append({"png": p, "paa": "tr_blank.paa", "format": "DXT1", "size": 8})

    # the mip tell: every level a flat colour
    mips = []
    s = TELL_SIZE
    k = 0
    while s >= 4:
        mips.append((s, s, paa_py.dxt1_flat(s, s, TELL_COLOURS[k])))
        s //= 2
        k += 1
    p = os.path.join(out, "paa", "tr_tell.paa")
    paa_py.write_paa(p, "DXT1", mips)
    summary["creature_paa"].append({"paa": p, "levels": len(mips), "size": TELL_SIZE})

    lods = load_base(os.path.join(base_dir, "dedushka.p3d"))
    models = os.path.join(out, "models")
    print("[texres] chart model", flush=True)
    summary["models"]["dedushka.p3d"] = build_chart(lods, os.path.join(models, "dedushka.p3d"))

    all_bones = sorted({b.bone for b in boxes_of(lods[0])[0]})
    print("[texres] before model", flush=True)
    info, sheets, rep = build_atlas_model(lods, os.path.join(models, "dedushka_b.p3d"), "tr_b", {"all": all_bones},
                                          BEFORE_SHEET, lambda box, side: 64.0, tiles, rng)
    summary["models"]["dedushka_b.p3d"] = dict(info, sheets=rep)
    for tex, img in sheets.items():
        stem = tex.split("\\")[1][:-4]
        p = os.path.join(out, "tex", stem + ".png")
        save_png(img, p)
        summary["creature_png"].append({"png": p, "paa": stem + ".paa", "format": "DXT1", "size": img.shape[0]})

    print("[texres] after model", flush=True)
    dens = lambda box, side: float(face_density(box, side, layout))  # noqa: E731
    info, sheets, rep = build_atlas_model(lods, os.path.join(models, "dedushka_a.p3d"), "tr_a", layout["groups"],
                                          layout["sheet"], dens, tiles, rng)
    summary["models"]["dedushka_a.p3d"] = dict(info, sheets=rep)
    for tex, img in sheets.items():
        stem = tex.split("\\")[1][:-4]
        p = os.path.join(out, "tex", stem + ".png")
        save_png(img, p)
        summary["creature_png"].append({"png": p, "paa": stem + ".paa", "format": "DXT1", "size": img.shape[0]})
    # the same model and sheets with shuffled faces: the worst case for sections
    info, _sheets, _rep = build_atlas_model(lods, os.path.join(models, "dedushka_s.p3d"), "tr_a", layout["groups"],
                                            layout["sheet"], dens, tiles, np.random.default_rng(1),
                                            shuffle=random.Random(7))
    summary["models"]["dedushka_s.p3d"] = info
    # the AFTER model with one mip anchor face per sheet (same sheets, same UVs)
    info, _sheets, _rep = build_atlas_model(lods, os.path.join(models, "dedushka_m.p3d"), "tr_a", layout["groups"],
                                            layout["sheet"], dens, tiles, np.random.default_rng(1), anchors=True)
    summary["models"]["dedushka_m.p3d"] = info

    print("[texres] limits panels", flush=True)
    for pid, size, kind in PANEL_SPECS:
        if kind.startswith("paa:"):
            fmt = kind[4:]
            im = limits_chart(size, "%s %d" % (fmt, size), alpha=fmt in ("DXT5", "ARGB8888", "ARGB4444"))
            p = os.path.join(out, "tex_kjtexres", pid + ".png")
            im.save(p, compress_level=1)
            summary["panel_png"].append({"png": p, "paa": pid + ".paa", "format": fmt, "size": size})
            texture = tex_name(PANELS, pid)
        elif kind.startswith("raw:"):
            fmt = kind[4:]
            p = os.path.join(out, "paa_kjtexres", pid + ".paa")
            if pid == "d1_8192w":
                one = limits_chart(1024, "DXT1 8192x1024").convert("RGB")
                wide = Image.new("RGB", (8192, 1024))
                for k in range(8):
                    wide.paste(one, (k * 1024, 0))
                mips = []
                cur = wide
                while cur.size[1] >= 4:
                    mips.append((cur.size[0], cur.size[1], paa_py.dxt1_encode(np.asarray(cur))))
                    cur = cur.resize((cur.size[0] // 2, cur.size[1] // 2), Image.BOX)
                paa_py.write_paa(p, fmt, mips)
                summary["panel_paa"].append({"paa": p, "size": [8192, 1024], "levels": len(mips)})
            else:
                # 4096 chart on level 0, then every further level a flat colour down to 4 x 4
                arr = np.asarray(limits_chart(size, "DXT1 %d +levels" % size).convert("RGB"))
                mips = [(size, size, paa_py.dxt1_encode(arr))]
                s2, k = size // 2, 1
                while s2 >= 4:
                    mips.append((s2, s2, paa_py.dxt1_flat(s2, s2, TELL_COLOURS[k])))
                    s2 //= 2
                    k += 1
                paa_py.write_paa(p, fmt, mips)
                summary["panel_paa"].append({"paa": p, "size": size, "levels": len(mips)})
            texture = tex_name(PANELS, pid)
        else:
            im = limits_chart(size, "%s %d" % (kind.upper(), size))
            p = os.path.join(out, "paa_kjtexres", pid + "." + kind)
            if kind == "jpg":
                im.save(p, quality=92)
            else:
                im.save(p)
            summary["panel_loose"].append({"file": p, "size": size})
            texture = "%s\\%s.%s" % (PANELS, pid, kind)
        summary["models"]["txp_%s.p3d" % pid] = {"bytes": build_panel(os.path.join(models, "txp_%s.p3d" % pid), texture),
                                                 "texture": texture}
    summary["panels"] = [s[0] for s in PANEL_SPECS]
    with open(os.path.join(out, "kjtexres_config.cpp"), "w", encoding="utf-8", newline="\n") as fh:
        fh.write(panel_config([s[0] for s in PANEL_SPECS]))
    summary["after_layout"] = layout
    with open(os.path.join(out, "texres_gen.json"), "w", encoding="utf-8") as fh:
        json.dump(summary, fh, indent=1)
    for name, m in summary["models"].items():
        if "faces" in m:
            print("[texres] %-16s %5d points %4d faces %2d textures in %3d runs" % (name, m["points"], m["faces"], len(m["textures"]), m["texture_runs"]))
            for g, r in (m.get("sheets") or {}).items():
                print("           %-10s %s fill %.2f scale %.2f texels/m %s" % (g, r["texture"], r["fill"], r["scale"], r["texels_per_metre"]))


if __name__ == "__main__":
    main()
