#!/usr/bin/env python3
"""Generate the @udisland fixture: a tiny GPL-clean island mod (issue #56 task 7).

Writes addons/udisland.pbo holding

  udisland.wrp            a 64x64-cell RVW v4 ("4WVR") world at 50 m, a dome
                          island in a sea, three towns, a road net
  houses/house_a.p3d      an MLOD box (10 x 6 x 10 m) tagged class=house
  roads/silnice_a.p3d     an MLOD flat slab (50 x 10 m) tagged class=road
  config.cpp              CfgPatches (worlds[]), CfgWorldList, CfgWorlds with a
                          Names block, and the House-derived classes the runtime
                          needs to turn the placed shapes into Building objects

Every byte is written from scratch here: no Bohemia data is copied, so the
fixture can live in the tree and mount in CI next to the engine.

Why these shapes: the Guerrilla tooling classifies a world by model path
(Guerrilla::ModelIsRoad / ModelIsBuilding in IslandScaffold.cpp: a "roads"
directory, a "silnice" stem, a "hous" directory) and the mission-time
settlement probe (LandscapeSettlementProbe in ZoneRegistry.cpp) accepts a
Names entry as a town when at least three Building objects with a bounding
sphere over 4 m stand within 300 m of it on dry land. A 10 m house box has a
bounding radius of about 7.7 m. The runtime makes a wrp object a Building only
when its LOD-0 "class" property is "house" AND a CfgVehicles class with
simulation "house" names the same model, which is what the config.cpp here
provides (Landscape::ObjectCreate -> NewObject -> VehicleTypes lookup).

Formats are written exactly as the engine reads them:
  * RVW v4:  LandSave.cpp Landscape::LoadData (magic "4WVR", short heights at
             LANDDATA_SCALE 0.045, short texture indices, 512 x 32-byte texture
             names, SingleObject4 records {float[12] matrix, int id, char[76]})
  * MLOD:    ShapeLOD.cpp / ShapeSetup.cpp Shape::LoadTagged ("MLOD", version
             0x101, nLods, then per LOD "SP3X" header, points, normals, faces,
             "TAGG" tags, resolution float); the tag layout mirrors the
             tests/fixtures/packages/mini shapes byte for byte
  * PBO:     the OFP-era header (name, packing, original size, reserved,
             timestamp, data size), a blank terminator entry, then the data

Run from anywhere:  python generate_udisland.py
"""
from __future__ import annotations

import math
import struct
from pathlib import Path

HERE = Path(__file__).resolve().parent
ADDONS = HERE / "addons"

GRID = 64  # cells per side
CELL = 50.0  # metres per cell (landGrid)
SIZE = GRID * CELL  # 3200 m
CENTRE = SIZE / 2
LANDDATA_SCALE = 0.045

# Towns: name, x, z (metres). All well inside the dome (radius 1500 m).
TOWNS = [
    ("Northam", 1000.0, 2350.0),
    ("Eastwick", 2350.0, 1150.0),
    ("Southmere", 900.0, 900.0),
]
HOUSES_PER_TOWN = 5
ROAD_STEP = 50.0  # one road slab per 50 m along each leg

HOUSE_MODEL = r"udisland\houses\house_a.p3d"
ROAD_MODEL = r"udisland\roads\silnice_a.p3d"


# ---------------------------------------------------------------------------
# terrain
# ---------------------------------------------------------------------------
def ground_height(x: float, z: float) -> float:
    """A dome island: +40 m at the centre, sea level at 1500 m, sea beyond."""
    d = math.hypot(x - CENTRE, z - CENTRE)
    h = 40.0 * (1.0 - (d / 1500.0) ** 2)
    return max(h, -12.0)


def fixed(value: str, size: int) -> bytes:
    raw = value.encode("ascii")
    if len(raw) >= size:
        raise ValueError(f"{value!r} does not fit in {size} bytes")
    return raw + bytes(size - len(raw))


