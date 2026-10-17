"""Synthetic OFP-like scene for the lighting lab (renderer-independent data).

World space matches the engine: X east, Y up, Z north (left-handed), metres.
All mesh vertices are stored in world space (the renderer builds the
camera-relative world matrix the engine uses). The sky dome is the exception:
it is stored camera-relative (it is re-centred on the camera every frame, as
Landscape::DrawSky does).

Triangle winding: every triangle is stored so that numpy.cross(b - a, c - a)
points along the face's outward normal. In the engine's left-handed world with
glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE) that is the D3D convention
"clockwise on screen = front face", so renderers can cull back faces with
glFrontFace(GL_CW) exactly like EngineGL33::BeginPass.

Contents
  terrain   600 m x 600 m detail area at a 6.25 m grid (smooth normals by central
            differences) inside a coarser tensor-product skirt out to ~1.5 km so the
            long view has no edge; one texture per 50 m square, uv = world_xz / 50,
            detail UV generated in the shader (texMat1 = scale 32, TGDetail)
  village   2 houses (flat-shaded walls + gable roof), an APC-like vehicle (bevelled
            hull, smoothed edges, cylinder wheels), trees (trunk + crossed alpha-cutout
            cards), a soldier capsule, rocks and crates
  sky       hemisphere, camera-relative, outward normals (DoSkyLighting input)
"""

from __future__ import annotations

import math
from dataclasses import dataclass, field

import numpy as np

import textures as T
from lighting import MaterialRecipe

GRID = 6.25  # terrain grid (m), default OFP landGrid / terrain subdivision
TEX_SQUARE = 50.0  # one terrain texture per 50 m square
DETAIL_AREA = 600.0

# Foliage lighting. The task brief asks for forcedDiffuse = 0.5 * accom on top
# of the normal material ("custom"). The engine's own per-vertex lighting flags
# would give CreateMaterialConstant(accom, factor): MSHalfLighted (factor 0.5)
# or MSFullLighted (factor 1.0, forcedDiffuse = diffuse = 0.5 * accom).
FOLIAGE_MATERIAL = MaterialRecipe(kind="custom", forced_diffuse=0.5)
# FOLIAGE_MATERIAL = MaterialRecipe(kind="constant", factor=1.0)   # MSFullLighted

NORMAL = MaterialRecipe(kind="normal")
VEHICLE_PAINT = MaterialRecipe(kind="normal", specular=(0.2, 0.2, 0.2, 1.0), specular_power=20)


@dataclass
class Mesh:
    name: str
    positions: np.ndarray  # (N, 3) float32
    normals: np.ndarray  # (N, 3) float32, unit
    uvs: np.ndarray  # (N, 2) float32
    indices: np.ndarray  # (M, 3) uint32
    texture: str
    material: MaterialRecipe = field(default_factory=lambda: NORMAL)
    shader: str = "normal"  # 'normal' (PSNormal) | 'detail' (PSDetail + TGDetail)
    pass_id: str = "opaque"  # 'opaque' | 'cutout' (alpha test, ref 0xC0)
    detail_texture: str | None = None
    colors: np.ndarray | None = None  # optional (N, 4) pre-lit colours (sky only)
    # (min, max) of the whole model this mesh belongs to (Shape::Min/Max, which
    # the remastered crown normals use); None = this mesh's own bounds.
    bounds: tuple | None = None

    @property
    def n_tris(self) -> int:
        return int(self.indices.shape[0])


@dataclass
class CameraPreset:
    name: str
    position: np.ndarray
    forward: np.ndarray
    up: np.ndarray = field(default_factory=lambda: np.array([0.0, 1.0, 0.0]))
    fov: float = 0.7  # engine default camera fov (World.cpp)
    description: str = ""

    def basis(self) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
        """(aside, up, forward) orthonormal, left-handed (aside = up x forward)."""
        f = self.forward / np.linalg.norm(self.forward)
        u = self.up - f * np.dot(self.up, f)
        u /= np.linalg.norm(u)
        a = np.cross(u, f)
        return a, u, f


# ---------------------------------------------------------------------------
# Geometry helpers
# ---------------------------------------------------------------------------


class MeshBuilder:
    def __init__(self):
        self.p: list[np.ndarray] = []
        self.n: list[np.ndarray] = []
        self.t: list[np.ndarray] = []
        self.i: list[np.ndarray] = []
        self.count = 0

    def add(self, pos, nrm, uv, tris, outward=None):
        """Append vertices + triangles (indices local to this call).

        If `outward` (per-triangle (M,3) or a single vector) is given, triangles
        are flipped so cross(b-a, c-a) . outward > 0. Otherwise the vertex
        normals are used as the reference.
        """
        pos = np.asarray(pos, dtype=np.float64).reshape(-1, 3)
        nrm = np.asarray(nrm, dtype=np.float64).reshape(-1, 3)
        uv = np.asarray(uv, dtype=np.float64).reshape(-1, 2)
        tris = np.asarray(tris, dtype=np.int64).reshape(-1, 3).copy()
        a, b, c = pos[tris[:, 0]], pos[tris[:, 1]], pos[tris[:, 2]]
        cr = np.cross(b - a, c - a)
        if outward is None:
            ref = nrm[tris[:, 0]] + nrm[tris[:, 1]] + nrm[tris[:, 2]]
        else:
            ref = np.broadcast_to(np.asarray(outward, dtype=np.float64), cr.shape)
        flip = np.einsum("ij,ij->i", cr, ref) < 0
        tris[flip] = tris[flip][:, [0, 2, 1]]
        self.p.append(pos)
        self.n.append(nrm / np.linalg.norm(nrm, axis=1, keepdims=True))
        self.t.append(uv)
        self.i.append(tris + self.count)
        self.count += pos.shape[0]

    def quad(self, corners, normal, uvs, vertex_normals=None):
        corners = np.asarray(corners, dtype=np.float64)
        nrm = np.tile(np.asarray(normal, dtype=np.float64), (4, 1)) if vertex_normals is None else vertex_normals
        self.add(corners, nrm, uvs, [[0, 1, 2], [0, 2, 3]], outward=normal)

    def tri(self, corners, normal, uvs, vertex_normals=None):
        corners = np.asarray(corners, dtype=np.float64)
        nrm = np.tile(np.asarray(normal, dtype=np.float64), (3, 1)) if vertex_normals is None else vertex_normals
        self.add(corners, nrm, uvs, [[0, 1, 2]], outward=normal)

    def build(self, name, texture, **kw) -> Mesh:
        return Mesh(
            name=name,
            positions=np.concatenate(self.p).astype(np.float32),
            normals=np.concatenate(self.n).astype(np.float32),
            uvs=np.concatenate(self.t).astype(np.float32),
            indices=np.concatenate(self.i).astype(np.uint32),
            texture=texture,
            **kw,
        )


