"""Pure Python reader and writer for OFP runtime animations (RTM).

The layout mirrors what the engine reads:
``engine/Poseidon/Asset/Formats/RTM/RTMReader.hpp`` (``readRTMHeader``,
``readRTMPhases``) as used by ``AnimationRT::Load`` / ``FullLoad`` in
``engine/Poseidon/World/Simulation/Animation/RtAnimation.cpp``.

File layout, little endian::

    char  magic[8]        "RTM_0101" (or the older "RTM_0100")
    RTM_0101: float stepX, stepY, stepZ
    RTM_0100: float stepZ
    int32 nPhases  int32 nBones          (engine limits: 0..100000, 0..1000)
    nBones x char name[32]               header bone names (the engine uses these)
    nPhases x {
        float time
        nBones x { char name[32]; float m[12] }   per-phase name is skipped
    }

``m`` is a ``Matrix4P``: ``[aside.xyz, up.xyz, dir.xyz, pos.xyz]``. The first
three triples are the COLUMNS of the 3x3 part, so a point maps as
``p' = aside * p.x + up * p.y + dir * p.z + pos`` (``Vector3P::SetMultiply``).

Space: the matrices act on the model's file space (MLOD coordinates, X, Y up,
Z) and are absolute, not parent relative: a vertex weighted to bone b lands at
``M_b * v_rest``. Stock men stand facing +X in that rest space and every stock
RTM turns them to face -Z with the feet near y = 0. The engine loads a man's
model and its RTMs ``reversed`` (both conjugated by diag(-1, 1, -1)), which
only turns the whole result 180 degrees so the unit faces engine +Z.

Step: the unit advances by ``+step`` (file space) per full cycle; a walk
forward has a negative stepZ (``Man::AdvanceMoveQueue``: moveZ -= phase * stepZ,
in the reversed engine frame). The animation itself stays in place, so a
planted foot slides by ``-step`` per cycle in the matrices.

Times: one cycle spans phase time 0..1. The engine interpolates matrices
linearly between neighbouring phases and, for looped moves, drops the first
phase after index 0 whose time is above 0.999 (``RemoveLoopFrame``), then
wraps from the last phase back to phase 0.

This module does not import bpy, so it can be unit tested with plain Python.
"""

import math
import os
import struct
from dataclasses import dataclass, field

MAGIC_V100 = b"RTM_0100"
MAGIC_V101 = b"RTM_0101"
BONE_NAME_LEN = 32
MATRIX_FLOATS = 12
MAX_PHASES = 100000  # readRTMHeader limit
MAX_BONES = 1000  # readRTMHeader limit
MAX_SKELETON_BONES = 128  # AnimationRT::Apply uses MATRIX_4_ARRAY(matrix, 128)
LOOP_FRAME_TIME = 0.999  # AnimationRT::RemoveLoopFrame

IDENTITY = (1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0)

# The 25 bones every stock man RTM carries, in the order of civilbeh-45l.rtm.
# p = right, l = left. roura and zbran are the launcher and weapon bones.
STOCK_MAN_BONES = (
    "pchodidlo", "lchodidlo", "pprsty", "lprsty", "lholen", "pholen", "pstehno", "lstehno",
    "pzadek", "lzadek", "bricho", "zebra", "hrudnik", "krk", "prameno", "lrameno", "hlava",
    "pbiceps", "lbiceps", "ploket", "lloket", "roura", "zbran", "pruka", "lruka",
)

# Bones the engine looks up by name on an isMan unit and complains about when the
# moves skeleton lacks them (SoldierOldSimProxy.cpp, ManType::InitShape).
ENGINE_REQUIRED_MAN_BONES = ("zbran", "roura", "lruka", "pruka", "hlava")


class RtmError(ValueError):
    """The data would be rejected by the engine (or by the strict checks)."""


# ---------------------------------------------------------------------------
# Small matrix helpers on the 12-float file layout


def mat_apply(m, p):
    """Engine ``Matrix4P * Vector3P``: transform point p by the 12-float matrix m."""
    x, y, z = p
    return (
        m[0] * x + m[3] * y + m[6] * z + m[9],
        m[1] * x + m[4] * y + m[7] * z + m[10],
        m[2] * x + m[5] * y + m[8] * z + m[11],
    )


