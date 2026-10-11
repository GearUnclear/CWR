"""Verified component/model repairs; no new damage counters or engine protocol.

Only Hitpoints memberships change: existing hull coverage is expanded, missing
legacy-name aliases are appended. Geometry, animation and collision stay intact.
"""
from dataclasses import dataclass
import math
import re
import struct

from .config import class_block, class_spans, scalar, set_scalar, tokens

ADDONS = ("coc_diver", "lobo_m1", "lobomerkava2", "lobomerkava4")
_MARKER = "// UD supported component armor; see tools/lobo/ARMOR-MODELS.md."
_SUPPORTED = {"hitengine", "hithull", "hitturret", "hitgun", "hitltrack", "hitrtrack"}
_M1 = ("lobo_m1a1e", "lobo_m1a1e_r", "lobo_m1a2e", "lobo_m1a2e_r")
MODEL_MEMBERS = tuple(f"lobo_m1/{name}.p3d" for name in _M1) + (
    "lobomerkava2/lobomerkava2.p3d", "lobomerkava2/lobomerkava3.p3d",
)
_M1_UNIONS = {
    "hull": ("Hull", "FHull", "BHull", "LHull", "RHull"),
    "turet": ("Turret", "FTurret", "BTurret", "LTurret", "RTurret", "TTurret"),
    "pasL": ("LTrack",),
    "pasP": ("RTrack",),
}
_MERKAVA_UNIONS = {
    "hull": ("hull", "fhull"),
    "turet": ("turetf", "turett", "turetb"),
}


def _value(value):
    return value.strip().strip('"').casefold() if value is not None else None


def _checked(text, path, key, old, new):
    value = _value(scalar(text, path, key))
    if value not in {_value(str(old)), _value(str(new))}:
        raise ValueError(f"Unexpected {path}/{key}: {value!r}")
    return set_scalar(text, path, key, new)


def _remove_class(text, path, name, expected, repaired):
    a, b = class_block(text, path)
    matches = [(n, x, y) for n, x, y in class_spans(text[a:b]) if n.casefold() == name.casefold()]
    if not matches:
        if repaired:
            return text
        raise ValueError(f"Missing original class {path}/{name}")
    if len(matches) != 1 or repaired:
        raise ValueError(f"Mixed or duplicate armor repair state: {path}/{name}")
    _, x, y = matches[0]
    child = path + (name,)
    for key, value in expected.items():
        if _value(scalar(text, child, key)) != _value(str(value)):
            raise ValueError(f"Unexpected directional armor {child}/{key}")
    ts = tokens(text[a+x:a+y])
    keys = {ts[i].text.casefold() for i in range(len(ts)-1) if ts[i+1].text == '='}
    if keys != {key.casefold() for key in expected}:
        raise ValueError(f"Unknown properties in {child}")
    # Match only the already parsed, direct class declaration.
    head = text[a:a+x]
    starts = list(re.finditer(r'\bclass\s+' + re.escape(name) + r'\s*\{\s*$', head, re.I))
    if len(starts) != 1:
        raise ValueError(f"Unexpected class declaration {child}")
    start = a + starts[0].start()
    end = a + y + 1
    if text[end:end+1] != ';':
        raise ValueError(f"Missing class terminator {child}")
    return text[:start] + text[end+1:]


def _remove_property(text, path, name, expected, repaired):
    a, b = class_block(text, path)
    actual = scalar(text, path, name)
    if actual is None:
        if repaired:
            return text
        raise ValueError(f"Missing original armor property {path}/{name}")
    if repaired or _value(actual) != _value(str(expected)):
        raise ValueError(f"Unexpected armor property {path}/{name}")
    body = text[a:b]
    pattern = r'(?m)^[ \t]*' + re.escape(name) + r'\s*=\s*' + re.escape(actual) + r'\s*;[ \t]*(?:\r?\n)?'
    body, count = re.subn(pattern, '', body, flags=re.I)
    if count != 1:
        raise ValueError(f"Unexpected armor assignment {path}/{name}")
    return text[:a] + body + text[b:]


def _component(text, path, name, armor, material, selection):
    a, b = class_block(text, path)
    present = [n for n, _, _ in class_spans(text[a:b]) if n.casefold() == name.casefold()]
    if present:
        child = path + (name,)
        expected = {"armor": str(armor), "material": str(material), "name": selection, "passThrough": "1"}
        if any(_value(scalar(text, child, k)) != _value(v) for k, v in expected.items()):
            raise ValueError(f"Unexpected supported component {child}")
        return text
    nl = '\r\n' if '\r\n' in text else '\n'
    line = f'{nl}\t\tclass {name} {{armor={armor};material={material};name={selection};passThrough=1;}};{nl}\t'
    return text[:b] + line + text[b:]