def rot_y(deg: float) -> np.ndarray:
    """Rotation about +Y; yaw 0 = facing +Z (north), 90 = facing +X (east)."""
    a = math.radians(deg)
    c, s = math.cos(a), math.sin(a)
    # columns = images of local x, y, z
    return np.array([[c, 0.0, s], [0.0, 1.0, 0.0], [-s, 0.0, c]])


def transform(mesh_builder: MeshBuilder, rot: np.ndarray, offset: np.ndarray) -> None:
    for k in range(len(mesh_builder.p)):
        mesh_builder.p[k] = mesh_builder.p[k] @ rot.T + offset
        mesh_builder.n[k] = mesh_builder.n[k] @ rot.T


def heading(az_deg: float) -> np.ndarray:
    a = math.radians(az_deg)
    return np.array([math.sin(a), 0.0, math.cos(a)])


# ---------------------------------------------------------------------------
# Terrain
# ---------------------------------------------------------------------------


class Heightfield:
    """Deterministic rolling hills + valley + one steep ridge + a flat village pad."""

    def __init__(self, seed: int = 1, village=(300.0, 300.0)):
        rng = np.random.default_rng(1000 + seed)
        self.waves = []
        for wl, amp in [(520, 7.0), (380, 5.0), (260, 3.5), (170, 2.2), (110, 1.2), (70, 0.6)]:
            ang = rng.uniform(0, 2 * np.pi)
            self.waves.append((2 * np.pi / wl * math.cos(ang), 2 * np.pi / wl * math.sin(ang),
                               rng.uniform(0, 2 * np.pi), amp))
        self.village = np.array(village)
        # ridge: segment roughly perpendicular to the overview view (SW of the village)
        self.ridge_a = np.array([60.0, 250.0])
        self.ridge_b = np.array([260.0, 40.0])
        self.ridge_h = 30.0
        self.ridge_w = 26.0

    def _raw(self, x, z):
        h = np.zeros(np.broadcast(x, z).shape)
        for kx, kz, ph, amp in self.waves:
            h += amp * np.sin(kx * x + kz * z + ph)
        # valley: sinuous channel running roughly north-south, west of the village
        vx = 200.0 + 35.0 * np.sin(z / 95.0)
        h -= 9.0 * np.exp(-(((x - vx) / 32.0) ** 2))
        # ridge: steep gaussian along a tapered segment
        ab = self.ridge_b - self.ridge_a
        L2 = float(ab @ ab)
        t = ((x - self.ridge_a[0]) * ab[0] + (z - self.ridge_a[1]) * ab[1]) / L2
        tc = np.clip(t, 0.0, 1.0)
        px = self.ridge_a[0] + tc * ab[0]
        pz = self.ridge_a[1] + tc * ab[1]
        d = np.sqrt((x - px) ** 2 + (z - pz) ** 2)
        taper = np.sin(np.pi * np.clip(t, 0, 1)) ** 0.6 * np.exp(-np.maximum(np.abs(t - 0.5) - 0.5, 0) * 8)
        h += self.ridge_h * taper * np.exp(-((d / self.ridge_w) ** 2) * 1.6)
        return h + 20.0

    def __call__(self, x, z):
        x = np.asarray(x, dtype=np.float64)
        z = np.asarray(z, dtype=np.float64)
        h = self._raw(x, z)
        # flatten a village pad (blend toward a gentle plane)
        d = np.sqrt((x - self.village[0]) ** 2 + (z - self.village[1]) ** 2)
        w = np.clip((d - 55.0) / 50.0, 0.0, 1.0)
        w = w * w * (3 - 2 * w)
        pad = self._raw(self.village[0], self.village[1]) + 0.012 * (x - self.village[0]) - 0.008 * (z - self.village[1])
        return pad * (1 - w) + h * w

    def normal(self, x, z, eps=GRID):
        """Smooth vertex normal by central differences over one grid step."""
        dhdx = (self(x + eps, z) - self(x - eps, z)) / (2 * eps)
        dhdz = (self(x, z + eps) - self(x, z - eps)) / (2 * eps)
        n = np.stack([-dhdx, np.ones_like(dhdx), -dhdz], axis=-1)
        return n / np.linalg.norm(n, axis=-1, keepdims=True)


def terrain_axis() -> np.ndarray:
    inner = np.arange(0.0, DETAIL_AREA + GRID * 0.5, GRID)
    steps = [12.5, 25, 50, 75, 100, 150, 200, 250]
    lo = [0.0]
    for s in steps:
        lo.append(lo[-1] - s)
    hi = [DETAIL_AREA]
    for s in steps:
        hi.append(hi[-1] + s)
    return np.array(sorted(set(lo[1:] + list(inner) + hi[1:])))


def build_terrain(hf: Heightfield, dirt_squares: set[tuple[int, int]]) -> list[Mesh]:
    xs = terrain_axis()
    zs = terrain_axis()
    X, Z = np.meshgrid(xs, zs, indexing="xy")  # (nz, nx)
    H = hf(X, Z)
    N = hf.normal(X, Z)
    nz, nx = X.shape
    pos = np.stack([X, H, Z], axis=-1).reshape(-1, 3)
    nrm = N.reshape(-1, 3)
    uv = np.stack([X / TEX_SQUARE, Z / TEX_SQUARE], axis=-1).reshape(-1, 2)
    # cells -> texture by the 50 m square of the cell centre
    cx = 0.5 * (xs[:-1] + xs[1:])
    cz = 0.5 * (zs[:-1] + zs[1:])
    sq_x = np.floor(cx / TEX_SQUARE).astype(int)
    sq_z = np.floor(cz / TEX_SQUARE).astype(int)
    groups: dict[str, list] = {"grass": [], "dirt": []}
    for j in range(nz - 1):
        for i in range(nx - 1):
            v00 = j * nx + i
            v10 = v00 + 1
            v01 = v00 + nx
            v11 = v01 + 1
            key = "dirt" if (sq_x[i], sq_z[j]) in dirt_squares else "grass"
            groups[key].append((v00, v01, v11))
            groups[key].append((v00, v11, v10))
    meshes = []
    for key, tris in groups.items():
        if not tris:
            continue
        tris = np.array(tris, dtype=np.int64)
        used, inv = np.unique(tris.reshape(-1), return_inverse=True)
        mb = MeshBuilder()
        mb.add(pos[used], nrm[used], uv[used], inv.reshape(-1, 3), outward=np.array([0.0, 1.0, 0.0]))
        meshes.append(mb.build(f"terrain_{key}", key, shader="detail", detail_texture="detail"))
    return meshes


