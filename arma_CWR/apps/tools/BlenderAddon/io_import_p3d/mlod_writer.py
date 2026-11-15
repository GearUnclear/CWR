"""Pure Python writer and strict reader for OFP-era MLOD P3D files.

The layout mirrors what the engine reads at runtime:
``LODShape::Load`` (engine/Poseidon/Graphics/Rendering/Shape/ShapeLOD.cpp) and
``Shape::LoadTagged`` (ShapeSetup.cpp), with the structs from
``engine/Poseidon/Core/Data3D.h``. Only the OFP-era SP3X LOD layout is written;
the Arma-era P3DM layout is not loadable by this engine.

File layout::

    "MLOD"  int version (0x0101)  int nLods
    per LOD:
      "SP3X"  int headSize=28  int version  int nPos  int nNorm  int nFace  int flags
      nPos  x {float x, y, z; int flags}
      nNorm x {float x, y, z}
      nFace x {char texture[32]; int n; 4 x {int point, int normal, float u, float v}; int flags}
      "TAGG"
      tags: {char name[64]; int size; size bytes}   ending with "#EndOfFile#" (size 0)
      float resolution

Coordinates are game space: X right, Y up, Z forward. Face corners are stored
in file order; the engine reverses every face on load (``poly.Reverse()``), so
a face that should be front facing in the engine is written in the reverse of
its engine order. ``file_order`` does that conversion.

This module does not import bpy, so it can be unit tested with plain Python.
"""

import math
import struct
from dataclasses import dataclass, field

# Special LOD resolutions, from engine/Poseidon/Graphics/Rendering/Draw/SpecLods.hpp.
GEOMETRY_SPEC = 1e13
SPEC_LOD = 1e15
MEMORY_SPEC = SPEC_LOD * 1
LANDCONTACT_SPEC = SPEC_LOD * 2
ROADWAY_SPEC = SPEC_LOD * 3
PATHS_SPEC = SPEC_LOD * 4
HITPOINTS_SPEC = SPEC_LOD * 5
VIEW_GEOM_SPEC = SPEC_LOD * 6
FIRE_GEOM_SPEC = SPEC_LOD * 7
VIEW_CARGO_GEOM_SPEC = SPEC_LOD * 8
VIEW_COMMANDER_SPEC = SPEC_LOD * 10
VIEW_COMMANDER_GEOM_SPEC = SPEC_LOD * 11
VIEW_PILOT_GEOM_SPEC = SPEC_LOD * 13
VIEW_GUNNER_GEOM_SPEC = SPEC_LOD * 15
FIRE_GUNNER_GEOM_SPEC = SPEC_LOD * 16
VIEW_GUNNER = 1000.0
VIEW_PILOT = 1100.0
VIEW_CARGO = 1200.0

# Names accepted for special LODs (lower case, spaces and underscores removed).
SPECIAL_LOD_NAMES = {
    "geometry": GEOMETRY_SPEC,
    "memory": MEMORY_SPEC,
    "landcontact": LANDCONTACT_SPEC,
    "roadway": ROADWAY_SPEC,
    "paths": PATHS_SPEC,
    "hitpoints": HITPOINTS_SPEC,
    "viewgeometry": VIEW_GEOM_SPEC,
    "firegeometry": FIRE_GEOM_SPEC,
    "viewcargogeometry": VIEW_CARGO_GEOM_SPEC,
    "viewcommander": VIEW_COMMANDER_SPEC,
    "viewcommandergeometry": VIEW_COMMANDER_GEOM_SPEC,
    "viewpilotgeometry": VIEW_PILOT_GEOM_SPEC,
    "viewgunnergeometry": VIEW_GUNNER_GEOM_SPEC,
    "firegunnergeometry": FIRE_GUNNER_GEOM_SPEC,
    "viewgunner": VIEW_GUNNER,
    "viewpilot": VIEW_PILOT,
    "viewcargo": VIEW_CARGO,
}

# LODs the engine loads as geometry only (ResolGeometryOnly in ShapeShared.hpp).
GEOMETRY_ONLY_SPECS = (
    GEOMETRY_SPEC,
    VIEW_GEOM_SPEC,
    FIRE_GEOM_SPEC,
    VIEW_PILOT_GEOM_SPEC,
    VIEW_CARGO_GEOM_SPEC,
    VIEW_COMMANDER_GEOM_SPEC,
    VIEW_GUNNER_GEOM_SPEC,
    MEMORY_SPEC,
    LANDCONTACT_SPEC,
    ROADWAY_SPEC,
    PATHS_SPEC,
    HITPOINTS_SPEC,
)