def mat_rotate(m, v):
    """Orientation part only (``Matrix3P * Vector3P``), for directions and normals."""
    x, y, z = v
    return (
        m[0] * x + m[3] * y + m[6] * z,
        m[1] * x + m[4] * y + m[7] * z,
        m[2] * x + m[5] * y + m[8] * z,
    )


def mat_get(m, i, j):
    """Element row i, column j (``Matrix4P::Get``); column 3 is the position."""
    return m[3 * j + i]


def mat_from_rows(rows):
    """12-float matrix from 3 rows of 4 (row-major 3x4, translation in column 3)."""
    return tuple(float(rows[i][j]) for j in range(4) for i in range(3))


def mat_to_rows(m):
    """3 rows of 4 (row-major 3x4) from a 12-float matrix."""
    return [[mat_get(m, i, j) for j in range(4)] for i in range(3)]


def mat_mul(a, b):
    """Engine ``Matrix4P::SetMultiply(a, b)``: the matrix of ``a * (b * p)``."""
    r = [0.0] * 12
    for j in range(3):
        for i in range(3):
            r[3 * j + i] = a[i] * b[3 * j] + a[3 + i] * b[3 * j + 1] + a[6 + i] * b[3 * j + 2]
    for i in range(3):
        r[9 + i] = a[i] * b[9] + a[3 + i] * b[10] + a[6 + i] * b[11] + a[9 + i]
    return tuple(r)


def mat_det(m):
    """Determinant of the 3x3 part. Negative means a mirror."""
    a, b, c = m[0:3], m[3:6], m[6:9]
    return (a[0] * (b[1] * c[2] - b[2] * c[1])
            - b[0] * (a[1] * c[2] - a[2] * c[1])
            + c[0] * (a[1] * b[2] - a[2] * b[1]))


def mat_orthonormal_error(m):
    """Largest deviation of the 3x3 columns from an orthonormal basis."""
    cols = (m[0:3], m[3:6], m[6:9])
    err = 0.0
    for i in range(3):
        for j in range(3):
            dot = sum(cols[i][k] * cols[j][k] for k in range(3))
            err = max(err, abs(dot - (1.0 if i == j else 0.0)))
    return err


# ---------------------------------------------------------------------------
# Data model


@dataclass
class RtmFrame:
    """One phase (keyframe): its time in 0..1 and one 12-float matrix per bone."""

    time: float
    matrices: list
    # Raw 32-byte per-phase bone name fields, kept for byte-exact rewrites. The
    # engine skips them; the writer regenerates them from the header names.
    raw_names: list = None


@dataclass
class RtmAnimation:
    bone_names: list
    frames: list = field(default_factory=list)
    step: tuple = (0.0, 0.0, 0.0)
    magic: bytes = MAGIC_V101
    # Raw 32-byte header name fields as read (bytes after the NUL kept), and any
    # bytes after the last phase. Only used for byte-exact rewrites of read files.
    raw_bone_names: list = None
    trailing: bytes = b""
    warnings: list = field(default_factory=list)

    @property
    def bone_count(self):
        return len(self.bone_names)

    @property
    def frame_count(self):
        return len(self.frames)

    @property
    def times(self):
        return [f.time for f in self.frames]

    def bone_index(self, name):
        """Index of a bone, case-insensitive like the engine (names are lowercased)."""
        low = name.lower()
        for i, n in enumerate(self.bone_names):
            if n.lower() == low:
                return i
        return -1

    def matrix(self, frame, bone):
        if isinstance(bone, str):
            idx = self.bone_index(bone)
            if idx < 0:
                raise KeyError(bone)
            bone = idx
        return self.frames[frame].matrices[bone]


# ---------------------------------------------------------------------------
# Reading


def _name_from_field(raw):
    """What the engine keeps of a 32-byte name field: up to the first NUL, lowercased."""
    text = raw.split(b"\0", 1)[0]
    return text.decode("latin-1")