# ---------------------------------------------------------------------------
# Objects
# ---------------------------------------------------------------------------


def build_house(center, yaw, width, depth, wall_h, roof_h, ground_y, overhang=0.4) -> list[Mesh]:
    """Box walls + gable roof, flat-shaded. Ridge runs along local x (width)."""
    rot = rot_y(yaw)
    hw, hd = width / 2, depth / 2
    base = -0.4  # sink below the ground a little
    top = wall_h
    walls = MeshBuilder()
    # The wall texture is one ~4 m x 3 m bay with a centred window; each wall
    # gets a whole number of bays. v runs from the wall top (0) down per 3 m.
    vb = (top - base) / 3
    for sz in (+1, -1):
        z = sz * hd
        nb = max(1, round(width / 4))
        corners = [(-hw, top, z), (hw, top, z), (hw, base, z), (-hw, base, z)]
        walls.quad(corners, (0, 0, sz), [(0, 0), (nb, 0), (nb, vb), (0, vb)])
    # short walls (normal +-x) with gables
    for sx in (+1, -1):
        x = sx * hw
        nb = max(1, round(depth / 4))
        corners = [(x, top, -hd), (x, top, hd), (x, base, hd), (x, base, -hd)]
        walls.quad(corners, (sx, 0, 0), [(0, 0), (nb, 0), (nb, vb), (0, vb)])
        # gable: map onto the window-free plaster strip at the top-left of the bay
        gable = [(x, top, -hd), (x, top, hd), (x, top + roof_h, 0.0)]
        walls.tri(gable, (sx, 0, 0), [(0.02, 0.2), (0.32, 0.2), (0.17, 0.02)])
    transform(walls, rot, np.array([center[0], ground_y, center[1]]))

    roof = MeshBuilder()
    rx = hw + overhang
    slope_len = math.hypot(hd + overhang, roof_h * (hd + overhang) / hd)
    for sz in (+1, -1):
        ze = sz * (hd + overhang)
        ye = top - roof_h * overhang / hd
        n = np.array([0.0, hd, sz * roof_h])
        n /= np.linalg.norm(n)
        corners = [(-rx, top + roof_h, 0.0), (rx, top + roof_h, 0.0), (rx, ye, ze), (-rx, ye, ze)]
        uvs = [(0, 0), (2 * rx / 2, 0), (2 * rx / 2, slope_len / 2), (0, slope_len / 2)]
        roof.quad(corners, n, uvs)
        # soffit / underside (faces down-out), so the overhang is not see-through
        roof.quad(corners, -n, uvs)
    transform(roof, rot, np.array([center[0], ground_y, center[1]]))
    return [walls.build("house_walls", "plaster"), roof.build("house_roof", "roof")]


def chamfered_box(mb: MeshBuilder, half, r, offset, uv_scale=0.5):
    """Box with every edge bevelled by r. Main faces flat; bevel and corner
    vertices take the adjacent main-face normal, so edges shade as rounded."""
    a = np.asarray(half, dtype=np.float64)
    off = np.asarray(offset, dtype=np.float64)

    def uv_for(p, n):
        ax = int(np.argmax(np.abs(n)))
        if ax == 0:
            return np.stack([p[:, 2], -p[:, 1]], axis=1) * uv_scale
        if ax == 1:
            return np.stack([p[:, 0], p[:, 2]], axis=1) * uv_scale
        return np.stack([p[:, 0], -p[:, 1]], axis=1) * uv_scale

    def pt(axis_main, s_main, s1, s2, inset_main=False):
        """Point on face (axis_main, s_main) at tangential signs s1, s2."""
        p = np.zeros(3)
        o = [k for k in range(3) if k != axis_main]
        p[axis_main] = s_main * (a[axis_main] - (r if inset_main else 0.0))
        p[o[0]] = s1 * (a[o[0]] - r)
        p[o[1]] = s2 * (a[o[1]] - r)
        return p

    # main faces
    for ax in range(3):
        for s in (+1, -1):
            n = np.zeros(3)
            n[ax] = s
            ps = np.array([pt(ax, s, -1, -1), pt(ax, s, 1, -1), pt(ax, s, 1, 1), pt(ax, s, -1, 1)])
            mb.quad(ps + off, n, uv_for(ps + off, n))
    # edge bevels between face (ax1, s1) and (ax2, s2)
    for ax1 in range(3):
        for ax2 in range(ax1 + 1, 3):
            ax3 = 3 - ax1 - ax2
            for s1 in (+1, -1):
                for s2 in (+1, -1):
                    n1 = np.zeros(3)
                    n1[ax1] = s1
                    n2 = np.zeros(3)
                    n2[ax2] = s2
                    quad = []
                    nrms = []
                    for s3 in (-1, +1):
                        p = np.zeros(3)
                        p[ax1] = s1 * a[ax1]
                        p[ax2] = s2 * (a[ax2] - r)
                        p[ax3] = s3 * (a[ax3] - r)
                        q = np.zeros(3)
                        q[ax1] = s1 * (a[ax1] - r)
                        q[ax2] = s2 * a[ax2]
                        q[ax3] = s3 * (a[ax3] - r)
                        quad.append((p, q))
                        nrms.append((n1, n2))
                    corners = np.array([quad[0][0], quad[1][0], quad[1][1], quad[0][1]])
                    vn = np.array([nrms[0][0], nrms[1][0], nrms[1][1], nrms[0][1]])
                    fn = n1 + n2
                    fn /= np.linalg.norm(fn)
                    mb.quad(corners + off, fn, uv_for(corners + off, fn), vertex_normals=vn)
    # corner triangles
    for sx in (+1, -1):
        for sy in (+1, -1):
            for sz in (+1, -1):
                s = np.array([sx, sy, sz], dtype=np.float64)
                ps, ns = [], []
                for ax in range(3):
                    p = s * (a - r)
                    p[ax] = s[ax] * a[ax]
                    n = np.zeros(3)
                    n[ax] = s[ax]
                    ps.append(p)
                    ns.append(n)
                ps = np.array(ps)
                fn = s / np.linalg.norm(s)
                mb.tri(ps + off, fn, uv_for(ps + off, fn), vertex_normals=np.array(ns))