def _armor(text, vehicle, directional, fields, *, hull, turret, add_turret=False):
    path = ("CfgVehicles", vehicle)
    a, b = class_block(text, path)
    repaired = _MARKER in text[a:b]
    for name, armor, material, selection in directional:
        expected = {"armor": armor, "material": material, "name": selection, "passThrough": 1}
        text = _remove_class(text, path, name, expected, repaired)
    for name, value in fields.items():
        text = _remove_property(text, path, name, value, repaired)
    if hull:
        text = _checked(text, path + ("HitHull",), "name", "hull", hull)
    if add_turret:
        text = _component(text, path, "HitTurret", 1, 51, turret)
    elif turret:
        text = _checked(text, path + ("HitTurret",), "name", "turet", turret)
    a, b = class_block(text, path)
    unsupported = [n for n, _, _ in class_spans(text[a:b]) if n.lower().startswith('hit') and n.lower() not in _SUPPORTED]
    if unsupported:
        raise ValueError(f"Unrecognized hitpoint definitions in {path}: {unsupported}")
    if not repaired:
        nl = '\r\n' if '\r\n' in text else '\n'
        text = text[:a] + nl + '\t\t' + _MARKER + text[a:]
    return text


def repair(documents: dict[str, str]) -> dict[str, str]:
    changed = {}
    key = "coc_diver/config.cpp"
    if key in documents:
        text = documents[key]
        for name, old in (("CoC_Diverdes", r'"\seals\KMMwood.p3d"'),
                          ("CoC_DiverSaboteurj", r'"\seals\KMMwood1.p3d"')):
            text = _checked(text, ("CfgVehicles", name), "model", old, r'"\CoC_Diver\CoC_Diver.p3d"')
        if text != documents[key]:
            changed[key] = text
    key = "lobo_m1/config.cpp"
    if key in documents:
        directional = [("Hit" + side + "Hull", armor, 50, side + "hull")
                       for side, armor in (("F", "1.6"), ("B", "0.6"), ("R", "1.25"), ("L", "1.25"))]
        directional += [("Hit" + side + "Turret", armor, 51, side + "turet")
                        for side, armor in (("F", "1.5"), ("B", "0.7"), ("L", "1.25"), ("R", "1.25"), ("T", "0.5"))]
        fields = {"armor" + name[3:]: armor for name, armor, _, _ in directional}
        text = _armor(documents[key], "LoBo_M1", directional, fields,
                      hull="hull", turret="turet")
        for component, selection in (("HitLTrack", "pasL"), ("HitRTrack", "pasP")):
            text = _checked(text, ("CfgVehicles", "LoBo_M1", component), "name", selection, selection)
        if text != documents[key]:
            changed[key] = text
    targets = {
        "lobomerkava2/config.cpp": (("LoBoMerkava1", "1.0", ("1.0", "0.7", "0.75", "1.0"), "hull", "turet"),
                                   ("LoBoMerkava2", "1.25", ("1.5", "0.7", "0.75", "1.5"), "hull", "turet"),
                                   ("LoBoMerkava2C", None, ("1.5", "2", "0.75", "1.5"), None, None)),
        "lobomerkava4/config.cpp": (("LoBoMerkava4DT", "1.25", ("1.5", "1.5", "1.5", "1.5"), "hull", "turet"),),
    }
    for key, vehicles in targets.items():
        if key not in documents:
            continue
        text = documents[key]
        for vehicle, front_hull, values, hull, turret in vehicles:
            directional = [] if front_hull is None else [
                ("HitTurretFront", "1.25", 51, "turetf"), ("HitFHull", front_hull, 50, "fhull"),
                ("HitTurretTop", "1", 51, "turett"), ("HitTurretBack", "0.80", 51, "turetb"),
            ]
            fields = dict(zip(("armorFHull", "armorTurretTop", "armorTurretBack", "armorTurretFront"), values))
            text = _armor(text, vehicle, directional, fields, hull=hull, turret=turret, add_turret=turret is not None)
        if text != documents[key]:
            changed[key] = text
    return changed


@dataclass(frozen=True)
class _Lod:
    start: int
    end: int
    point_count: int
    face_count: int
    resolution: float
    end_tag: int
    selections: dict[str, bytes | None]
    payload_offsets: dict[str, int]


