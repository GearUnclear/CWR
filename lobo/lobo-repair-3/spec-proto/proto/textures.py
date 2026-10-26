"""SPEC PROTOTYPE: fixed-width MLOD face texture repoints (d1, e1, e2).

MLOD 257 / SP3X 153 faces are 104 bytes; bytes 0..31 are the texture field,
read up to the first NUL (UD copies 32 bytes into a 33-byte buffer; Classic
relies on a NUL being present). A new name is at most 31 characters followed
by NUL fill to 32 bytes. Only listed LODs (by resolution) are touched, with
exact per-LOD counts; the already-repaired state is a no-op.
"""
import math
import struct

from repair.models import _lods as _strict_lods

BS = chr(92)


def _lods(data):
    """Prototype stand-in for the relaxed models._lods the spec asks for: accept
    any 32-byte Oxygen default-texture-path trailer (printable path + NUL),
    e.g. b'H:\\Codemast\\0...' in LoBo_MD500D_R.p3d, not only b'data\\\\\\0'."""
    tail = data[-32:]
    head = tail.split(b"\0", 1)[0]
    if b"\0" in tail and head and all(32 <= c <= 126 for c in head) and not tail.startswith(b"data\\\0"):
        return _strict_lods(data[:-32] + b"data\\\0" + bytes(26))
    return _strict_lods(data)


def _face_table(data, lod):
    header, points, normals, faces = struct.unpack_from('<4x i 4x i i i', data, lod.start)
    start = lod.start + header + points * 16 + normals * 12
    return start, faces


def field_name(raw, *, raw32=False):
    return raw if raw32 else raw.split(b"\0", 1)[0]


def textures(data):
    """[(resolution, Counter(name bytes -> count))] for inspection/tests."""
    from collections import Counter
    out = []
    for lod in _lods(data):
        start, faces = _face_table(data, lod)
        c = Counter(data[start + i * 104:start + i * 104 + 32].split(b"\0", 1)[0] for i in range(faces))
        out.append((lod.resolution, c))
    return out


def repoint(data, resolutions, mapping):
    """mapping: {old name bytes: (new name bytes, expected count per listed LOD, raw32)}.

    raw32=True matches all 32 raw bytes (a name that overflowed its field and
    has no NUL); otherwise the NUL-terminated name, case-insensitively.
    """
    fixed = bytearray(data)
    hits = 0
    for lod in _lods(data):
        listed = any(math.isclose(lod.resolution, r, rel_tol=1e-6) for r in resolutions)
        start, faces = _face_table(data, lod)
        counts = {old: 0 for old in mapping}
        repaired = {old: 0 for old in mapping}
        for i in range(faces):
            at = start + i * 104
            raw = bytes(fixed[at:at + 32])
            name = raw.split(b"\0", 1)[0]
            for old, (new, _, raw32) in mapping.items():
                if (raw == old if raw32 else name.lower() == old.lower()):
                    if not listed:
                        raise ValueError(f"Unexpected {old!r} outside the listed LODs ({lod.resolution})")
                    if len(new) > 31:
                        raise ValueError("Texture name longer than 31 characters")
                    fixed[at:at + 32] = new + bytes(32 - len(new))
                    counts[old] += 1
                elif listed and raw == new + bytes(32 - len(new)):
                    repaired[old] += 1
        if listed:
            for old, (new, expected, _) in mapping.items():
                if counts[old] + repaired[old] != expected:
                    raise ValueError(f"LOD {lod.resolution}: expected {expected} faces {old!r}; "
                                     f"found {counts[old]} original + {repaired[old]} repaired")
                hits += counts[old]
    return bytes(fixed)


def _b(*parts):
    return BS.join(parts).encode("ascii")


MD500 = "loboidfhelo/lobo_md500d_r.p3d"
CREW_JOR = "lobojordan/lobo_crew_jor.p3d"
CREW_SYR = "lobosyria/lobo_crew_syria.p3d"
MODEL_MEMBERS = (MD500, CREW_JOR, CREW_SYR)
PLANS = {
    # d1: collision/fire/hitpoint LODs still named the BAS Little Birds source textures.
    MD500: ((1e13, 5e15, 7e15), {
        _b("bas_lbs", "m", "uh1ffarpod.pac"): (_b("LoboIDFhelo", "MD", "uh1ffarpod.pac"), 10, False),
        _b("bas_lbs", "m", "uh1podend.pac"): (_b("LoboIDFhelo", "MD", "uh1podend.pac"), 8, False),
    }),
    # e1: LOD 5.0 body armor faces name textures that never shipped; LOD 0/1 use ba_*.
    CREW_JOR: ((5.0,), {
        _b("LoBoJordan", "wl", "rba_frt.paa"): (_b("LoBoJordan", "wl", "ba_frt.paa"), 26, False),
        _b("LoBoJordan", "wl", "rba_bk.paa"): (_b("LoBoJordan", "wl", "ba_bk.paa"), 26, False),
    }),
    # e2: LOD 5.0 names overflowed the 32-byte field (no NUL, folder syr_crew absent).
    CREW_SYR: ((5.0,), {
        _b("LoBoSyria", "syr_crew", "e_tank_hrud_p"): (_b("LoBoSyria", "crew", "hrud_p.pac"), 23, True),
        _b("LoBoSyria", "syr_crew", "e_tank_nohy_p"): (_b("LoBoSyria", "crew", "nohy_p.pac"), 15, True),
        _b("LoBoSyria", "syr_crew", "e_tank_nohy_z"): (_b("LoBoSyria", "crew", "nohy_z.pac"), 6, True),
        _b("LoBoSyria", "syr_crew", "e_tank_zada_z"): (_b("LoBoSyria", "crew", "zada_z.pac"), 18, True),
        _b("LoBoSyria", "syr_crew", "00000mc_tanki"): (_b("data", "00000mc_tankistae2.paa"), 38, True),
        _b("LoBoSyria", "syr_crew", "00002mc_tanki"): (_b("data", "00002mc_tankistae2.paa"), 24, True),
    }),
}
ADDONS = tuple(sorted({key.split("/")[0] for key in MODEL_MEMBERS}))


def repair(documents):
    return {}


def repair_models(documents):
    changed = {}
    for key, (resolutions, mapping) in PLANS.items():
        if key not in documents:
            raise ValueError("Missing model " + key)
        fixed = repoint(documents[key], resolutions, mapping)
        if fixed != documents[key]:
            changed[key] = fixed
    return changed