def cylinder(mb: MeshBuilder, radius, length, segments, center, axis="x", caps=True, uv_scale=1.0,
             radius_end=None):
    """Cylinder (or truncated cone with radius_end) with smooth side normals and flat caps."""
    radius_end = radius if radius_end is None else radius_end
    ang = np.linspace(0, 2 * np.pi, segments + 1)
    c, s = np.cos(ang), np.sin(ang)
    h = np.array([-length / 2, length / 2])
    slope = (radius - radius_end) / length

    def place(local):  # local: (..., 3) with axial coord in [0]
        if axis == "x":
            return local[..., [0, 1, 2]]
        if axis == "y":
            return local[..., [1, 0, 2]]
        return local[..., [1, 2, 0]]

    pos, nrm, uv = [], [], []
    for k, hk in enumerate(h):
        rk = radius if k == 0 else radius_end
        for i in range(segments + 1):
            pos.append([hk, rk * c[i], rk * s[i]])
            n = np.array([slope, c[i], s[i]])
            nrm.append(n / np.linalg.norm(n))
            uv.append([i / segments * 2 * uv_scale, k * length * 0.5 * uv_scale])
    tris = []
    for i in range(segments):
        a0, a1 = i, i + 1
        b0, b1 = segments + 1 + i, segments + 2 + i
        tris += [[a0, b0, b1], [a0, b1, a1]]
    pos = place(np.array(pos)) + np.asarray(center)
    nrm = place(np.array(nrm))
    mb.add(pos, nrm, np.array(uv), np.array(tris))
    if caps:
        for k, hk in enumerate(h):
            rk = radius if k == 0 else radius_end
            sgn = -1.0 if k == 0 else 1.0
            ring = np.array([[hk, rk * c[i], rk * s[i]] for i in range(segments)])
            cen = np.array([[hk, 0.0, 0.0]])
            p = place(np.concatenate([cen, ring])) + np.asarray(center)
            n = place(np.array([[sgn, 0.0, 0.0]]))[0]
            nn = np.tile(n, (segments + 1, 1))
            uvc = np.concatenate([[[0.5, 0.5]], 0.5 + 0.5 * np.stack([c[:-1], s[:-1]], 1)])
            t = [[0, 1 + i, 1 + (i + 1) % segments] for i in range(segments)]
            mb.add(p, nn, uvc, np.array(t), outward=n)


def _inset_convex(poly: np.ndarray, r: float) -> np.ndarray:
    """Offset a convex polygon (N,2) inward by r (edge lines moved, adjacent ones intersected)."""
    n = len(poly)
    area = 0.5 * np.sum(poly[:, 0] * np.roll(poly[:, 1], -1) - np.roll(poly[:, 0], -1) * poly[:, 1])
    sgn = 1.0 if area > 0 else -1.0
    lines = []
    for i in range(n):
        p, q = poly[i], poly[(i + 1) % n]
        d = (q - p) / np.linalg.norm(q - p)
        inward = sgn * np.array([-d[1], d[0]])
        lines.append((p + inward * r, d))
    out = []
    for i in range(n):
        (p1, d1), (p2, d2) = lines[i - 1], lines[i]
        A = np.array([d1, -d2]).T
        t = np.linalg.solve(A, p2 - p1)
        out.append(p1 + d1 * t[0])
    return np.array(out)


def extruded_hull(mb: MeshBuilder, profile, half_width: float, r: float, smooth_deg: float = 40.0,
                  uv_scale: float = 0.5) -> None:
    """Convex side profile (z, y) extruded across x with bevelled side edges.

    Profile corners sharper than `smooth_deg` stay hard; flatter ones (the small
    chamfer segments) share a smoothed normal. Bevel strips blend the profile
    normal into the side-plate normal, so the long edges shade as rounded.
    """
    P = np.asarray(profile, dtype=np.float64)
    n = len(P)
    area = 0.5 * np.sum(P[:, 0] * np.roll(P[:, 1], -1) - np.roll(P[:, 0], -1) * P[:, 1])
    sgn = 1.0 if area > 0 else -1.0
    # outward edge normals in (z, y)
    en = []
    for i in range(n):
        d = P[(i + 1) % n] - P[i]
        d /= np.linalg.norm(d)
        en.append(-sgn * np.array([-d[1], d[0]]))
    en = np.array(en)
    cos_lim = math.cos(math.radians(smooth_deg))
    # per edge, normals at its start / end vertex
    start_n, end_n = [], []
    for i in range(n):
        prev_e, next_e = en[i - 1], en[(i + 1) % n]
        s = en[i] + prev_e if np.dot(en[i], prev_e) > cos_lim else en[i]
        e = en[i] + next_e if np.dot(en[i], next_e) > cos_lim else en[i]
        start_n.append(s / np.linalg.norm(s))
        end_n.append(e / np.linalg.norm(e))
    Q = _inset_convex(P, r)
    xi = half_width - r
    arc = np.concatenate([[0.0], np.cumsum(np.linalg.norm(np.diff(np.vstack([P, P[:1]]), axis=0), axis=1))])

    def v3(x, zy):
        return np.array([x, zy[1], zy[0]])

    def n3(nx, zy):
        return np.array([nx, zy[1], zy[0]])

    for i in range(n):
        j = (i + 1) % n
        fn = n3(0.0, en[i])
        # body strip
        corners = [v3(-xi, P[i]), v3(xi, P[i]), v3(xi, P[j]), v3(-xi, P[j])]
        vn = [n3(0, start_n[i]), n3(0, start_n[i]), n3(0, end_n[i]), n3(0, end_n[i])]
        uvs = [(-xi * uv_scale, arc[i] * uv_scale), (xi * uv_scale, arc[i] * uv_scale),
               (xi * uv_scale, arc[i + 1] * uv_scale), (-xi * uv_scale, arc[i + 1] * uv_scale)]
        mb.quad(corners, fn, uvs, vertex_normals=np.array(vn))
        # bevel strips to the side plates
        for sx in (+1, -1):
            corners = [v3(sx * xi, P[i]), v3(sx * xi, P[j]), v3(sx * half_width, Q[j]), v3(sx * half_width, Q[i])]
            side = np.array([float(sx), 0.0, 0.0])
            vn = [n3(0, start_n[i]), n3(0, end_n[i]), side, side]
            bn = fn + side
            uvs = [(P[i][0] * uv_scale, -P[i][1] * uv_scale), (P[j][0] * uv_scale, -P[j][1] * uv_scale),
                   (Q[j][0] * uv_scale, -Q[j][1] * uv_scale), (Q[i][0] * uv_scale, -Q[i][1] * uv_scale)]
            mb.quad(corners, bn / np.linalg.norm(bn), uvs, vertex_normals=np.array(vn))
    # side plates (fan)
    for sx in (+1, -1):
        pts = np.array([v3(sx * half_width, q) for q in Q])
        nrm = np.tile([float(sx), 0.0, 0.0], (n, 1))
        uv = np.stack([Q[:, 0], -Q[:, 1]], 1) * uv_scale
        tris = np.array([[0, k, k + 1] for k in range(1, n - 1)])
        mb.add(pts, nrm, uv, tris, outward=np.array([float(sx), 0.0, 0.0]))