def road_objects() -> list[tuple[float, float, float]]:
    """(x, z, heading) for one slab per ROAD_STEP along each town-to-town leg."""
    out = []
    legs = [(TOWNS[0], TOWNS[1]), (TOWNS[1], TOWNS[2]), (TOWNS[2], TOWNS[0])]
    for (_, ax, az), (_, bx, bz) in legs:
        length = math.hypot(bx - ax, bz - az)
        heading = math.atan2(bx - ax, bz - az)
        n = int(length // ROAD_STEP)
        for i in range(n + 1):
            t = i / max(n, 1)
            out.append((ax + (bx - ax) * t, az + (bz - az) * t, heading))
    return out


def house_objects() -> list[tuple[float, float, float]]:
    out = []
    for _, tx, tz in TOWNS:
        for i in range(HOUSES_PER_TOWN):
            ang = i * (2 * math.pi / HOUSES_PER_TOWN)
            r = 45.0
            out.append((tx + r * math.cos(ang), tz + r * math.sin(ang), ang))
    return out


def rotation_rows(heading: float) -> tuple[float, ...]:
    """Row-major 3x3 rotation about Y for a heading in radians (engine matrix layout)."""
    c, s = math.cos(heading), math.sin(heading)
    return (c, 0.0, s, 0.0, 1.0, 0.0, -s, 0.0, c)


def build_wrp() -> bytes:
    out = bytearray(b"4WVR")
    out += struct.pack("<ii", GRID, GRID)
    heights = []
    for zc in range(GRID):
        for xc in range(GRID):
            h = ground_height(xc * CELL, zc * CELL)
            heights.append(int(round(h / LANDDATA_SCALE)))
    out += struct.pack(f"<{GRID * GRID}h", *heights)
    # texture index per cell: 0 is always the engine's water texture
    # (Landscape::SetTexture special-cases index 0), 1 is the land texture
    tex = []
    for zc in range(GRID):
        for xc in range(GRID):
            tex.append(0 if ground_height(xc * CELL, zc * CELL) <= 0.0 else 1)
    out += struct.pack(f"<{GRID * GRID}h", *tex)
    # 512 texture names of 32 bytes. Index 1 names a Classic landtext so a
    # boot on the Classic package renders grass; on the config-only mini
    # package nothing loads textures.
    names = [r"landtext\mo.pac", r"landtext\tr.pac"]
    for i in range(512):
        out += fixed(names[i] if i < len(names) else "", 32)
    object_id = 1
    for x, z, heading in house_objects():
        y = ground_height(x, z)
        out += struct.pack("<12f", *rotation_rows(heading), x, y, z)
        out += struct.pack("<i", object_id)
        out += fixed(HOUSE_MODEL, 76)
        object_id += 1
    for x, z, heading in road_objects():
        y = ground_height(x, z)
        out += struct.pack("<12f", *rotation_rows(heading), x, y, z)
        out += struct.pack("<i", object_id)
        out += fixed(ROAD_MODEL, 76)
        object_id += 1
    return bytes(out)


# ---------------------------------------------------------------------------
# MLOD shapes
# ---------------------------------------------------------------------------
def mlod_lod(points, normals, faces, properties, resolution: float) -> bytes:
    """One SP3X LOD as Shape::LoadTagged reads it.

    points:  [(x, y, z)]            normals: [(x, y, z)]
    faces:   [[(point, normal, u, v), ...]]  (3 or 4 vertices, texture "")
    properties: [(name, value)]
    """
    out = bytearray(b"SP3X")
    out += struct.pack("<iiiiii", 28, 1, len(points), len(normals), len(faces), 0)
    for x, y, z in points:
        out += struct.pack("<fffi", x, y, z, 0)
    for x, y, z in normals:
        out += struct.pack("<fff", x, y, z)
    for face in faces:
        out += fixed("", 32)
        out += struct.pack("<i", len(face))
        for i in range(4):
            if i < len(face):
                p, n, u, v = face[i]
            else:
                p, n, u, v = 0, 0, 0.0, 0.0
            out += struct.pack("<iiff", p, n, u, v)
        out += struct.pack("<i", 0)
    out += b"TAGG"
    for name, value in properties:
        out += fixed("#Property#", 64)
        out += struct.pack("<i", 128)
        out += fixed(name, 64)
        out += fixed(value, 64)
    out += fixed("#EndOfFile#", 64)
    out += struct.pack("<i", 0)
    out += struct.pack("<f", resolution)
    return bytes(out)


def mlod_file(lods: list[bytes]) -> bytes:
    return b"MLOD" + struct.pack("<ii", 0x101, len(lods)) + b"".join(lods)


def box_lod(sx: float, sy: float, sz: float, properties, resolution: float) -> bytes:
    """An axis-aligned box standing on y = 0, centred at the origin in x/z."""
    hx, hz = sx / 2, sz / 2
    pts = [
        (-hx, 0.0, -hz), (hx, 0.0, -hz), (hx, 0.0, hz), (-hx, 0.0, hz),
        (-hx, sy, -hz), (hx, sy, -hz), (hx, sy, hz), (-hx, sy, hz),
    ]
    norms = [(0, 1, 0), (0, -1, 0), (1, 0, 0), (-1, 0, 0), (0, 0, 1), (0, 0, -1)]
    def quad(a, b, c, d, n):
        return [(a, n, 0.0, 0.0), (b, n, 1.0, 0.0), (c, n, 1.0, 1.0), (d, n, 0.0, 1.0)]
    faces = [
        quad(4, 5, 6, 7, 0),  # top
        quad(3, 2, 1, 0, 1),  # bottom
        quad(1, 2, 6, 5, 2),  # +x
        quad(0, 4, 7, 3, 3),  # -x
        quad(2, 3, 7, 6, 4),  # +z
        quad(0, 1, 5, 4, 5),  # -z
    ]
    return mlod_lod(pts, norms, faces, properties, resolution)


def slab_lod(sx: float, sz: float, properties, resolution: float) -> bytes:
    """A flat quad on y = 0 (a road piece)."""
    hx, hz = sx / 2, sz / 2
    pts = [(-hx, 0.0, -hz), (hx, 0.0, -hz), (hx, 0.0, hz), (-hx, 0.0, hz)]
    norms = [(0.0, 1.0, 0.0)]
    faces = [[(0, 0, 0.0, 0.0), (1, 0, 1.0, 0.0), (2, 0, 1.0, 1.0), (3, 0, 0.0, 1.0)]]
    return mlod_lod(pts, norms, faces, properties, resolution)


def build_house() -> bytes:
    # LOD 0 (resolution 1.0) is where the runtime reads the class hint. The
    # geometry LOD (1e13) gives the box a collision volume so units cannot
    # walk through it.
    props = [("class", "house"), ("fixture", "udisland")]
    return mlod_file([box_lod(10.0, 6.0, 10.0, props, 1.0), box_lod(10.0, 6.0, 10.0, [], 1.0e13)])


def build_road() -> bytes:
    props = [("class", "road"), ("fixture", "udisland")]
    return mlod_file([slab_lod(10.0, 52.0, props, 1.0)])


# ---------------------------------------------------------------------------
# config
# ---------------------------------------------------------------------------
def build_config() -> str:
    names = "\n".join(
        f"            class {n}\n"
        f"            {{\n"
        f'                name = "{n}";\n'
        f"                position[] = {{{x:.0f}, {z:.0f}, 100}};\n"
        f"            }};"
        for n, x, z in TOWNS
    )
    return f"""// @udisland: a synthetic Guerrilla island for the clean-clone CI lane
// (issue #56 task 7). Generated by generate_udisland.py; edit that, not this.
//
// A world addon in the shape the engine expects from a third-party island
// pack (compare @LoBo's lost.pbo): CfgPatches names the world so the addon
// walk activates it, CfgWorldList lists it for the island menus, CfgWorlds
// carries the description, the .wrp path inside this pbo and the Names block
// the Guerrilla scaffold and auto-seed read. The House-derived classes are what
// turn the placed house_a shapes into Building objects at runtime (the
// settlement probe counts Buildings, not shapes).

#define private   0
#define protected 1
#define public    2

class CfgPatches
{{
    class udisland
    {{
        units[] = {{}};
        weapons[] = {{}};
        worlds[] = {{"UdIsland"}};
        requiredVersion = 1.30;
        requiredAddons[] = {{}};
    }};
}};

class CfgWorldList
{{
    class UdIsland {{}};
}};

class CfgWorlds
{{
    // re-declared empty so a standalone parse of this addon config sees the
    // base chain; the deferred merge lays it back onto the real DefaultWorld
    class DefaultWorld {{}};
    class UdIsland : DefaultWorld
    {{
        access = 3;
        description = "UD Island";
        icon = "";
        worldName = "\\udisland\\udisland.wrp";
        cutscenes[] = {{}};
        plateFormat = "UD$ - #####";
        plateLetters = "ABCDEFHIKLMOPRSTVXYZ";
        landGrid = {CELL:.0f};
        centerPosition[] = {{{CENTRE:.0f}, {CENTRE:.0f}, 0}};
        latitude = -40;
        longitude = 15;
        startTime = "9:00";
        startDate = "1/6/85";
        startWeather = 0.2;
        startFog = 0;
        forecastWeather = 0.2;
        forecastFog = 0;
        seagullPos[] = {{{CENTRE:.0f}, {CENTRE:.0f}, 60}};
        ilsPosition[] = {{{CENTRE:.0f}, {CENTRE:.0f}, 0}};
        ilsDirection[] = {{0, 0.08, -1}};
        ilsTaxiIn[] = {{}};
        ilsTaxiOff[] = {{}};
        class Sounds
        {{
            sounds[] = {{}};
        }};
        class Animation
        {{
            vehicles[] = {{}};
        }};
        class Names
        {{
{names}
        }};
    }};
}};

class CfgVehicles
{{
    class All {{}};
    class AllVehicles : All {{}};
    class Static : All {{}};
    class NonStrategic : Static {{}};
    class House : NonStrategic {{}};
    // the runtime looks a placed shape up by model path + simulation "house"
    // (inherited from Static); scope 1 = protected, the editor never lists it
    class UDIslandHouseA : House
    {{
        scope = protected;
        model = "\\udisland\\houses\\house_a";
        displayName = "UD House";
    }};
}};
"""


# ---------------------------------------------------------------------------
# PBO
# ---------------------------------------------------------------------------
def build_pbo(entries: list[tuple[str, bytes]]) -> bytes:
    def header(name: str, size: int) -> bytes:
        return name.encode("ascii") + b"\0" + struct.pack("<IIIII", 0, size, 0, 0, size)

    out = bytearray()
    for name, data in entries:
        out += header(name, len(data))
    out += header("", 0)
    for _, data in entries:
        out += data
    return bytes(out)


def main() -> None:
    ADDONS.mkdir(parents=True, exist_ok=True)
    entries = [
        ("config.cpp", build_config().encode("ascii")),
        ("udisland.wrp", build_wrp()),
        (r"houses\house_a.p3d", build_house()),
        (r"roads\silnice_a.p3d", build_road()),
    ]
    pbo = build_pbo(entries)
    (ADDONS / "udisland.pbo").write_bytes(pbo)
    print(f"wrote {ADDONS / 'udisland.pbo'} ({len(pbo)} bytes): "
          f"{len(house_objects())} houses, {len(road_objects())} road slabs, {len(TOWNS)} towns")


if __name__ == "__main__":
    main()