def read_rtm(source, *, strict=False):
    """Read an RTM from a path, bytes or bytearray.

    Raises RtmError whenever ``readRTM`` would return false (bad magic,
    truncation, negative or too large counts). Names are returned as stored
    (the engine lowercases them; ``bone_index`` compares case-insensitively).
    With ``strict=True`` the semantic checks of ``check_rtm`` also run: errors
    raise, warnings land in ``anim.warnings``.
    """
    if isinstance(source, (bytes, bytearray, memoryview)):
        data = bytes(source)
    else:
        with open(os.fspath(source), "rb") as f:
            data = f.read()

    off = 0

    def need(n, what):
        if off + n > len(data):
            raise RtmError(f"truncated: {what} needs {n} bytes at offset {off}, file has {len(data)}")

    need(8, "magic")
    magic = data[0:8]
    off = 8
    # strcmp on a 9-byte NUL-terminated buffer: the 8 bytes must match exactly.
    if magic == MAGIC_V100:
        need(4, "step")
        (sz,) = struct.unpack_from("<f", data, off)
        off += 4
        step = (0.0, 0.0, sz)
    elif magic == MAGIC_V101:
        need(12, "step")
        step = struct.unpack_from("<3f", data, off)
        off += 12
    else:
        raise RtmError(f"bad magic {magic!r} (expected RTM_0101 or RTM_0100)")
    need(8, "counts")
    n_phases, n_bones = struct.unpack_from("<2i", data, off)
    off += 8
    if n_phases < 0 or n_bones < 0:
        raise RtmError(f"negative counts: {n_phases} phases, {n_bones} bones")
    if n_phases > MAX_PHASES or n_bones > MAX_BONES:
        raise RtmError(f"counts over the engine limits: {n_phases} phases (max {MAX_PHASES}), "
                       f"{n_bones} bones (max {MAX_BONES})")
    raw_names = []
    for i in range(n_bones):
        need(BONE_NAME_LEN, f"bone name {i}")
        raw_names.append(data[off:off + BONE_NAME_LEN])
        off += BONE_NAME_LEN
    names = [_name_from_field(r) for r in raw_names]

    frames = []
    rec = BONE_NAME_LEN + 4 * MATRIX_FLOATS
    for p in range(n_phases):
        need(4, f"phase {p} time")
        (t,) = struct.unpack_from("<f", data, off)
        off += 4
        need(rec * n_bones, f"phase {p} bones")
        mats = []
        pnames = []
        for _ in range(n_bones):
            pnames.append(data[off:off + BONE_NAME_LEN])
            off += BONE_NAME_LEN
            mats.append(struct.unpack_from("<12f", data, off))
            off += 4 * MATRIX_FLOATS
        frames.append(RtmFrame(time=t, matrices=mats, raw_names=pnames))

    anim = RtmAnimation(bone_names=names, frames=frames, step=tuple(step), magic=magic,
                        raw_bone_names=raw_names, trailing=data[off:])
    if strict:
        errors, warnings = check_rtm(anim)
        anim.warnings = warnings
        if errors:
            raise RtmError("; ".join(errors))
    return anim


def read_rtm_strict(source):
    """``read_rtm(source, strict=True)``: raise on anything the engine would reject or
    misplay, keep the rest as warnings in ``anim.warnings``."""
    return read_rtm(source, strict=True)


# ---------------------------------------------------------------------------
# Semantic checks