# APC side profile (z forward, y up), metres; chamfer segments keep edges soft
APC_PROFILE = [
    (-3.05, 0.42), (2.85, 0.42), (3.45, 0.95), (3.50, 1.08), (2.55, 1.78), (2.35, 1.86),
    (-2.80, 1.90), (-3.20, 1.78), (-3.42, 1.45), (-3.42, 0.80),
]


def build_vehicle(pos_xz, yaw, ground_y) -> list[Mesh]:
    """APC-like 8-wheeler: extruded bevelled hull, turret, barrel, wheels."""
    hull = MeshBuilder()
    extruded_hull(hull, APC_PROFILE, 1.42, 0.16)
    chamfered_box(hull, (0.66, 0.24, 0.74), 0.16, (0, 2.12, 0.10))  # turret
    cylinder(hull, 0.13, 0.30, 10, (0, 2.14, 0.96), axis="z", uv_scale=0.5)  # mantlet
    cylinder(hull, 0.065, 1.9, 10, (0, 2.14, 2.05), axis="z", uv_scale=0.5)  # barrel
    chamfered_box(hull, (0.30, 0.10, 0.36), 0.06, (-0.8, 1.98, -1.2))  # hatch
    chamfered_box(hull, (0.30, 0.10, 0.36), 0.06, (0.8, 1.98, -1.2))  # hatch
    rot = rot_y(yaw)
    off = np.array([pos_xz[0], ground_y, pos_xz[1]])
    transform(hull, rot, off)
    wheels = MeshBuilder()
    for z in (-2.3, -0.85, 0.85, 2.3):
        for sx in (+1, -1):
            cylinder(wheels, 0.55, 0.40, 20, (sx * 1.30, 0.55, z), axis="x", uv_scale=0.5)
    transform(wheels, rot, off)
    return [
        hull.build("vehicle_hull", "paint", material=VEHICLE_PAINT),
        wheels.build("vehicle_wheels", "rubber"),
    ]


def build_tree(base, scale, yaw, rng) -> list[Mesh]:
    trunk = MeshBuilder()
    th = 4.6 * scale
    cylinder(trunk, 0.24 * scale, th, 10, (0, th / 2, 0), axis="y", caps=False, uv_scale=0.5,
             radius_end=0.12 * scale)
    leaves = MeshBuilder()
    w = 5.6 * scale
    hgt = 5.4 * scale
    cy = 5.2 * scale
    for k in range(3):
        a = math.radians(k * 60.0 + rng.uniform(-8, 8))
        d = np.array([math.cos(a), 0.0, math.sin(a)])
        n = np.array([-d[2], 0.0, d[0]])
        p0 = -d * w / 2 + np.array([0, cy + hgt / 2, 0])
        p1 = d * w / 2 + np.array([0, cy + hgt / 2, 0])
        p2 = d * w / 2 + np.array([0, cy - hgt / 2, 0])
        p3 = -d * w / 2 + np.array([0, cy - hgt / 2, 0])
        uvs = [(0, 0), (1, 0), (1, 1), (0, 1)]
        leaves.quad([p0, p1, p2, p3], n, uvs)
        leaves.quad([p0, p1, p2, p3], -n, uvs)  # back face (OFP models double faces)
    # one tilted canopy card for top coverage
    tilt = np.array([0.0, 0.94, 0.34])
    tilt /= np.linalg.norm(tilt)
    e1 = np.array([1.0, 0.0, 0.0])
    e2 = np.cross(tilt, e1)
    c = np.array([0, cy + hgt * 0.22, 0])
    s = w * 0.42
    ps = [c - e1 * s - e2 * s, c + e1 * s - e2 * s, c + e1 * s + e2 * s, c - e1 * s + e2 * s]
    uvs = [(0, 0), (1, 0), (1, 1), (0, 1)]
    leaves.quad(ps, tilt, uvs)
    leaves.quad(ps, -tilt, uvs)
    rot = rot_y(yaw)
    off = np.asarray(base, dtype=np.float64)
    transform(trunk, rot, off)
    transform(leaves, rot, off)
    meshes = [
        trunk.build("tree_trunk", "bark"),
        leaves.build("tree_foliage", "foliage", material=FOLIAGE_MATERIAL, pass_id="cutout"),
    ]
    # one model in the engine: trunk and crown share the shape's bounding box
    allp = np.vstack([m.positions for m in meshes])
    for m in meshes:
        m.bounds = (allp.min(axis=0), allp.max(axis=0))
    return meshes


def sphere(mb: MeshBuilder, radius, center, rings=10, segs=16, y_from=-1.0, y_to=1.0, uv_scale=1.0,
           squash=(1.0, 1.0, 1.0), noise=None):
    lat = np.linspace(math.asin(y_from), math.asin(y_to), rings + 1)
    lon = np.linspace(0, 2 * np.pi, segs + 1)
    LAT, LON = np.meshgrid(lat, lon, indexing="ij")
    d = np.stack([np.cos(LAT) * np.cos(LON), np.sin(LAT), np.cos(LAT) * np.sin(LON)], -1)
    r = radius * (1.0 + (noise(d) if noise is not None else 0.0))
    p = d * np.asarray(squash) * (r[..., None] if np.ndim(r) else r)
    uv = np.stack([LON / (2 * np.pi) * 2 * uv_scale, (LAT - lat[0]) * radius * uv_scale], -1)
    tris = []
    for i in range(rings):
        for j in range(segs):
            a = i * (segs + 1) + j
            b = a + segs + 1
            tris += [[a, b, b + 1], [a, b + 1, a + 1]]
    tris = np.array(tris)
    P = p.reshape(-1, 3)
    # smooth normals from the (possibly displaced) surface
    vn = np.zeros_like(P)
    fa, fb, fc = P[tris[:, 0]], P[tris[:, 1]], P[tris[:, 2]]
    fnm = np.cross(fb - fa, fc - fa)
    outward = (fa + fb + fc) / 3.0
    fnm *= np.sign(np.einsum("ij,ij->i", fnm, outward))[:, None]
    for k in range(3):
        np.add.at(vn, tris[:, k], fnm)
    # seam duplicates: average normals of coincident vertices
    vn = vn.reshape(rings + 1, segs + 1, 3)
    seam = vn[:, 0] + vn[:, -1]
    vn[:, 0] = seam
    vn[:, -1] = seam
    vn[0, :] = vn[0, :].sum(0) if y_from <= -0.999 else vn[0, :]
    vn[-1, :] = vn[-1, :].sum(0) if y_to >= 0.999 else vn[-1, :]
    vn = vn.reshape(-1, 3)
    vn /= np.maximum(np.linalg.norm(vn, axis=1, keepdims=True), 1e-9)
    mb.add(P + np.asarray(center), vn, uv.reshape(-1, 2), tris, outward=fnm)


