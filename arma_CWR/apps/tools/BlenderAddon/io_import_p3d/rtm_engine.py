"""The engine's RTM playback math, reimplemented in pure Python for checks.

Each function names the C++ it follows, all in
``engine/Poseidon/World/Simulation/Animation/RtAnimation.cpp`` unless noted:

- ``EngineAnimation``: ``AnimationRT::Load`` / ``FullLoad`` / ``LoadFromRTM``
  (bone map from the header names into a skeleton, identity for skeleton
  bones the file lacks, optional ``reversed`` conjugation) and
  ``RemoveLoopFrame``.
- ``EngineAnimation.find`` / ``matrices``: ``AnimationRT::Find`` and
  ``PrepareMatrices`` (element-wise linear blend of the two neighbouring
  phases, wrapping for looped moves).
- ``vertex_weights``: ``Skeleton::Prepare``, ``AnimationRTWeights::AddSelection``
  and ``AnimationRTWeight::Normalize`` (at most 4 bones per vertex, weights
  quantized to 1/128).
- ``skin_point``: ``AnimationRT::ApplyMatricesComplex``.

Used by the unit tests and the Blender proofs; the add-on itself does not need it.
No bpy import.
"""

import math
import struct

try:
    from .rtm_io import IDENTITY, LOOP_FRAME_TIME, mat_apply, mat_mul
except ImportError:  # plain-Python use (tests) outside the add-on package
    from rtm_io import IDENTITY, LOOP_FRAME_TIME, mat_apply, mat_mul

# Matrix4(MScale, -1, 1, -1): the swap applied to reversed shapes and animations.
REVERSE = (-1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0)

WEIGHT_SCALE = 128.0  # RtAnimation.hpp WeightScale
MAX_WEIGHTS = 4  # ARTWMaxSize: 4 pairs per vertex


def _f32(x):
    return struct.unpack("<f", struct.pack("<f", x))[0]


def reverse_matrix(m):
    """``swapMatrix * transform * swapMatrix`` from ``LoadFromRTM`` with reversed=true."""
    return mat_mul(mat_mul(REVERSE, m), REVERSE)


def reverse_point(p):
    """``Shape::Reverse`` / ``ReverseVector``: (x, y, z) -> (-x, y, -z)."""
    return (-p[0], p[1], -p[2])


def _scale(m, f):
    return tuple(v * f for v in m)


def _add(a, b):
    return tuple(x + y for x, y in zip(a, b))


class EngineAnimation:
    """One loaded ``AnimationRT``.

    skeleton: the moves class's bone list (``Skeleton``; the union of names of
    every RTM it loads, in first-seen order). Defaults to this file's names.
    """

    def __init__(self, anim, skeleton=None, looped=True, reversed_=False):
        names = [n.lower() for n in anim.bone_names]
        if skeleton is None:
            skeleton = []
        self.skeleton = [s.lower() for s in skeleton]
        for n in names:  # Skeleton::NewBone during AnimationRT::Load
            if n not in self.skeleton:
                self.skeleton.append(n)
        bone_map = [self.skeleton.index(n) for n in names]
        bones_used = max(bone_map) + 1 if bone_map else 0
        self.step = tuple(anim.step)
        self.looped = looped
        self.phases = []
        self.times = []
        for fr in anim.frames:
            mats = [IDENTITY] * bones_used
            for i, idx in enumerate(bone_map):
                m = tuple(fr.matrices[i])
                mats[idx] = reverse_matrix(m) if reversed_ else m
            self.phases.append(mats)
            self.times.append(fr.time)
        if looped:  # RemoveLoopFrame
            for i in range(len(self.phases)):
                if self.times[i] > LOOP_FRAME_TIME and i > 0:
                    del self.phases[i:]
                    del self.times[i:]
                    break

    def find(self, time):
        """``AnimationRT::Find``: returns (next_index, prev_index)."""
        n = len(self.phases)
        n_index = 0
        while n_index < n:
            if self.times[n_index] > time:
                break
            n_index += 1
        p_index = n_index - 1
        if p_index < 0:
            p_index = n - 1 if self.looped else 0
        if n_index >= n:
            n_index = 0 if self.looped else n - 1
        return n_index, p_index

    def interpolation(self, time):
        """(prev_index, next_index, interpol) exactly as ``PrepareMatrices`` computes them."""
        n_index, p_index = self.find(time)
        next_time = self.times[n_index]
        prev_time = self.times[p_index]
        if next_time <= time:
            next_time += 1
        if prev_time > time:
            prev_time -= 1
        interpol = (time - prev_time) / (next_time - prev_time) if next_time > prev_time else 1.0
        return p_index, n_index, interpol

    def matrices(self, time, factor=1.0):
        """``PrepareMatrices`` into a fresh array: one 12-float matrix per skeleton bone."""
        p_index, n_index, interpol = self.interpolation(time)
        f1 = (1 - interpol) * factor
        f2 = interpol * factor
        prev = self.phases[p_index]
        nxt = self.phases[n_index]
        out = []
        for i in range(len(self.skeleton)):
            if i < len(prev):
                out.append(_add(_scale(prev[i], f1), _scale(nxt[i], f2)))
            else:
                out.append(_scale(IDENTITY, factor))
        return out

    def bone_matrix(self, bone, time):
        return self.matrices(time)[self.skeleton.index(bone.lower())]