def check_rtm(anim, *, require_bones=(), looped=None):
    """Return (errors, warnings) for an animation.

    Errors are things the engine rejects or plays wrongly: no phases
    (``FullLoad`` asserts at least one), names it cannot hold, duplicate bones
    (``FindBone`` maps both to one matrix), unsorted times (``Find`` assumes
    ascending), non-finite numbers, and mirrored bone matrices (determinant
    below zero: the skin turns inside out). Warnings are allowed but unusual.
    ``require_bones`` lists names that must be present (for a man:
    ``STOCK_MAN_BONES``).
    """
    errors = []
    warnings = []
    if anim.magic not in (MAGIC_V100, MAGIC_V101):
        errors.append(f"bad magic {anim.magic!r}")
    if anim.magic == MAGIC_V100 and (anim.step[0] != 0.0 or anim.step[1] != 0.0):
        errors.append("RTM_0100 stores only stepZ; stepX/stepY would be lost")
    if not all(math.isfinite(s) for s in anim.step):
        errors.append(f"non-finite step {anim.step}")
    if len(anim.frames) < 1:
        errors.append("no phases (AnimationRT::FullLoad asserts at least one)")
    if len(anim.frames) > MAX_PHASES:
        errors.append(f"{len(anim.frames)} phases, engine limit {MAX_PHASES}")
    if len(anim.bone_names) > MAX_BONES:
        errors.append(f"{len(anim.bone_names)} bones, engine limit {MAX_BONES}")
    elif len(anim.bone_names) > MAX_SKELETON_BONES:
        warnings.append(f"{len(anim.bone_names)} bones; a skeleton over {MAX_SKELETON_BONES} overflows "
                        "the static matrix array in AnimationRT::Apply")

    seen = {}
    for i, n in enumerate(anim.bone_names):
        raw = n.encode("latin-1", "replace")
        if not n:
            errors.append(f"bone {i} has an empty name")
        if b"\0" in raw:
            errors.append(f"bone {i} name {n!r} contains a NUL")
        if len(raw) >= BONE_NAME_LEN:
            errors.append(f"bone {i} name {n!r} is {len(raw)} bytes; at most {BONE_NAME_LEN - 1} "
                          "leave room for the NUL older readers need")
        if n != n.lower():
            warnings.append(f"bone {i} name {n!r} is not lowercase (the engine lowercases it)")
        low = n.lower()
        if low in seen:
            errors.append(f"bone {n!r} appears twice (indices {seen[low]} and {i}); the engine maps "
                          "both to one matrix and the later one wins")
        else:
            seen[low] = i
    for req in require_bones:
        if req.lower() not in seen:
            errors.append(f"required bone {req!r} missing")

    prev = None
    nb = len(anim.bone_names)
    for p, fr in enumerate(anim.frames):
        t = fr.time
        if not math.isfinite(t):
            errors.append(f"phase {p} time is not finite")
            continue
        if prev is not None and t < prev:
            errors.append(f"phase {p} time {t} is below the previous {prev} (times must ascend)")
        if t < 0.0 or t > 1.0:
            warnings.append(f"phase {p} time {t} outside 0..1")
        prev = t
        if len(fr.matrices) != nb:
            errors.append(f"phase {p} has {len(fr.matrices)} matrices for {nb} bones")
            continue
        if fr.raw_names is not None and len(fr.raw_names) == nb:
            for b, raw in enumerate(fr.raw_names):
                if _name_from_field(raw).lower() != anim.bone_names[b].lower():
                    warnings.append(f"phase {p} bone {b} per-phase name {_name_from_field(raw)!r} differs "
                                    f"from header {anim.bone_names[b]!r} (engine uses the header)")
                    break
        for b, m in enumerate(fr.matrices):
            if len(m) != MATRIX_FLOATS:
                errors.append(f"phase {p} bone {b} matrix has {len(m)} floats")
                continue
            if not all(math.isfinite(v) for v in m):
                errors.append(f"phase {p} bone {anim.bone_names[b]} matrix is not finite")
                continue
            d = mat_det(m)
            if d < -1e-2:
                errors.append(f"phase {p} bone {anim.bone_names[b]} matrix is mirrored (det {d:.4f})")
            elif d < 1e-2:
                warnings.append(f"phase {p} bone {anim.bone_names[b]} matrix is collapsed (det {d:.4f})")
    if anim.frames:
        t0 = anim.frames[0].time
        if t0 != 0.0:
            warnings.append(f"first phase time is {t0}, not 0")
        tl = anim.frames[-1].time
        if len(anim.frames) > 1 and tl < LOOP_FRAME_TIME:
            warnings.append(f"last phase time is {tl}; stock cycles end at 1.0")
        if looped:
            loop = [i for i, f in enumerate(anim.frames) if i > 0 and f.time > LOOP_FRAME_TIME]
            if loop and loop[0] != len(anim.frames) - 1:
                warnings.append(f"looped: the engine drops phase {loop[0]} and every phase after it")
    if anim.trailing:
        warnings.append(f"{len(anim.trailing)} bytes after the last phase (ignored by the engine)")
    return errors, warnings