def build_jeep(pos_xz, yaw, ground_y) -> list[Mesh]:
    """Small open utility vehicle: body tub, bonnet, windscreen frame, wheels."""
    body = MeshBuilder()
    chamfered_box(body, (0.78, 0.36, 1.70), 0.10, (0, 0.92, 0))  # tub
    chamfered_box(body, (0.74, 0.16, 0.62), 0.08, (0, 1.36, 1.02))  # bonnet
    chamfered_box(body, (0.74, 0.32, 0.03), 0.02, (0, 1.68, 0.38))  # windscreen frame
    chamfered_box(body, (0.30, 0.16, 0.28), 0.05, (-0.38, 1.36, -0.55))  # seat backs
    chamfered_box(body, (0.30, 0.16, 0.28), 0.05, (0.38, 1.36, -0.55))
    rot = rot_y(yaw)
    off = np.array([pos_xz[0], ground_y, pos_xz[1]])
    transform(body, rot, off)
    wheels = MeshBuilder()
    for sx in (-0.84, 0.84):
        for sz in (-1.12, 1.12):
            cylinder(wheels, 0.40, 0.26, 16, (sx, 0.40, sz), axis="x", uv_scale=0.5)
    transform(wheels, rot, off)
    return [body.build("jeep_body", "paint", material=VEHICLE_PAINT), wheels.build("jeep_wheels", "rubber")]


def build_net(center_xz, yaw, heights_fn, width=8.0, depth=6.0, ridge=2.7, edge=0.7, n=24) -> list[Mesh]:
    """Camouflage net draped over a ridge pole: a sagging two-sided cutout
    sheet (OFP nets are half-lit, like foliage)."""
    rot = rot_y(yaw)
    xs = np.linspace(-width / 2, width / 2, n)
    zs = np.linspace(-depth / 2, depth / 2, n)
    X, Z = np.meshgrid(xs, zs)
    # tent: high along the local z ridge line, falling to the edges, with a
    # little sag between the ridge and the corners
    tx = np.abs(X) / (width / 2)
    tz = np.abs(Z) / (depth / 2)
    local_h = edge + (ridge - edge) * (1.0 - tx ** 1.4) * (1.0 - 0.35 * tz ** 2)
    world = np.stack([X, local_h, Z], axis=-1) @ rot.T
    world[..., 0] += center_xz[0]
    world[..., 2] += center_xz[1]
    for i in range(n):
        for j in range(n):
            world[i, j, 1] += heights_fn(world[i, j, 0], world[i, j, 2])
    # vertex normals from the grid
    dx = np.gradient(world, axis=1)
    dz = np.gradient(world, axis=0)
    nrm = np.cross(dz, dx)
    nrm /= np.linalg.norm(nrm, axis=-1, keepdims=True)
    nrm[nrm[..., 1] < 0] *= -1.0
    uv = np.stack([X / 3.0, Z / 3.0], axis=-1)
    idx = np.arange(n * n).reshape(n, n)
    tris = []
    for i in range(n - 1):
        for j in range(n - 1):
            a, b, c, d = idx[i, j], idx[i, j + 1], idx[i + 1, j + 1], idx[i + 1, j]
            tris += [[a, b, c], [a, c, d]]
    tris = np.array(tris)
    mb = MeshBuilder()
    P, N, U = world.reshape(-1, 3), nrm.reshape(-1, 3), uv.reshape(-1, 2)
    mb.add(P, N, U, tris)
    mb.add(P, -N, U, tris)  # back face (OFP models double faces)
    return [mb.build("camo_net", "camonet", material=FOLIAGE_MATERIAL, pass_id="cutout")]


def build_soldier(base, yaw) -> list[Mesh]:
    body = MeshBuilder()
    r = 0.23
    cylinder(body, r, 1.05, 14, (0, 0.30 + 0.525, 0), axis="y", caps=False, uv_scale=1.0)
    sphere(body, r, (0, 0.30, 0), rings=5, segs=14, y_from=-1.0, y_to=0.0)  # hips/legs end
    sphere(body, r, (0, 1.35, 0), rings=5, segs=14, y_from=0.0, y_to=1.0)  # shoulders
    head = MeshBuilder()
    sphere(head, 0.12, (0, 1.68, 0.02), rings=8, segs=12)
    rot = rot_y(yaw)
    off = np.asarray(base, dtype=np.float64)
    # sink the bottom cap to the ground
    off = off + np.array([0.0, -0.07, 0.0])
    transform(body, rot, off)
    transform(head, rot, off)
    return [body.build("soldier_body", "uniform"), head.build("soldier_head", "skin")]


def build_rock(base, radius, rng) -> Mesh:
    mb = MeshBuilder()
    k = rng.normal(size=(6, 3))
    ph = rng.uniform(0, 2 * np.pi, 6)

    def noise(d):
        return 0.18 * np.sum([np.sin(d @ (k[i] * 2.2) + ph[i]) for i in range(6)], axis=0) / 3.0

    sphere(mb, radius, (0, 0, 0), rings=9, segs=14, squash=(1.0, 0.62, 0.85), noise=noise, uv_scale=0.6)
    transform(mb, rot_y(rng.uniform(0, 360)), np.asarray(base) + np.array([0, radius * 0.18, 0]))
    return mb.build("rock", "rock")


def build_crate(base, size, yaw) -> Mesh:
    mb = MeshBuilder()
    h = size / 2
    for ax in range(3):
        for s in (+1, -1):
            n = np.zeros(3)
            n[ax] = s
            o = [k for k in range(3) if k != ax]
            corners = []
            for t1, t2 in ((-1, -1), (1, -1), (1, 1), (-1, 1)):
                p = np.zeros(3)
                p[ax] = s * h
                p[o[0]] = t1 * h
                p[o[1]] = t2 * h
                corners.append(p)
            uvs = [(0, 0), (1, 0), (1, 1), (0, 1)]
            mb.quad(np.array(corners) + np.array([0, h, 0]), n, uvs)
    transform(mb, rot_y(yaw), np.asarray(base))
    return mb.build("crate", "crate")