# LODs whose ComponentNN selections become convex collision components
# (LODShape::InitConvexComponents is run on these levels).
COMPONENT_SPECS = (
    GEOMETRY_SPEC,
    VIEW_GEOM_SPEC,
    FIRE_GEOM_SPEC,
    VIEW_PILOT_GEOM_SPEC,
    VIEW_CARGO_GEOM_SPEC,
    VIEW_COMMANDER_GEOM_SPEC,
    VIEW_GUNNER_GEOM_SPEC,
)

# Point and face flag masks the engine accepts (Data3D.h); anything else is
# treated as the "Objektiv bug" and reset to 0 on load.
POINT_ALL_FLAGS = 0x1 | 0x2 | 0x4 | 0x8 | 0x100 | 0x200 | 0x10 | 0x40 | 0x80 | 0x20 | 0x1000 | 0x2000 | 0xFF0000 | 0xF000000
FACE_ALL_FLAGS = 0x1 | 0x2 | 0x4 | 0x20 | 0x80 | 0x100000 | 0x200000 | 0x8 | 0x10 | 0x1000000 | 0xFE000000 | 0x300

MLOD_VERSION = 0x0101  # 1.01: engine accepts major (ver >> 8) == 1
SP3X_VERSION = 0x0100
SP3X_HEAD_SIZE = 28
FACE_STRUCT_SIZE = 32 + 4 + 4 * 16 + 4  # 104 bytes, DataFaceEx
TEXTURE_FIELD = 32  # bytes including the terminating NUL
NAME_FIELD = 64  # TAGG names and #Property# name/value fields
MAX_UV = 1e5  # the engine zeroes UVs beyond this

TEXT_ENCODING = "cp1252"


class MlodError(ValueError):
    """Raised for data the engine reader would reject or misread."""


def is_spec(resolution, spec):
    """Engine IsSpec: relative match within 1e-3."""
    return abs(resolution - spec) < spec * 1e-3


def is_geometry_only(resolution):
    return any(is_spec(resolution, s) for s in GEOMETRY_ONLY_SPECS)


def wants_components(resolution):
    return any(is_spec(resolution, s) for s in COMPONENT_SPECS)


def lod_name(resolution):
    """Readable name for a resolution value."""
    for name, spec in SPECIAL_LOD_NAMES.items():
        if spec >= 1e13 and is_spec(resolution, spec):
            return name
    for name, spec in (("viewgunner", VIEW_GUNNER), ("viewpilot", VIEW_PILOT), ("viewcargo", VIEW_CARGO)):
        if abs(resolution - spec) < 0.5:
            return name
    return "%g" % resolution


def parse_resolution(text):
    """Parse a LOD resolution from a name such as '0.15', '1e13', 'Geometry' or 'LOD 1.0'.

    Returns a float, or None when the text does not name a LOD. Raises MlodError
    for ShadowVolume, which this engine has no LOD type for.
    """
    if text is None:
        return None
    if isinstance(text, (int, float)):
        return float(text)
    s = str(text).strip()
    key = s.lower().replace(" ", "").replace("_", "").replace("-", "")
    if key.startswith("shadowvolume"):
        raise MlodError(
            "'%s': this engine has no shadow-volume LOD; shadows come from the nearest resolution LOD "
            "without the lodnoshadow property" % s
        )
    if key in SPECIAL_LOD_NAMES:
        return SPECIAL_LOD_NAMES[key]
    if key.startswith("lod"):
        key = key[3:]
    if key.endswith("m"):
        key = key[:-1]
    try:
        value = float(key)
    except ValueError:
        return None
    if math.isnan(value) or math.isinf(value) or value < 0:
        return None
    return value


def file_order(corners):
    """Engine (front facing) corner order -> order written to the file.

    The engine reverses every face after reading it, so writing the reverse
    gives back the intended order in game.
    """
    return list(reversed(corners))