# ---------------------------------------------------------------------------
# Writing


def _name_field(name, raw=None):
    if raw is not None and len(raw) == BONE_NAME_LEN and _name_from_field(raw) == name:
        return bytes(raw)
    enc = name.encode("latin-1")
    if len(enc) >= BONE_NAME_LEN or b"\0" in enc or not enc:
        raise RtmError(f"bone name {name!r} must be 1..{BONE_NAME_LEN - 1} bytes without NUL")
    return enc + b"\0" * (BONE_NAME_LEN - len(enc))


def write_rtm(anim, path=None, *, verify=True, require_bones=(), looped=None):
    """Serialize ``anim`` to bytes (and to ``path`` when given). Returns the bytes.

    Raw name fields and trailing bytes read from a file are written back as
    they were, so read -> write is byte exact. With ``verify=True`` the
    result is read back strictly first and ``check_rtm`` errors raise RtmError
    before anything is written.
    """
    if anim.magic not in (MAGIC_V100, MAGIC_V101):
        raise RtmError(f"bad magic {anim.magic!r}")
    if verify:
        errors, _ = check_rtm(anim, require_bones=require_bones, looped=looped)
        if errors:
            raise RtmError("; ".join(errors))
    nb = len(anim.bone_names)
    raw_header = anim.raw_bone_names if anim.raw_bone_names and len(anim.raw_bone_names) == nb else [None] * nb
    header_fields = [_name_field(n, r) for n, r in zip(anim.bone_names, raw_header)]
    out = bytearray()
    out += anim.magic
    if anim.magic == MAGIC_V101:
        out += struct.pack("<3f", *anim.step)
    else:
        if anim.step[0] != 0.0 or anim.step[1] != 0.0:
            raise RtmError("RTM_0100 stores only stepZ")
        out += struct.pack("<f", anim.step[2])
    out += struct.pack("<2i", len(anim.frames), nb)
    for fld in header_fields:
        out += fld
    for p, fr in enumerate(anim.frames):
        if len(fr.matrices) != nb:
            raise RtmError(f"phase {p} has {len(fr.matrices)} matrices for {nb} bones")
        out += struct.pack("<f", fr.time)
        praw = fr.raw_names if fr.raw_names is not None and len(fr.raw_names) == nb else None
        for b in range(nb):
            if praw is not None and _name_from_field(praw[b]).lower() == anim.bone_names[b].lower():
                out += bytes(praw[b])
            else:
                out += header_fields[b]
            m = fr.matrices[b]
            if len(m) != MATRIX_FLOATS:
                raise RtmError(f"phase {p} bone {b} matrix has {len(m)} floats")
            out += struct.pack("<12f", *m)
    out += anim.trailing
    data = bytes(out)
    if verify:
        back = read_rtm(data)
        if back.bone_names != [_name_from_field(f) for f in header_fields] or len(back.frames) != len(anim.frames):
            raise RtmError("read-back mismatch")
    if path is not None:
        with open(os.fspath(path), "wb") as f:
            f.write(data)
    return data


def new_animation(bone_names, frames, step=(0.0, 0.0, 0.0)):
    """Build an RTM_0101 animation from bone names and (time, [12-float matrix per bone]) pairs."""
    return RtmAnimation(
        bone_names=list(bone_names),
        frames=[RtmFrame(time=float(t), matrices=[tuple(float(v) for v in m) for m in mats]) for t, mats in frames],
        step=tuple(float(s) for s in step),
        magic=MAGIC_V101,
    )


def describe(anim):
    """One-paragraph summary for logs."""
    t = anim.times
    return (f"{anim.magic.decode()} {anim.bone_count} bones, {anim.frame_count} phases, "
            f"times {t[0] if t else '-'}..{t[-1] if t else '-'}, step {tuple(round(s, 4) for s in anim.step)}")
