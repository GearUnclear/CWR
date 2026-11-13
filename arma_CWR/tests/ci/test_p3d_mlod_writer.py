"""No-Blender tests for the MLOD (SP3X) writer used by the Blender P3D exporter.

Run: python -m unittest discover -s arma_CWR/tests/ci -p 'test_p3d_*.py' -v
"""
from pathlib import Path
import struct
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'apps' / 'tools' / 'BlenderAddon' / 'io_import_p3d'))
import mlod_writer as mw  # noqa: E402


def box(lod, center, half, texture='', selection=None):
    """Append an axis-aligned box of 6 quads (engine front-facing order) to an MlodLod."""
    cx, cy, cz = center
    hx, hy, hz = half
    base = len(lod.points)
    for dx in (-1, 1):
        for dy in (-1, 1):
            for dz in (-1, 1):
                lod.points.append((cx + dx * hx, cy + dy * hy, cz + dz * hz))
    idx = {}
    i = base
    for dx in (-1, 1):
        for dy in (-1, 1):
            for dz in (-1, 1):
                idx[(dx, dy, dz)] = i
                i += 1
    sides = [((1, 0, 0), [(1, -1, -1), (1, 1, -1), (1, 1, 1), (1, -1, 1)]),
             ((-1, 0, 0), [(-1, -1, 1), (-1, 1, 1), (-1, 1, -1), (-1, -1, -1)]),
             ((0, 1, 0), [(-1, 1, -1), (-1, 1, 1), (1, 1, 1), (1, 1, -1)]),
             ((0, -1, 0), [(-1, -1, 1), (-1, -1, -1), (1, -1, -1), (1, -1, 1)]),
             ((0, 0, 1), [(1, -1, 1), (1, 1, 1), (-1, 1, 1), (-1, -1, 1)]),
             ((0, 0, -1), [(-1, -1, -1), (-1, 1, -1), (1, 1, -1), (1, -1, -1)])]
    first_face = len(lod.faces)
    for normal, corners in sides:
        lod.normals.append(normal)
        n = len(lod.normals) - 1
        uv = [(0.0, 0.0), (0.0, 1.0), (1.0, 1.0), (1.0, 0.0)]
        engine = [(idx[c], n, u, v) for c, (u, v) in zip(corners, uv)]
        lod.faces.append(mw.MlodFace(corners=mw.file_order(engine), texture=texture))
    if selection:
        sel = lod.get_or_add_selection(selection)
        for p in range(base, base + 8):
            sel.points[p] = 1.0
        sel.faces.update(range(first_face, first_face + 6))
    return base


def creature():
    """A two-box creature: body on 'hrudnik', head on 'hlava', a shared weighted neck point."""
    vis = mw.MlodLod(resolution=1.0)
    box(vis, (0.0, 0.0, 0.0), (0.2, 0.4, 0.15), texture=r'chupa\chupa_co.paa', selection='hrudnik')
    head = box(vis, (0.0, 0.6, 0.05), (0.12, 0.12, 0.14), texture=r'chupa\chupa_co.paa', selection='hlava')
    vis.get_or_add_selection('hrudnik').points[head] = 0.5  # one partially weighted head point
    vis.properties.append(('lodnoshadow', '1'))
    geo = mw.MlodLod(resolution=mw.GEOMETRY_SPEC)
    box(geo, (0.0, 0.0, 0.0), (0.2, 0.4, 0.15), selection='hrudnik')
    box(geo, (0.0, 0.6, 0.05), (0.12, 0.12, 0.14), selection='hlava')
    geo.properties.append(('autocenter', '0'))
    geo.mass = [60.0 / 16] * 16
    mem = mw.MlodLod(resolution=mw.MEMORY_SPEC, points=[(0.0, 0.6, 0.2), (0.0, 0.5, 0.0)])
    mem.get_or_add_selection('zamerny').points[0] = 1.0
    mem.get_or_add_selection('osa mireni').points[1] = 1.0
    land = mw.MlodLod(resolution=mw.LANDCONTACT_SPEC, points=[(0.0, -0.4, 0.0)])
    # deliberately out of order: the writer must sort
    return [geo, mem, vis, land]