def encode_weight(weight):
    """Selection weight in [0, 1] -> O2 point byte.

    0 means "not selected", 1 means full weight. Partial weights are stored
    inverted: the engine keeps ``(byte)(-value)`` and divides by 255
    (ShapeSetup.cpp ``SelInfo(i, -val)``, RtAnimation.cpp ``Weight * 1/255``).
    """
    if weight is None or weight <= 0.0:
        return 0
    q = int(round(min(1.0, float(weight)) * 255.0))
    if q <= 0:
        return 0
    return (256 - q) & 0xFF


def decode_weight(byte):
    if byte == 0:
        return 0.0
    return ((256 - byte) & 0xFF) / 255.0


@dataclass
class MlodFace:
    """One polygon. ``corners`` are (point, normal, u, v) tuples in FILE order."""

    corners: list
    texture: str = ""
    flags: int = 0


@dataclass
class MlodSelection:
    name: str
    points: dict = field(default_factory=dict)  # point index -> weight in (0, 1]
    faces: set = field(default_factory=set)


@dataclass
class MlodLod:
    resolution: float
    points: list = field(default_factory=list)  # (x, y, z) game space
    point_flags: list = None  # int per point, or None for all 0
    normals: list = field(default_factory=list)  # (x, y, z)
    faces: list = field(default_factory=list)  # MlodFace
    selections: list = field(default_factory=list)  # MlodSelection
    properties: list = field(default_factory=list)  # (name, value)
    mass: list = None  # float per point (geometry LOD only), or None

    def selection(self, name):
        low = name.lower()
        for sel in self.selections:
            if sel.name.lower() == low:
                return sel
        return None

    def get_or_add_selection(self, name):
        sel = self.selection(name)
        if sel is None:
            sel = MlodSelection(name)
            self.selections.append(sel)
        return sel


def _fixed(text, size, what):
    raw = text.encode(TEXT_ENCODING) if isinstance(text, str) else bytes(text)
    if len(raw) >= size:
        raise MlodError("%s '%s' is %d bytes; the field holds at most %d plus the NUL" % (what, text, len(raw), size - 1))
    return raw + b"\0" * (size - len(raw))


def _check_finite(values, what):
    for v in values:
        if not math.isfinite(v):
            raise MlodError("%s has a non-finite value: %r" % (what, values))


def faces_fully_in(lod, point_set):
    """Indices of faces whose every corner point is in ``point_set``."""
    out = set()
    for fi, face in enumerate(lod.faces):
        if all(c[0] in point_set for c in face.corners):
            out.add(fi)
    return out


def auto_components(lod, prefix="Component"):
    """Add ComponentNN selections (one per connected face island) when the LOD has none.

    Returns the number of components added. Islands with fewer than 4 faces
    are skipped, as InitConvexComponents ignores them.
    """
    if any(sel.name.lower().startswith("component") for sel in lod.selections):
        return 0
    parent = list(range(len(lod.points)))

    def find(a):
        while parent[a] != a:
            parent[a] = parent[parent[a]]
            a = parent[a]
        return a

    for face in lod.faces:
        pts = [c[0] for c in face.corners]
        for p in pts[1:]:
            ra, rb = find(pts[0]), find(p)
            if ra != rb:
                parent[rb] = ra
    islands = {}
    for fi, face in enumerate(lod.faces):
        islands.setdefault(find(face.corners[0][0]), []).append(fi)
    added = 0
    for faces in islands.values():
        if len(faces) < 4:
            continue
        added += 1
        sel = MlodSelection("%s%02d" % (prefix, added))
        for fi in faces:
            for c in lod.faces[fi].corners:
                sel.points[c[0]] = 1.0
            sel.faces.add(fi)
        lod.selections.append(sel)
    return added


