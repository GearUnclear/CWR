"""Fixed-width MLOD face texture repoints (textures-icons d1, e1, e2).

MLOD 257 / SP3X 153 faces are 104 bytes; bytes 0..31 are the texture field,
read up to the first NUL (UD copies 32 bytes into a 33-byte buffer; Classic
relies on a NUL being present). A new name is at most 31 characters followed
by NUL fill to 32 bytes. Only the listed LODs (by resolution) may contain a
match, each listed LOD must hold the exact expected count (original plus
already repaired faces), and file length and every other byte stay unchanged.
"""
import math
import struct

from .models import _lods

BS = chr(92)
FACE = 104
FIELD = 32


def _face_table(data: bytes, lod) -> tuple[int, int]:
    header, points, normals, faces = struct.unpack_from('<4x i 4x i i i', data, lod.start)
    return lod.start + header + points * 16 + normals * 12, faces


def face_textures(data: bytes) -> list[tuple[float, list[bytes]]]:
    """[(resolution, [raw 32-byte texture field per face])] for inspection and tests."""
    out = []
    for lod in _lods(data):
        start, faces = _face_table(data, lod)
        out.append((lod.resolution, [data[start + i * FACE:start + i * FACE + FIELD] for i in range(faces)]))
    return out


def repoint(data: bytes, resolutions: tuple, mapping: dict) -> bytes:
    """mapping: {old name bytes: (new name bytes, expected count per listed LOD, raw32)}.

    raw32=True matches all 32 raw bytes (a name that overflowed its field and
    has no NUL); otherwise the NUL-terminated name, case-insensitively. A field
    whose first byte is NUL never matches a raw32 pattern.
    """
    for old, (new, _, raw32) in mapping.items():
        if not new or len(new) > FIELD - 1 or b'\0' in new:
            raise ValueError(f"Texture name must be 1..{FIELD - 1} characters: {new!r}")
        if raw32 and (len(old) != FIELD or b'\0' in old):
            raise ValueError(f"Overflowed texture field must be {FIELD} bytes without NUL: {old!r}")
    fixed = bytearray(data)
    for lod in _lods(data):
        listed = any(math.isclose(lod.resolution, r, rel_tol=1e-6) for r in resolutions)
        start, faces = _face_table(data, lod)
        counts = {old: 0 for old in mapping}
        repaired = {old: 0 for old in mapping}
        for i in range(faces):
            at = start + i * FACE
            raw = bytes(fixed[at:at + FIELD])
            name = raw.split(b'\0', 1)[0]
            for old, (new, _, raw32) in mapping.items():
                if (raw == old) if raw32 else (name.lower() == old.lower()):
                    if not listed:
                        raise ValueError(f"Unexpected {old!r} outside the listed LODs ({lod.resolution})")
                    fixed[at:at + FIELD] = new + bytes(FIELD - len(new))
                    counts[old] += 1
                elif listed and raw == new + bytes(FIELD - len(new)):
                    repaired[old] += 1
        if listed:
            for old, (new, expected, _) in mapping.items():
                if counts[old] + repaired[old] != expected:
                    raise ValueError(f"LOD {lod.resolution}: expected {expected} faces {old!r}; "
                                     f"found {counts[old]} original + {repaired[old]} repaired")
    if len(fixed) != len(data):
        raise ValueError("Texture repoint changed the model length")
    return bytes(fixed)


def _b(*parts: str) -> bytes:
    return BS.join(parts).encode("ascii")


MD500 = "loboidfhelo/lobo_md500d_r.p3d"
CREW_JOR = "lobojordan/lobo_crew_jor.p3d"
CREW_SYR = "lobosyria/lobo_crew_syria.p3d"
MODEL_MEMBERS = (MD500, CREW_JOR, CREW_SYR)
PLANS = {
    # d1: collision, hitpoint and fire LODs still named the BAS Little Birds
    # source textures; the visual LODs already use the LoboIDFhelo copies.
    MD500: ((1e13, 5e15, 7e15), {
        _b("bas_lbs", "m", "uh1ffarpod.pac"): (_b("LoboIDFhelo", "MD", "uh1ffarpod.pac"), 10, False),
        _b("bas_lbs", "m", "uh1podend.pac"): (_b("LoboIDFhelo", "MD", "uh1podend.pac"), 8, False),
    }),
    # e1: LOD 5.0 body armor faces name textures that never shipped; LOD 0/1
    # use ba_* on the same faces.
    CREW_JOR: ((5.0,), {
        _b("LoBoJordan", "wl", "rba_frt.paa"): (_b("LoBoJordan", "wl", "ba_frt.paa"), 26, False),
        _b("LoBoJordan", "wl", "rba_bk.paa"): (_b("LoBoJordan", "wl", "ba_bk.paa"), 26, False),
    }),
    # e2: LOD 5.0 names overflowed the 32-byte field (no NUL; folder syr_crew
    # does not exist). Targets are the recolours LOD 0..2 use and the base
    # atlases the truncated names match.
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


def repair(documents: dict[str, str]) -> dict[str, str]:
    # plan() calls repair() on every module; this one has no text rules.
    return {}


def repair_models(documents: dict[str, bytes]) -> dict[str, bytes]:
    changed = {}
    for key, (resolutions, mapping) in PLANS.items():
        if key not in documents:
            raise ValueError("Missing model " + key)
        try:
            fixed = repoint(documents[key], resolutions, mapping)
        except ValueError as exc:
            raise ValueError(f"{key}: {exc}") from exc
        if fixed != documents[key]:
            changed[key] = fixed
    return changed