class WriterLayout(unittest.TestCase):
    def setUp(self):
        self.lods = creature()
        self.data = mw.write_mlod(self.lods)

    def test_header_matches_engine_reader(self):
        magic, ver, nlods = struct.unpack_from('<4sii', self.data, 0)
        self.assertEqual(magic, b'MLOD')
        self.assertEqual(ver >> 8, 1)  # LODShape::Load accepts major 1 only
        self.assertEqual(nlods, 4)
        self.assertEqual(self.data[12:16], b'SP3X')
        head = struct.unpack_from('<iiiiii', self.data, 16)
        self.assertEqual(head[0], 28)

    def test_face_record_is_104_bytes(self):
        lod = mw.MlodLod(resolution=0.0, points=[(0, 0, 0), (1, 0, 0), (0, 1, 0)], normals=[(0, 0, 1)])
        lod.faces.append(mw.MlodFace(corners=[(0, 0, 0, 0), (1, 0, 1, 0), (2, 0, 0, 1)], texture='a'))
        empty = mw.MlodLod(resolution=0.0, points=[(0, 0, 0), (1, 0, 0), (0, 1, 0)], normals=[(0, 0, 1)])
        self.assertEqual(len(mw.write_lod(lod)) - len(mw.write_lod(empty)), 104)

    def test_round_trip_strict(self):
        ver, lods, warnings = mw.read_mlod(self.data, strict=True)
        self.assertEqual(warnings, [])
        f32 = [struct.unpack('<f', struct.pack('<f', l.resolution))[0] for l in self.lods]
        self.assertEqual([l.resolution for l in lods], sorted(f32))  # stored as float, ascending
        vis = lods[0]
        self.assertEqual(len(vis.points), 16)
        self.assertEqual(len(vis.faces), 12)
        self.assertEqual({f.texture for f in vis.faces}, {r'chupa\chupa_co.paa'})
        self.assertEqual(vis.properties, [('lodnoshadow', '1')])
        hr = vis.selection('hrudnik')
        self.assertEqual(sorted(hr.faces), list(range(6)))
        self.assertAlmostEqual(hr.points[8], 128 / 255)  # 0.5 quantized
        self.assertEqual(hr.points[0], 1.0)
        geo = next(l for l in lods if mw.is_spec(l.resolution, mw.GEOMETRY_SPEC))
        self.assertAlmostEqual(sum(geo.mass), 60.0, places=4)
        self.assertEqual(geo.properties, [('autocenter', '0')])
        mem = next(l for l in lods if mw.is_spec(l.resolution, mw.MEMORY_SPEC))
        self.assertEqual(len(mem.faces), 0)
        self.assertEqual(sorted(s.name for s in mem.selections), ['osa mireni', 'zamerny'])

    def test_winding_is_reversed_in_file(self):
        engine = [(0, 0, 0.0, 0.0), (1, 0, 1.0, 0.0), (2, 0, 0.0, 1.0)]
        lod = mw.MlodLod(resolution=0.0, points=[(0, 0, 0), (1, 0, 0), (0, 1, 0)], normals=[(0, 0, 1)])
        lod.faces.append(mw.MlodFace(corners=mw.file_order(engine)))
        _, (back,), _ = mw.read_mlod(mw.write_mlod([lod]))
        # The engine reverses on load (Shape::LoadTagged poly.Reverse()).
        self.assertEqual([c[0] for c in reversed(back.faces[0].corners)], [0, 1, 2])

    def test_selection_bytes(self):
        vis = self.lods[2]
        raw = mw.write_lod(vis)
        npos, nface = len(vis.points), len(vis.faces)
        tag = mw._fixed('hrudnik', 64, 'name') + struct.pack('<i', npos + nface)
        at = raw.index(tag) + len(tag)
        payload = raw[at:at + npos + nface]
        self.assertEqual(payload[0], 1)  # full weight
        self.assertEqual(payload[8], 256 - 128)  # O2 inverted partial weight
        self.assertEqual(payload[9], 0)
        self.assertEqual(list(payload[npos:npos + 6]), [1] * 6)
        self.assertEqual(list(payload[npos + 6:]), [0] * 6)


class WeightEncoding(unittest.TestCase):
    def test_values(self):
        self.assertEqual(mw.encode_weight(1.0), 1)
        self.assertEqual(mw.encode_weight(0.0), 0)
        self.assertEqual(mw.encode_weight(1e-4), 0)
        self.assertEqual(mw.encode_weight(1 / 255), 255)
        for w in (0.1, 0.25, 0.5, 0.75, 0.99):
            self.assertAlmostEqual(mw.decode_weight(mw.encode_weight(w)), w, delta=0.5 / 255)
        # engine view: (byte)(-val) / 255
        b = mw.encode_weight(0.5)
        self.assertAlmostEqual(((-b) & 0xFF) / 255, 0.5, delta=0.5 / 255)