def build_sky_dome(radius=500.0, rings=18, segs=48) -> Mesh:
    """Camera-relative hemisphere (elevation 0..90). Normals point outward
    (DoSkyLighting: cosSun = -sunDir . normal). v = elevation / 90 (0 = horizon)."""
    el = np.linspace(0.0, 0.5 * np.pi, rings + 1)
    az = np.linspace(0.0, 2 * np.pi, segs + 1)
    EL, AZ = np.meshgrid(el, az, indexing="ij")
    d = np.stack([np.cos(EL) * np.sin(AZ), np.sin(EL), np.cos(EL) * np.cos(AZ)], -1).reshape(-1, 3)
    uv = np.stack([AZ / (2 * np.pi) * 4.0, EL / (0.5 * np.pi)], -1).reshape(-1, 2)
    tris = []
    for i in range(rings):
        for j in range(segs):
            a = i * (segs + 1) + j
            b = a + segs + 1
            tris += [[a, b, b + 1], [a, b + 1, a + 1]]
    mb = MeshBuilder()
    # Seen from inside these are back faces; renderers draw the sky with
    # culling off (the engine's sky object is drawn ClipAll & ~ClipBack).
    mb.add(d * radius, d, uv, np.array(tris), outward=None)
    return mb.build("sky", "sky")


def sun_billboards(to_sun: np.ndarray, cam_up: np.ndarray, distance=120.0, disc_deg=4.0, halo_deg=34.0):
    """Camera-relative quads for the sun disc and halo (Landscape::DrawSky puts
    the sun object 120 m from the camera along -SunDirection). Returns
    [(name, positions(4,3), uvs(4,2))], halo first (drawn behind the disc)."""
    f = to_sun / np.linalg.norm(to_sun)
    up = cam_up - f * np.dot(cam_up, f)
    if np.linalg.norm(up) < 1e-4:
        up = np.array([0.0, 0.0, 1.0]) - f * f[2]
    up /= np.linalg.norm(up)
    right = np.cross(up, f)
    c = f * distance
    out = []
    for name, deg in (("sun_halo", halo_deg), ("sun_disc", disc_deg)):
        r = distance * math.tan(math.radians(deg / 2))
        p = np.array([c - right * r + up * r, c + right * r + up * r, c + right * r - up * r, c - right * r - up * r])
        out.append((name, p, np.array([(0, 0), (1, 0), (1, 1), (0, 1)], dtype=np.float64)))
    return out


HORIZON_MATERIAL = MaterialRecipe(kind="constant", factor=0.0)  # vertices flagged MSInShadow


def horizon_polygon(cam_pos, cam_forward, c_far: float, c_left: float, sea_level: float = 0.0):
    """Landscape::DrawHorizont: the 8-point background polygon at the far plane.

    Model space: z along the camera direction flattened to the horizontal, y up.
    Points 4/5 at topY = 0.06 backZ, 2/3 and 6/7 at midY = 0.03 backZ, 0/1 at
    botY (down to sea level and beyond, then moved onto the far plane along
    the camera ray). Returns camera-relative (positions (8,3), uvs (8,2),
    triangles (4,3)). The original model's UV layout is unknown; the lab maps
    the upper quad to v 0..0.5 and the lower quad to v 0.5..1 of its horizon
    texture, u repeating 8x across the width.
    """
    d = np.asarray(cam_forward, dtype=np.float64)
    d = d / np.linalg.norm(d)
    flat = np.array([d[0], 0.0, d[2]])
    if np.linalg.norm(flat) < 1e-6:
        flat = np.array([0.0, 0.0, 1.0])
    flat /= np.linalg.norm(flat)
    up = np.array([0.0, 1.0, 0.0])
    aside = np.cross(up, flat)
    extend_down = max(-d[1], 0.0) + 0.02
    camera_above = float(cam_pos[1]) - sea_level
    back_z = c_far * 0.99
    pos_x = back_z * c_left * 2.0
    top_y, mid_y = back_z * 0.06, back_z * 0.03
    bot_y = -camera_above - extend_down * back_z
    local = np.array([
        (-pos_x, bot_y, back_z), (pos_x, bot_y, back_z),
        (-pos_x, mid_y, back_z), (-pos_x, mid_y, back_z),
        (-pos_x, top_y, back_z), (pos_x, top_y, back_z),
        (pos_x, mid_y, back_z), (pos_x, mid_y, back_z),
    ])
    R = np.stack([aside, up, flat], axis=1)
    world = local @ R.T  # camera-relative (horPos = camera position)
    # move points 0/1 onto the far plane (normal = camera direction, at back_z)
    for i in (0, 1):
        along = float(world[i] @ d)
        if abs(along) > 1e-6:
            world[i] = world[i] * (back_z / along)
    uvs = np.array([(0, 1.0), (8, 1.0), (0, 0.5), (0, 0.5), (0, 0.0), (8, 0.0), (8, 0.5), (8, 0.5)], dtype=np.float64)
    tris = np.array([[0, 1, 6], [0, 6, 2], [3, 7, 5], [3, 5, 4]])
    # orient toward the camera (outward = -flat)
    a, b, c = world[tris[:, 0]], world[tris[:, 1]], world[tris[:, 2]]
    flip = np.einsum("ij,j->i", np.cross(b - a, c - a), -flat) < 0
    tris[flip] = tris[flip][:, [0, 2, 1]]
    return world, uvs, tris


# ---------------------------------------------------------------------------
# Scene assembly
# ---------------------------------------------------------------------------


@dataclass
class Scene:
    meshes: list[Mesh]
    sky: Mesh
    textures: dict[str, dict]
    cameras: dict[str, CameraPreset]
    heightfield: Heightfield
    anchors: dict[str, np.ndarray]
    seed: int

    def sky_texture(self, sky_through: float) -> np.ndarray:
        """Sky texture for a weather state: jasno (clear) blended toward zatazeno
        (overcast) as skyThrough falls (Weather basics: through 1.0 / 0.7 / 0.1)."""
        overcast = float(np.clip((1.0 - sky_through) / 0.9, 0.0, 1.0))
        key = round(overcast, 3)
        cache = self.textures.setdefault("_sky_cache", {})
        if key not in cache:
            cache[key] = T.sky(256, overcast, seed=91 + self.seed)
        return cache[key]

    def sky_texture_color(self, sky_through: float) -> np.ndarray:
        """Scene::CalculateSkyColor: _skyColor = sky texture GetPixel(0, 1, 1)."""
        px = self.sky_texture(sky_through)[1, 1]
        return np.round(px * 255.0) / 255.0  # the engine reads the 8-bit texel

    def stats(self) -> dict:
        tri = sum(m.n_tris for m in self.meshes)
        return {"meshes": len(self.meshes), "triangles": tri, "sky_triangles": self.sky.n_tris}