def validate_lod(lod):
    """Raise MlodError for anything the engine would reject, misread or crash on."""
    npos, nnorm, nface = len(lod.points), len(lod.normals), len(lod.faces)
    where = "LOD %s" % lod_name(lod.resolution)
    if lod.point_flags is not None and len(lod.point_flags) != npos:
        raise MlodError("%s: point_flags has %d entries for %d points" % (where, len(lod.point_flags), npos))
    for p in lod.points:
        _check_finite(p, where + " point")
    for n in lod.normals:
        _check_finite(n, where + " normal")
    if lod.point_flags:
        for i, fl in enumerate(lod.point_flags):
            if (fl & 0xFFFFFFFF) & ~POINT_ALL_FLAGS:
                raise MlodError("%s: point %d has flags 0x%x the engine rejects" % (where, i, fl))
    for fi, face in enumerate(lod.faces):
        if len(face.corners) not in (3, 4):
            raise MlodError("%s: face %d has %d corners; only triangles and quads are stored" % (where, fi, len(face.corners)))
        _fixed(face.texture or "", TEXTURE_FIELD, "%s face %d texture" % (where, fi))
        if (face.flags & 0xFFFFFFFF) & ~FACE_ALL_FLAGS:
            raise MlodError("%s: face %d has flags 0x%x the engine rejects" % (where, fi, face.flags & 0xFFFFFFFF))
        for p, n, u, v in face.corners:
            if not (0 <= p < npos):
                raise MlodError("%s: face %d uses point %d of %d" % (where, fi, p, npos))
            if not (0 <= n < nnorm):
                raise MlodError("%s: face %d uses normal %d of %d" % (where, fi, n, nnorm))
            if not (math.isfinite(u) and math.isfinite(v)) or abs(u) > MAX_UV or abs(v) > MAX_UV:
                raise MlodError("%s: face %d has UV (%r, %r) out of range" % (where, fi, u, v))
    seen = set()
    for sel in lod.selections:
        if not sel.name or sel.name.startswith("#"):
            raise MlodError("%s: selection name '%s' is empty or starts with '#'" % (where, sel.name))
        _fixed(sel.name, NAME_FIELD, "%s selection name" % where)
        key = sel.name.lower()
        if key in seen:
            raise MlodError("%s: selection '%s' appears twice (names are case-insensitive)" % (where, sel.name))
        seen.add(key)
        for p in sel.points:
            if not (0 <= p < npos):
                raise MlodError("%s: selection '%s' uses point %d of %d" % (where, sel.name, p, npos))
        for f in sel.faces:
            if not (0 <= f < nface):
                raise MlodError("%s: selection '%s' uses face %d of %d" % (where, sel.name, f, nface))
    for name, value in lod.properties:
        _fixed(name, NAME_FIELD, "%s property name" % where)
        _fixed(value, NAME_FIELD, "%s property value" % where)
    if lod.mass is not None:
        if not is_spec(lod.resolution, GEOMETRY_SPEC):
            raise MlodError("%s: #Mass# is only read from the Geometry LOD" % where)
        if len(lod.mass) != npos:
            raise MlodError("%s: #Mass# has %d values for %d points" % (where, len(lod.mass), npos))
        _check_finite(lod.mass, where + " mass")


def _tag(name, payload):
    return _fixed(name, NAME_FIELD, "tag name") + struct.pack("<i", len(payload)) + payload


def write_lod(lod):
    validate_lod(lod)
    npos, nface = len(lod.points), len(lod.faces)
    out = bytearray(b"SP3X")
    out += struct.pack("<iiiiii", SP3X_HEAD_SIZE, SP3X_VERSION, npos, len(lod.normals), nface, 0)
    flags = lod.point_flags or [0] * npos
    for (x, y, z), fl in zip(lod.points, flags):
        out += struct.pack("<fffI", x, y, z, fl & 0xFFFFFFFF)
    for x, y, z in lod.normals:
        out += struct.pack("<fff", x, y, z)
    for face in lod.faces:
        out += _fixed(face.texture or "", TEXTURE_FIELD, "texture")
        out += struct.pack("<i", len(face.corners))
        for i in range(4):
            if i < len(face.corners):
                p, n, u, v = face.corners[i]
            else:
                p, n, u, v = 0, 0, 0.0, 0.0
            out += struct.pack("<iiff", p, n, u, v)
        out += struct.pack("<I", face.flags & 0xFFFFFFFF)
    out += b"TAGG"
    for sel in lod.selections:
        payload = bytearray(npos + nface)
        for p, w in sel.points.items():
            payload[p] = encode_weight(w)
        for f in sel.faces:
            payload[npos + f] = 1
        out += _tag(sel.name, bytes(payload))
    for name, value in lod.properties:
        out += _tag("#Property#", _fixed(name, NAME_FIELD, "property name") + _fixed(value, NAME_FIELD, "property value"))
    if lod.mass is not None:
        out += _tag("#Mass#", struct.pack("<%df" % npos, *lod.mass))
    out += _tag("#EndOfFile#", b"")
    out += struct.pack("<f", lod.resolution)
    return bytes(out)