def _lods(data: bytes) -> list[_Lod]:
    """Bounded reader for the known MLOD 257 / SP3X v153 fixed-width format.

    Layout matches MLODStructures.hpp. Writes replace verified selection payloads
    or insert before the Hitpoints end tag, never binary substring guesses.
    """
    def need(offset, length):
        if offset < 0 or length < 0 or offset + length > len(data):
            raise ValueError("Truncated or invalid MLOD")

    need(0, 12)
    signature, version, count = struct.unpack_from('<4sII', data)
    if signature != b'MLOD' or version != 257 or not 1 <= count <= 100:
        raise ValueError("Expected supported MLOD 257; no ODOL rewriting")
    pos = 12
    lods = []
    for _ in range(count):
        start = pos
        need(pos, 28)
        signature, header, version, points, normals, faces, flags = struct.unpack_from('<4s6i', data, pos)
        if signature != b'SP3X' or header < 28 or version != 153 or min(points, normals, faces) < 0:
            raise ValueError("Unsupported MLOD SP3X layout")
        amount = header + points * 16 + normals * 12 + faces * 104
        need(pos, amount + 4)
        pos += amount
        if data[pos:pos+4] != b'TAGG':
            raise ValueError("Expected MLOD TAGG section")
        pos += 4
        selections = {}
        payload_offsets = {}
        while True:
            need(pos, 68)
            tag_start = pos
            raw, size = struct.unpack_from('<64si', data, pos)
            name = raw.split(b'\0', 1)[0].decode('ascii')
            pos += 68
            need(pos, size)
            if name == '#EndOfFile#':
                if size != 0:
                    raise ValueError("Invalid MLOD end tag")
                break
            if name and name[0] not in '#-.':
                if size != points + faces:
                    raise ValueError(f"Invalid named selection {name}")
                folded = name.casefold()
                if folded in selections:
                    # Some unrelated source animation tags differ only in case.
                    # Never choose between duplicates in a repair's own inputs.
                    selections[folded] = None
                else:
                    selections[folded] = data[pos:pos+size]
                    payload_offsets[folded] = pos
            pos += size
        need(pos, 4)
        resolution = struct.unpack_from('<f', data, pos)[0]
        if not math.isfinite(resolution):
            raise ValueError("Invalid MLOD resolution")
        pos += 4
        lods.append(_Lod(start, pos, points, faces, resolution, tag_start, selections, payload_offsets))
    # Oxygen's legacy default-texture-path trailer is present in the installed
    # models. The engine reader ignores it; preserve all 32 bytes verbatim.
    if pos != len(data) and not (len(data) - pos == 32 and data[pos:pos+6] == b'data\\\0'):
        raise ValueError("Unexpected trailing MLOD data")
    return lods


def add_hitpoint_unions(data: bytes, unions: dict[str, tuple[str, ...]]) -> bytes:
    lods = [lod for lod in _lods(data) if math.isclose(lod.resolution, 5e15, rel_tol=1e-6)]
    if len(lods) != 1:
        raise ValueError("Expected exactly one Hitpoints LOD")
    lod = lods[0]
    additions = []
    fixed = bytearray(data)
    for target, sources in unions.items():
        arrays = []
        for source in sources:
            payload = lod.selections.get(source.casefold())
            if payload is None or not any(payload[:lod.point_count]):
                raise ValueError(f"Missing, duplicate, or empty Hitpoints selection {source}")
            arrays.append(payload)
        union = bytes(max(values) for values in zip(*arrays))
        if target.casefold() in lod.selections:
            if target.casefold() in {s.casefold() for s in sources}:
                # Expand an existing component's membership without renaming it:
                # setHit / Dammaged expose this model selection name to scripts.
                offset = lod.payload_offsets[target.casefold()]
                fixed[offset:offset+len(union)] = union
            elif lod.selections[target.casefold()] != union:
                raise ValueError(f"Existing {target} is not the expected Hitpoints union")
            continue
        name = target.encode('ascii')
        if not name or len(name) > 63 or name[:1] in (b'#', b'-', b'.'):
            raise ValueError("Invalid union name")
        additions.append(struct.pack('<64sI', name, len(union)) + union)
    return bytes(fixed[:lod.end_tag]) + b''.join(additions) + bytes(fixed[lod.end_tag:])


def repair_models(documents: dict[str, bytes]) -> dict[str, bytes]:
    changed = {}
    for key in MODEL_MEMBERS:
        if key not in documents:
            continue
        unions = _M1_UNIONS if key.startswith('lobo_m1/') else _MERKAVA_UNIONS
        fixed = add_hitpoint_unions(documents[key], unions)
        if fixed != documents[key]:
            changed[key] = fixed
    return changed
