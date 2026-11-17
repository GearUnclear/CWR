"""Reads a built creature P3D and poses any of its LODs with an RTM clip, without the game.

Plain Python with numpy (works in Blender's Python too). File space throughout: x = his left,
y = up, z = back. Used by the motion renderer, the collision floor check and the seam measures.

    import p3d_pose as pp
    model = pp.load(path)                 # every LOD
    lod = model.visual[0]                 # or model.special["geometry"]
    pose = pp.Pose(lod, clip)             # clip = pp.read_clip(path.rtm)
    pts = pose.at(0.25)                   # (n, 3) posed points at phase 0.25
"""
import os
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
ADDON_PKG = os.path.normpath(os.path.join(HERE, "..", "..", "..", "..", "..", "apps", "tools", "BlenderAddon",
                                          "io_import_p3d"))
if ADDON_PKG not in sys.path:
    sys.path.insert(0, ADDON_PKG)
import mlod_writer as mw  # noqa: E402
import rtm_io  # noqa: E402

SPECIAL = {"geometry": 1e13, "memory": 1e15, "landcontact": 2e15, "roadway": 3e15, "paths": 4e15,
           "hitpoints": 5e15, "viewgeometry": 6e15, "firegeometry": 7e15}


class Lod:
    def __init__(self, raw):
        self.raw = raw
        self.resolution = raw.resolution
        self.points = np.array(raw.points, dtype=np.float64).reshape(-1, 3)
        self.tris = []
        self.tri_face = []
        self.tri_uv = []
        for fi, f in enumerate(raw.faces):
            # file order is the reverse of the engine's front-facing order
            c = list(reversed(f.corners))
            for k in range(1, len(c) - 1):
                self.tris.append((c[0][0], c[k][0], c[k + 1][0]))
                self.tri_uv.append(((c[0][2], c[0][3]), (c[k][2], c[k][3]), (c[k + 1][2], c[k + 1][3])))
                self.tri_face.append(fi)
        self.tris = np.array(self.tris, dtype=np.int64).reshape(-1, 3)
        self.tri_uv = np.array(self.tri_uv, dtype=np.float64).reshape(-1, 3, 2)
        self.tri_tex = [raw.faces[fi].texture.lower() for fi in self.tri_face]
        self.tri_flags = np.array([raw.faces[fi].flags for fi in self.tri_face], dtype=np.int64)
        self.sel = {s.name.lower(): dict(s.points) for s in raw.selections if s.points}
        self.props = {k.lower(): v for k, v in raw.properties}

    def named_points(self, name):
        s = self.sel.get(name.lower())
        return self.points[sorted(s)] if s else np.zeros((0, 3))


class Model:
    def __init__(self, lods):
        self.lods = [Lod(l) for l in lods]
        self.visual = [l for l in self.lods if l.resolution < 9e3]
        self.special = {}
        for name, spec in SPECIAL.items():
            for l in self.lods:
                if abs(l.resolution - spec) < spec * 1e-3:
                    self.special[name] = l


def load(path):
    _ver, lods, _warn = mw.read_mlod(open(path, "rb").read())
    return Model(lods)


def read_clip(path):
    return rtm_io.read_rtm(path)


def clip_matrices(anim, phase):
    """{bone: (3x3 R, t)} at phase 0..1, a straight blend of the two keys round it, like the engine."""
    frames = anim.frames
    times = [f.time for f in frames]
    if phase <= times[0]:
        a, b, k = frames[0], frames[0], 0.0
    elif phase >= times[-1]:
        a, b, k = frames[-1], frames[-1], 0.0
    else:
        a, b, k = frames[-1], frames[-1], 0.0
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
        out[name.lower()] = (m[:9].reshape(3, 3).T, m[9:12])
    return out


class Pose:
    """Skins one LOD the way the engine does: the weighted sum of M_bone * v over its bone selections."""

    def __init__(self, lod, anim):
        self.lod = lod
        self.anim = anim
        bones = [n.lower() for n in anim.bone_names]
        n = len(lod.points)
        self.bones = [b for b in bones if b in lod.sel]
        self.w = np.zeros((len(self.bones), n))
        for k, b in enumerate(self.bones):
            sel = lod.sel[b]
            idx = np.fromiter(sel.keys(), dtype=np.int64)
            self.w[k, idx] = np.fromiter(sel.values(), dtype=np.float64)
        s = self.w.sum(axis=0)
        self.unbound = s < 1e-6
        s[self.unbound] = 1.0
        self.w /= s

    def at(self, phase):
        mats = clip_matrices(self.anim, phase)
        pts = self.lod.points
        out = np.zeros_like(pts)
        for k, b in enumerate(self.bones):
            wk = self.w[k]
            idx = np.nonzero(wk)[0]
            if len(idx) == 0:
                continue
            rot, t = mats[b]
            out[idx] += (pts[idx] @ rot.T + t) * wk[idx, None]
        out[self.unbound] = pts[self.unbound]
        return out

    def main_bone(self):
        k = np.argmax(self.w, axis=0)
        return [self.bones[i] for i in k]