def write_mlod(lods, sort=True):
    """Serialize LODs to MLOD bytes. LODs are sorted by resolution (the engine
    expects ascending order) unless ``sort`` is False."""
    if not lods:
        raise MlodError("no LODs to write")
    if len(lods) > 100:
        raise MlodError("%d LODs; the reader accepts at most 100" % len(lods))
    ordered = sorted(lods, key=lambda l: l.resolution) if sort else list(lods)
    seen = []
    for lod in ordered:
        f32 = struct.unpack("<f", struct.pack("<f", lod.resolution))[0]
        if f32 in seen:
            raise MlodError("two LODs share resolution %s" % lod_name(lod.resolution))
        seen.append(f32)
    if sum(1 for l in ordered if l.mass is not None) > 1:
        raise MlodError("#Mass# may appear on one LOD only (the Geometry LOD)")
    out = bytearray(b"MLOD")
    out += struct.pack("<ii", MLOD_VERSION, len(ordered))
    for lod in ordered:
        out += write_lod(lod)
    return bytes(out)


def write_mlod_file(path, lods, sort=True):
    data = write_mlod(lods, sort=sort)
    with open(path, "wb") as f:
        f.write(data)
    return len(data)


# ---------------------------------------------------------------------------
# Strict reader, mirroring Shape::LoadTagged / LODShape::Load. It is used by the
# unit tests and by the exporter's self-check; it raises MlodError where the
# engine would fail, assert, read out of bounds or silently drop data.
# ---------------------------------------------------------------------------


class _Cursor:
    def __init__(self, data):
        self.data = data
        self.pos = 0

    def take(self, n):
        if self.pos + n > len(self.data):
            raise MlodError("unexpected end of file at byte %d (wanted %d more)" % (self.pos, n))
        chunk = self.data[self.pos : self.pos + n]
        self.pos += n
        return chunk

    def unpack(self, fmt):
        size = struct.calcsize(fmt)
        return struct.unpack(fmt, self.take(size))


def _cstr(raw, what):
    nul = raw.find(b"\0")
    if nul < 0:
        raise MlodError("%s is not NUL terminated inside its %d byte field" % (what, len(raw)))
    return raw[:nul].decode(TEXT_ENCODING)


def read_mlod(data, strict=True):
    """Parse MLOD bytes. Returns (version, [MlodLod], warnings)."""
    cur = _Cursor(bytes(data))
    warnings = []
    if cur.take(4) != b"MLOD":
        raise MlodError("not an MLOD file")
    (ver, nlods) = cur.unpack("<ii")
    if (ver >> 8) != 1:
        raise MlodError("version 0x%x: the engine accepts only 1.xx" % ver)
    if (ver & 0xFF) == 0 or (ver & 0xFF) > 10:
        # The standalone MLODLoader reads the low byte as the major version.
        raise MlodError("version 0x%x: PoseidonTools' MLOD loader needs its low byte in 1..10" % ver)
    if not (1 <= nlods <= 100):
        raise MlodError("LOD count %d out of range" % nlods)
    lods = []
    for li in range(nlods):
        lods.append(_read_lod(cur, li, warnings, strict))
    if cur.pos != len(cur.data):
        msg = "%d trailing bytes after the last LOD" % (len(cur.data) - cur.pos)
        if strict:
            raise MlodError(msg)
        warnings.append(msg)
    prev = -1.0
    for lod in lods:
        if lod.resolution < prev:
            warnings.append("LODs are not in ascending resolution order (%s after %s)" % (lod_name(lod.resolution), lod_name(prev)))
        prev = lod.resolution
    masses = [l for l in lods if l.mass is not None]
    if len(masses) > 1:
        raise MlodError("#Mass# found on %d LODs; the engine asserts on more than one" % len(masses))
    return ver, lods, warnings