def vertex_weights(selections, skeleton, vertex_count):
    """Per-vertex [(skeleton bone index, weight)] as the engine builds them.

    selections: {selection name: {vertex index: weight 0..1}} for one LOD, with
    the weight as the DLL returns it (O2 byte / 255). Bones are matched to
    selections by name, case-insensitive (``Shape::FindNamedSel``), in skeleton
    order (``Skeleton::Prepare``).
    """
    lower = {k.lower(): v for k, v in selections.items()}
    data = [[] for _ in range(vertex_count)]  # [bone, quantized byte]
    inv255 = _f32(1.0 / 255)
    for bi, bone in enumerate(skeleton):
        sel = lower.get(bone.lower())
        if not sel:
            continue
        for vi, w in sel.items():  # AddSelection
            byte = int(round(w * 255))
            sel_w = _f32(byte * inv255)
            q = int(math.floor(_f32(sel_w * WEIGHT_SCALE)))
            entry = data[vi]
            if len(entry) < MAX_WEIGHTS:
                entry.append([bi, q])
            else:
                min_iw = sel_w
                min_i = -1
                for i, (_, iq) in enumerate(entry):
                    iw = iq / WEIGHT_SCALE
                    if min_iw > iw:
                        min_iw, min_i = iw, i
                if min_i > 0:  # the engine never replaces entry 0
                    entry[min_i] = [bi, q]
    out = []
    for entry in data:  # AnimationRTWeight::Normalize
        n = len(entry)
        if n == 0:
            out.append([])
        elif n == 1:
            out.append([(entry[0][0], 1.0)])
        else:
            total = sum(q / WEIGHT_SCALE for _, q in entry)
            if total <= 0:
                out.append([(b, 0.0) for b, _ in entry])
                continue
            inv = 1 / total
            rest = 1.001
            res = [None] * n
            for i in range(1, n):
                w = entry[i][1] / WEIGHT_SCALE * inv
                qw = math.floor(w * WEIGHT_SCALE) / WEIGHT_SCALE
                res[i] = (entry[i][0], qw)
                rest -= qw
            res[0] = (entry[0][0], math.floor(rest * WEIGHT_SCALE) / WEIGHT_SCALE)
            out.append(res)
    return out


def skin_point(p, weights, matrices):
    """``ApplyMatricesComplex`` for one point: sum of w_i * (M_i * p); unweighted points stay."""
    if not weights:
        return tuple(p)
    x = y = z = 0.0
    for bi, w in weights:
        q = mat_apply(matrices[bi], p)
        x += q[0] * w
        y += q[1] * w
        z += q[2] * w
    return (x, y, z)


def skin(points, weights, matrices):
    return [skin_point(p, w, matrices) for p, w in zip(points, weights)]


def estimate_step(anim, feet, ground_band=0.02):
    """Estimate the step from planted feet, using the engine's own semantics.

    feet: {foot: [(bone, contact point in rest file space), ...]}, for a man
    ``rtm_rig.man_foot_contacts(rest)`` (ankle and toe joint of each foot).
    A contact point counts as planted on a phase-to-phase segment when it stays
    within ``ground_band`` of the lowest contact height of that foot. The unit
    advances by +step per cycle, so a planted point slides by -step. Returns
    (x, 0, z) per cycle averaged over the feet, or None if nothing was planted.
    Good to about 5 % on the stock walks and runs; not meant for crawls.
    """
    times = anim.times
    acc = []
    for contacts in feet.values():
        tracks = []
        for bone, point in contacts:
            bi = anim.bone_index(bone)
            if bi >= 0:
                tracks.append([mat_apply(fr.matrices[bi], point) for fr in anim.frames])
        if not tracks:
            continue
        low = min(p[1] for tr in tracks for p in tr)
        vx = []
        vz = []
        for i in range(len(times) - 1):
            dt = times[i + 1] - times[i]
            if dt <= 0:
                continue
            for tr in tracks:
                if tr[i][1] < low + ground_band and tr[i + 1][1] < low + ground_band:
                    vx.append((tr[i + 1][0] - tr[i][0]) / dt)
                    vz.append((tr[i + 1][2] - tr[i][2]) / dt)
        if vx:
            acc.append((-sum(vx) / len(vx), -sum(vz) / len(vz)))
    if not acc:
        return None
    return (sum(a[0] for a in acc) / len(acc), 0.0, sum(a[1] for a in acc) / len(acc))