class Validation(unittest.TestCase):
    def tri_lod(self, **kw):
        lod = mw.MlodLod(resolution=0.0, points=[(0, 0, 0), (1, 0, 0), (0, 1, 0)], normals=[(0, 0, 1)])
        lod.faces.append(mw.MlodFace(corners=[(0, 0, 0, 0), (1, 0, 1, 0), (2, 0, 0, 1)], **kw))
        return lod

    def test_texture_31_chars_ok_32_rejected(self):
        mw.write_lod(self.tri_lod(texture='a' * 31))
        with self.assertRaises(mw.MlodError):
            mw.write_lod(self.tri_lod(texture='a' * 32))

    def test_bad_indices_rejected(self):
        lod = self.tri_lod()
        lod.faces[0].corners[2] = (3, 0, 0, 0)
        with self.assertRaises(mw.MlodError):
            mw.write_lod(lod)

    def test_ngon_rejected(self):
        lod = self.tri_lod()
        lod.points.append((1, 1, 0))
        lod.faces[0].corners.append((3, 0, 1, 1))
        lod.faces[0].corners.append((3, 0, 1, 1))
        with self.assertRaises(mw.MlodError):
            mw.write_lod(lod)

    def test_mass_only_on_geometry(self):
        lod = self.tri_lod()
        lod.mass = [1.0, 1.0, 1.0]
        with self.assertRaises(mw.MlodError):
            mw.write_lod(lod)

    def test_duplicate_resolution_rejected(self):
        with self.assertRaises(mw.MlodError):
            mw.write_mlod([self.tri_lod(), self.tri_lod()])

    def test_high_face_flag_bits_round_trip(self):
        lod = self.tri_lod(flags=0x02000000 | 0x10)  # user material 1 + no shadow
        _, (back,), _ = mw.read_mlod(mw.write_mlod([lod]))
        self.assertEqual(back.faces[0].flags, 0x02000010)

    def test_invalid_face_flags_rejected(self):
        with self.assertRaises(mw.MlodError):
            mw.write_lod(self.tri_lod(flags=0x40))


class Resolutions(unittest.TestCase):
    def test_special_names_match_engine_constants(self):
        self.assertEqual(mw.parse_resolution('Geometry'), 1e13)
        self.assertEqual(mw.parse_resolution('Memory'), 1e15)
        self.assertEqual(mw.parse_resolution('LandContact'), 2e15)
        self.assertEqual(mw.parse_resolution('HitPoints'), 5e15)
        self.assertEqual(mw.parse_resolution('View Geometry'), 6e15)
        self.assertEqual(mw.parse_resolution('fire_geometry'), 7e15)
        self.assertEqual(mw.parse_resolution('ViewPilot'), 1100.0)
        self.assertEqual(mw.parse_resolution('0.15'), 0.15)
        self.assertEqual(mw.parse_resolution('LOD 1.0'), 1.0)
        self.assertEqual(mw.parse_resolution('5m'), 5.0)
        self.assertIsNone(mw.parse_resolution('Visual'))
        with self.assertRaises(mw.MlodError):
            mw.parse_resolution('ShadowVolume')

    def test_float32_special_lods_still_match(self):
        f32 = struct.unpack('<f', struct.pack('<f', 7e15))[0]
        self.assertTrue(mw.is_spec(f32, mw.FIRE_GEOM_SPEC))
        self.assertEqual(mw.lod_name(f32), 'firegeometry')


class Components(unittest.TestCase):
    def test_one_component_per_closed_piece(self):
        geo = mw.MlodLod(resolution=mw.GEOMETRY_SPEC)
        box(geo, (0, 0, 0), (0.2, 0.4, 0.15))
        box(geo, (0, 1, 0), (0.1, 0.1, 0.1))
        self.assertEqual(mw.auto_components(geo), 2)
        names = [s.name for s in geo.selections]
        self.assertEqual(names, ['Component01', 'Component02'])
        self.assertEqual(len(geo.selections[0].faces), 6)
        self.assertEqual(mw.auto_components(geo), 0)  # never twice
        mw.read_mlod(mw.write_mlod([geo]), strict=True)


if __name__ == '__main__':
    unittest.main()
