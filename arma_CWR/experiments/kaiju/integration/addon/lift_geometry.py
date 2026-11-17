r"""Lifts the collision shape of a creature off the ground, in place, in a built P3D.

    python lift_geometry.py <model.p3d> [--floor 7.0] [--out <other.p3d>]

Why. The engine pushes a Man out of every man and vehicle his Geometry LOD overlaps, with no
regard for mass (World/Entities/Infantry/SoldierOldSim.cpp, the collision pass; measured by the
crush feature: a standing man moved the 20 000 t creature 3.3 m, a tank hull 6 m at every stride;
by the finals: a soldier inside a leg box moved D 10.6 m; by the brain: an 8 m jump in the demo).
Nothing on the ground is taller than a helicopter on its wheels (about 5 m). So every point of
the Geometry LOD under the floor is raised to it: men, cars, tanks and wrecks pass under his
collision shape and can no longer shove him, while aircraft still fly into his body, and the
shape keeps its radius (which sets how hard a hit counts) and its mass. Only the Geometry LOD
(resolution 1e13) is touched. FireGeometry stays as built, so a shot at his shin still hits.
What his legs do to things on the ground is the crush feature's scripted work, as before.

How. The file is patched in place: only the Y of the Geometry LOD's low points changes. A point
at height y under the floor goes to floor + 0.5 * y / floor, so a box that lay wholly under the
floor keeps a little thickness and its points keep their order (no flat, inverted or merged
component). Layout: mlod_writer.py in apps\tools\BlenderAddon\io_import_p3d.
"""
import argparse
import struct
import sys

GEOMETRY_SPEC = 1e13
FACE_SIZE = 32 + 4 + 4 * 16 + 4


def patch(data, floor):
    buf = bytearray(data)
    if buf[:4] != b"MLOD":
        raise SystemExit("not an MLOD P3D")
    _ver, nlods = struct.unpack_from("<ii", buf, 4)
    pos = 12
    report = []
    for li in range(nlods):
        if buf[pos:pos + 4] != b"SP3X":
            raise SystemExit("LOD %d: magic %r, expected SP3X" % (li, bytes(buf[pos:pos + 4])))
        head, _v, npos, nnorm, nface, _fl = struct.unpack_from("<iiiiii", buf, pos + 4)
        pts = pos + max(head, 28)  # the header size counts the magic
        p = pts + 16 * npos + 12 * nnorm + FACE_SIZE * nface
        if buf[p:p + 4] != b"TAGG":
            raise SystemExit("LOD %d: TAGG not where the layout says (offset %d)" % (li, p))
        p += 4
        while True:
            name = bytes(buf[p:p + 64]).split(b"\0", 1)[0].decode("cp1252")
            (size,) = struct.unpack_from("<i", buf, p + 64)
            p += 68 + size
            if name.lower() == "#endoffile#":
                break
        (res,) = struct.unpack_from("<f", buf, p)
        p += 4
        if abs(res - GEOMETRY_SPEC) < GEOMETRY_SPEC * 1e-3:
            lifted, lo, hi = 0, 1e9, -1e9
            mn, mx = [1e9] * 3, [-1e9] * 3
            for i in range(npos):
                off = pts + 16 * i + 4
                (y,) = struct.unpack_from("<f", buf, off)
                lo, hi = min(lo, y), max(hi, y)
                if floor > 0 and y < floor:
                    struct.pack_into("<f", buf, off, floor + 0.5 * max(y, 0.0) / floor)
                    lifted += 1
                xyz = struct.unpack_from("<fff", buf, pts + 16 * i)
                for k in range(3):
                    mn[k], mx[k] = min(mn[k], xyz[k]), max(mx[k], xyz[k])
            # the engine's geometry radius: half the diagonal of the Geometry LOD's box (ShapeLOD.cpp)
            radius = 0.5 * sum((mx[k] - mn[k]) ** 2 for k in range(3)) ** 0.5
            report.append("GEO_RADIUS %.3f" % radius)
            report.append("Geometry LOD #%d: %d points, heights %.2f to %.2f m, %d lifted to the %.2f m floor"
                          % (li, npos, lo, hi, lifted, floor))
        pos = p
    if pos != len(buf):
        raise SystemExit("the file has %d bytes after its last LOD: not patched" % (len(buf) - pos))
    if not report:
        report.append("no Geometry LOD (resolution 1e13): nothing to lift")
    return bytes(buf), report


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("p3d")
    ap.add_argument("--floor", type=float, default=7.0)
    ap.add_argument("--out", default="")
    a = ap.parse_args()
    data = open(a.p3d, "rb").read()
    out, report = patch(data, a.floor)
    with open(a.out or a.p3d, "wb") as f:
        f.write(out)
    for line in report:
        print(line)


if __name__ == "__main__":
    sys.exit(main())