def build_scene(seed: int = 1) -> Scene:
    rng = np.random.default_rng(seed)
    village = np.array([300.0, 300.0])
    hf = Heightfield(seed, village=tuple(village))

    def gy(x, z):
        return float(hf(x, z))

    # main ground camera: looks NW (az 325); sun presets at az 235 come from the left
    cam_az = 325.0
    f = heading(cam_az)
    right = np.array([f[2], 0.0, -f[0]])  # aside = up x forward
    cam_xz = village + np.array([12.0, -28.0])

    def at(fwd, side):
        p = cam_xz + f[[0, 2]] * fwd + right[[0, 2]] * side
        return p

    house1 = at(27.0, -8.0)
    house2 = at(52.0, 15.0)
    vehicle = at(14.0, 5.0)
    soldier = at(10.5, 1.2)
    net_site = at(12.0, -6.5)  # jeep under a camouflage net (the owner's test view)

    # 50 m texture squares that get the dirt texture: the village yard and
    # the valley floor
    dirt = set()
    for p in (house1, house2):
        dirt.add((int(p[0] // TEX_SQUARE), int(p[1] // TEX_SQUARE)))
    for zq in range(-2, 14):
        z = (zq + 0.5) * TEX_SQUARE
        vx = 200.0 + 35.0 * math.sin(z / 95.0)
        dirt.add((int(vx // TEX_SQUARE), zq))

    meshes: list[Mesh] = []
    meshes += build_terrain(hf, dirt)
    meshes += build_house(house1, 20.0, 9.0, 6.5, 3.0, 2.3, gy(*house1))
    meshes += build_house(house2, -25.0, 7.0, 5.5, 2.8, 2.0, gy(*house2))
    # three-quarter view: nose toward the camera's right-front, so the sunlit
    # right flank (normal az ~190) and the shaded nose (az ~100) both show
    veh_yaw = (cam_az + 135.0) % 360.0
    meshes += build_vehicle(vehicle, veh_yaw, gy(*vehicle))
    meshes += build_soldier((soldier[0], gy(*soldier), soldier[1]), cam_az + 160.0)
    meshes += build_jeep(net_site, cam_az + 70.0, gy(*net_site))
    meshes += build_net(net_site, cam_az + 70.0, gy)

    tree_spots = [
        (at(36.0, -19.0), 1.25), (at(31.0, 3.0), 1.0), (at(44.0, -2.0), 1.15), (at(20.0, 16.0), 0.9),
        (at(65.0, -12.0), 1.3), (at(70.0, 30.0), 1.1), (at(24.0, -22.0), 0.6), (at(15.0, -14.0), 0.45),
        (village + np.array([-60.0, 40.0]), 1.2), (village + np.array([-90.0, -20.0]), 1.3),
        (village + np.array([80.0, 60.0]), 1.0), (village + np.array([-30.0, 110.0]), 1.4),
    ]
    # a loose copse on the hill toward the overview camera's view
    for _ in range(14):
        p = np.array([rng.uniform(120, 260), rng.uniform(130, 260)])
        tree_spots.append((p, rng.uniform(0.9, 1.4)))
    for p, s in tree_spots:
        meshes += build_tree((p[0], gy(*p) - 0.1, p[1]), s, rng.uniform(0, 360), rng)

    for p, r in [(at(8.0, -6.0), 0.7), (at(18.0, -3.5), 0.5), (at(33.0, 9.0), 1.1), (at(12.0, 11.0), 0.6)]:
        meshes.append(build_rock((p[0], gy(*p), p[1]), r, rng))
    c0 = house1 + rot_y(20.0)[[0, 2]][:, [0, 2]] @ np.array([5.6, 2.0])
    for k, (dx, dz, dy) in enumerate([(0, 0, 0), (1.05, 0.1, 0), (0.5, 0.05, 1.0)]):
        p = c0 + np.array([dx, dz])
        meshes.append(build_crate((p[0], gy(*c0) + dy - 0.02, p[1]), 1.0, 20.0 + k * 7))

    eye = 1.8
    cam_pos = np.array([cam_xz[0], gy(*cam_xz) + eye, cam_xz[1]])
    look = np.array([*at(30.0, 0.0)])
    look3 = np.array([look[0], gy(*look) + 1.6, look[1]])
    ground_fwd = look3 - cam_pos

    over_xz = np.array([475.0, 480.0])
    over_pos = np.array([over_xz[0], gy(*over_xz) + 40.0, over_xz[1]])
    over_fwd = heading(235.0) + np.array([0.0, -0.12, 0.0])

    veh3 = np.array([vehicle[0], gy(*vehicle) + 1.2, vehicle[1]])
    close_dir = heading(veh_yaw + 180.0 + 50.0)  # front-right three-quarter of the hull
    close_pos = veh3 - close_dir * 6.0
    close_pos[1] = gy(close_pos[0], close_pos[2]) + 2.0
    close_fwd = veh3 - close_pos

    # first-person view in the spirit of the owner's LightingLowSun.Demo test
    # view: the APC, the soldier and a jeep under a camouflage net, 1.8 m eye
    tv_xz = at(2.0, -1.0)
    tv_pos = np.array([tv_xz[0], gy(*tv_xz) + eye, tv_xz[1]])
    tv_look = at(13.0, -0.5)
    tv_fwd = np.array([tv_look[0], gy(*tv_look) + 1.2, tv_look[1]]) - tv_pos

    cameras = {
        "testview": CameraPreset("testview", tv_pos, tv_fwd, description="first person: APC, soldier, jeep under a net"),
        "ground": CameraPreset("ground", cam_pos, ground_fwd, description="1.8 m eye height, house + vehicle"),
        "overview": CameraPreset("overview", over_pos, over_fwd, description="40 m up, long view toward az 235"),
        "closeup": CameraPreset("closeup", close_pos, close_fwd, description="vehicle at 6 m"),
    }
    anchors = {"village": village, "house1": house1, "house2": house2, "vehicle": vehicle, "soldier": soldier,
               "net": net_site}
    return Scene(meshes, build_sky_dome(), T.build_all(seed - 1), cameras, hf, anchors, seed)


if __name__ == "__main__":
    import time

    t0 = time.perf_counter()
    sc = build_scene()
    print(f"built in {time.perf_counter() - t0:.2f}s:", sc.stats())
    for m in sc.meshes[:6]:
        print(m.name, m.positions.shape, m.n_tris, m.texture, m.shader, m.pass_id)
    for k, c in sc.cameras.items():
        print(k, np.round(c.position, 2), np.round(c.forward / np.linalg.norm(c.forward), 3))