def _read_lod(cur, li, warnings, strict):
    where = "LOD #%d" % li
    magic = cur.take(4)
    if magic != b"SP3X":
        raise MlodError("%s: magic %r, expected SP3X" % (where, magic))
    head_size, _version, npos, nnorm, nface, _flags = cur.unpack("<iiiiii")
    if head_size < SP3X_HEAD_SIZE:
        raise MlodError("%s: header size %d < 28" % (where, head_size))
    cur.take(head_size - SP3X_HEAD_SIZE)
    if npos < 0 or nnorm < 0 or nface < 0:
        raise MlodError("%s: negative counts" % where)
    lod = MlodLod(resolution=0.0)
    lod.point_flags = []
    for i in range(npos):
        x, y, z, fl = cur.unpack("<fffI")
        if fl & ~POINT_ALL_FLAGS:
            raise MlodError("%s: point %d flags 0x%x would be reset by the engine" % (where, i, fl & 0xFFFFFFFF))
        lod.points.append((x, y, z))
        lod.point_flags.append(fl)
    for i in range(nnorm):
        n = cur.unpack("<fff")
        if not all(math.isfinite(c) for c in n):
            raise MlodError("%s: normal %d is not finite" % (where, i))
        lod.normals.append(n)
    for fi in range(nface):
        tex = _cstr(cur.take(TEXTURE_FIELD), "%s face %d texture" % (where, fi))
        (n,) = cur.unpack("<i")
        corners = [cur.unpack("<iiff") for _ in range(4)]
        (fl,) = cur.unpack("<I")
        if n not in (3, 4):
            raise MlodError("%s: face %d has n=%d" % (where, fi, n))
        if fl & ~FACE_ALL_FLAGS:
            raise MlodError("%s: face %d flags 0x%x would be reset by the engine" % (where, fi, fl & 0xFFFFFFFF))
        corners = corners[:n]
        for p, nrm, u, v in corners:
            if not (0 <= p < npos):
                raise MlodError("%s: face %d point index %d out of range" % (where, fi, p))
            if not (0 <= nrm < nnorm):
                raise MlodError("%s: face %d normal index %d out of range" % (where, fi, nrm))
            if abs(u) > MAX_UV or abs(v) > MAX_UV:
                raise MlodError("%s: face %d UV out of range" % (where, fi))
        lod.faces.append(MlodFace(corners=[tuple(c) for c in corners], texture=tex, flags=fl))
    if cur.take(4) != b"TAGG":
        raise MlodError("%s: missing TAGG" % where)
    while True:
        name = _cstr(cur.take(NAME_FIELD), "%s tag name" % where)
        (size,) = cur.unpack("<i")
        if size < 0:
            raise MlodError("%s: tag '%s' has negative size" % (where, name))
        if name.lower() == "#endoffile#":
            if size != 0:
                cur.take(size)
            break
        payload = cur.take(size)
        low = name.lower()
        if low == "#mass#":
            if size != 4 * npos:
                raise MlodError("%s: #Mass# size %d, expected %d" % (where, size, 4 * npos))
            lod.mass = list(struct.unpack("<%df" % npos, payload))
        elif low == "#property#":
            if size != 128:
                raise MlodError("%s: #Property# size %d, expected 128" % (where, size))
            lod.properties.append((_cstr(payload[:64], "property name"), _cstr(payload[64:], "property value")))
        elif name.startswith("#"):
            warnings.append("%s: tag %s skipped (the engine ignores it)" % (where, name))
        else:
            if size != npos + nface:
                msg = "%s: selection '%s' size %d, expected %d (engine drops it)" % (where, name, size, npos + nface)
                if strict:
                    raise MlodError(msg)
                warnings.append(msg)
                continue
            if name[:1] in ("-", "."):
                warnings.append("%s: selection '%s' is ignored by the engine" % (where, name))
                continue
            sel = MlodSelection(name)
            for p in range(npos):
                if payload[p]:
                    sel.points[p] = decode_weight(payload[p])
            for f in range(nface):
                if payload[npos + f]:
                    sel.faces.add(f)
            if name.lower().startswith("proxy:") and (len(sel.points) != 3 or len(sel.faces) != 1):
                warnings.append(
                    "%s: proxy '%s' has %d points and %d faces; the engine wants 3 and 1"
                    % (where, name, len(sel.points), len(sel.faces))
                )
            lod.selections.append(sel)
    (lod.resolution,) = cur.unpack("<f")
    return lod
