"""No-Blender tests for the RTM (runtime animation) reader/writer, the engine
playback math reimplementation and the stock rig derivation.

Run: python -m unittest discover -s arma_CWR/tests/ci -p "test_rtm_*.py" -v

All data here is synthetic. Set UD_RTM_STOCK_DIR to a folder of extracted
stock RTMs (local only, never in the repo) to also check that every one of
them reads and rewrites byte for byte.
"""
from pathlib import Path
import math
import os
import random
import struct
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'apps' / 'tools' / 'BlenderAddon' / 'io_import_p3d'))
import rtm_io as rio  # noqa: E402
import rtm_engine as reng  # noqa: E402
import rtm_rig as rrig  # noqa: E402


def rot_y(a, pos=(0.0, 0.0, 0.0)):
    """Rotation about +Y by angle a (right-handed formula on the column layout)."""
    c, s = math.cos(a), math.sin(a)
    # columns: aside = R*(1,0,0), up = R*(0,1,0), dir = R*(0,0,1)
    return (c, 0.0, -s, 0.0, 1.0, 0.0, s, 0.0, c) + tuple(pos)


def rot_axis(axis, a):
    x, y, z = axis
    n = math.sqrt(x * x + y * y + z * z)
    x, y, z = x / n, y / n, z / n
    c, s = math.cos(a), math.sin(a)
    t = 1 - c
    rows = [[t * x * x + c, t * x * y - s * z, t * x * z + s * y, 0.0],
            [t * x * y + s * z, t * y * y + c, t * y * z - s * x, 0.0],
            [t * x * z - s * y, t * y * z + s * x, t * z * z + c, 0.0]]
    return rio.mat_from_rows(rows)


def about(m, pivot):
    """Matrix m applied about a pivot point: T(p) * m * T(-p)."""
    t = rio.mat_apply(m, pivot)
    return m[:9] + tuple(m[9 + i] + pivot[i] - t[i] for i in range(3))


def translate(v):
    return rio.IDENTITY[:9] + tuple(v)


def f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def simple_anim(n_frames=3, bones=('hlava', 'krk'), step=(0.5, 0.0, -1.25)):
    frames = []
    for i in range(n_frames):
        t = i / (n_frames - 1) if n_frames > 1 else 0.0
        frames.append((t, [rot_y(0.3 * i + b, (0.1 * i, 0.2 * b, -0.3)) for b in range(len(bones))]))
    return rio.new_animation(bones, frames, step)


class LayoutTests(unittest.TestCase):
    def test_header_and_record_layout(self):
        anim = simple_anim()
        data = rio.write_rtm(anim)
        self.assertEqual(data[:8], b'RTM_0101')
        self.assertEqual(struct.unpack_from('<3f', data, 8), (0.5, 0.0, -1.25))
        self.assertEqual(struct.unpack_from('<2i', data, 20), (3, 2))
        self.assertEqual(data[28:60], b'hlava' + b'\0' * 27)
        self.assertEqual(data[60:92], b'krk' + b'\0' * 29)
        off = 92
        rec = 32 + 48
        self.assertEqual(len(data), off + 3 * (4 + 2 * rec))
        for i, fr in enumerate(anim.frames):
            self.assertEqual(struct.unpack_from('<f', data, off)[0], f32(fr.time))
            off += 4
            for b in range(2):
                self.assertEqual(data[off:off + 32], anim.bone_names[b].encode() + b'\0' * (32 - len(anim.bone_names[b])))
                off += 32
                got = struct.unpack_from('<12f', data, off)
                self.assertEqual(got, tuple(f32(v) for v in fr.matrices[b]))
                off += 48

    def test_matrix_columns_are_aside_up_dir(self):
        # Matrix4P stores aside, up, dir as columns: p' = aside*x + up*y + dir*z + pos
        m = (1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12)
        self.assertEqual(rio.mat_apply(m, (1, 0, 0)), (11, 13, 15))
        self.assertEqual(rio.mat_apply(m, (0, 1, 0)), (14, 16, 18))
        self.assertEqual(rio.mat_apply(m, (0, 0, 1)), (17, 19, 21))
        self.assertEqual(rio.mat_get(m, 1, 0), 2)  # Get(i, j) = (&aside)[j][i]
        self.assertEqual(rio.mat_get(m, 0, 3), 10)
        rows = rio.mat_to_rows(m)
        self.assertEqual(rows[0], [1, 4, 7, 10])
        self.assertEqual(rio.mat_from_rows(rows), tuple(float(v) for v in m))

    def test_mat_mul_is_composition(self):
        a = about(rot_axis((1, 2, 3), 0.7), (0.1, 0.2, 0.3))
        b = about(rot_axis((-2, 1, 0.5), -1.1), (1.0, -0.5, 0.25))
        p = (0.3, -0.7, 1.9)
        q = rio.mat_apply(a, rio.mat_apply(b, p))
        r = rio.mat_apply(rio.mat_mul(a, b), p)
        for i in range(3):
            self.assertAlmostEqual(q[i], r[i], places=12)

    def test_handedness_of_det(self):
        self.assertAlmostEqual(rio.mat_det(rot_axis((0.3, 1, -2), 2.0)), 1.0, places=12)
        mirror = (1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0)  # swap y and z
        self.assertAlmostEqual(rio.mat_det(mirror), -1.0)


class RoundTripTests(unittest.TestCase):
    def test_write_read_write_is_byte_exact(self):
        anim = simple_anim(5, bones=rio.STOCK_MAN_BONES)
        data = rio.write_rtm(anim)
        back = rio.read_rtm(data)
        self.assertEqual(back.bone_names, list(rio.STOCK_MAN_BONES))
        self.assertEqual(back.step, tuple(f32(s) for s in anim.step))
        self.assertEqual(rio.write_rtm(back), data)

    def test_raw_name_bytes_after_nul_survive(self):
        # Stock files keep junk after the NUL in name fields; keep it for exact rewrites.
        anim = simple_anim()
        data = bytearray(rio.write_rtm(anim))
        data[28 + 6:28 + 10] = b'JUNK'  # header field of 'hlava' after its NUL
        per_phase = 92 + 4 + 0  # first phase, first bone name field
        data[per_phase + 20:per_phase + 24] = b'XYZW'
        data += b'tail'
        back = rio.read_rtm(bytes(data))
        self.assertEqual(back.bone_names, ['hlava', 'krk'])
        self.assertEqual(back.trailing, b'tail')
        self.assertEqual(rio.write_rtm(back, verify=False), bytes(data))
        # Renaming a bone regenerates its fields instead of reusing stale raw bytes.
        back.bone_names[0] = 'hlavb'
        out = rio.write_rtm(back, verify=False)
        self.assertEqual(out[28:60], b'hlavb' + b'\0' * 27)

    def test_rtm_0100(self):
        data = bytearray(b'RTM_0100' + struct.pack('<f', -2.0) + struct.pack('<2i', 1, 1))
        data += b'krk' + b'\0' * 29 + struct.pack('<f', 0.0) + b'krk' + b'\0' * 29 + struct.pack('<12f', *rio.IDENTITY)
        anim = rio.read_rtm(bytes(data))
        self.assertEqual(anim.magic, rio.MAGIC_V100)
        self.assertEqual(anim.step, (0.0, 0.0, -2.0))
        self.assertEqual(rio.write_rtm(anim), bytes(data))
        anim.step = (1.0, 0.0, -2.0)
        with self.assertRaises(rio.RtmError):
            rio.write_rtm(anim)

    def test_names_lowercased_lookup_and_full_32_byte_name(self):
        anim = simple_anim(bones=('Hlava', 'KRK'))
        data = rio.write_rtm(anim, verify=False)
        back = rio.read_rtm(data)
        self.assertEqual(back.bone_index('hlava'), 0)
        self.assertEqual(back.bone_index('krk'), 1)
        errors, warnings = rio.check_rtm(back)
        self.assertEqual(errors, [])
        self.assertTrue(any('lowercase' in w for w in warnings))
        # A 32-byte name with no NUL is read like RTMReader does (it adds the NUL)
        raw = bytearray(data)
        raw[28:60] = b'a' * 32
        back = rio.read_rtm(bytes(raw))
        self.assertEqual(back.bone_names[0], 'a' * 32)
        errors, _ = rio.check_rtm(back)
        self.assertTrue(any('at most 31' in e for e in errors))


class StrictReaderTests(unittest.TestCase):
    def setUp(self):
        self.data = rio.write_rtm(simple_anim())

    def test_bad_magic(self):
        for magic in (b'RTM_0102', b'rtm_0101', b'RTM_01\0\0', b'MLOD\0\0\0\0'):
            with self.assertRaises(rio.RtmError):
                rio.read_rtm(magic + self.data[8:])

    def test_truncation_anywhere_fails(self):
        # readRTM returns false on any short read; so must we, at every cut point.
        for cut in (0, 4, 8, 12, 19, 20, 27, 28, 59, 60, 91, 92, 95, 96, 127, 128, 175, len(self.data) - 1):
            with self.assertRaises(rio.RtmError, msg=f'cut {cut}'):
                rio.read_rtm(self.data[:cut])

    def test_count_limits(self):
        for n_anim, n_sel in ((-1, 2), (3, -1), (100001, 2), (3, 1001)):
            bad = self.data[:20] + struct.pack('<2i', n_anim, n_sel) + self.data[28:]
            with self.assertRaises(rio.RtmError):
                rio.read_rtm(bad)
        ok = self.data[:20] + struct.pack('<2i', 0, 0)
        anim = rio.read_rtm(ok)  # the reader takes it ...
        errors, _ = rio.check_rtm(anim)
        self.assertTrue(any('no phases' in e for e in errors))  # ... the engine asserts
        with self.assertRaises(rio.RtmError):
            rio.read_rtm_strict(ok)

    def test_semantic_errors(self):
        def errs(anim, **kw):
            return rio.check_rtm(anim, **kw)[0]

        dup = simple_anim(bones=('krk', 'KRK'))
        self.assertTrue(any('twice' in e for e in errs(dup)))
        unsorted = simple_anim()
        unsorted.frames[1].time = 0.9
        unsorted.frames[2].time = 0.5
        self.assertTrue(any('ascend' in e for e in errs(unsorted)))
        nan = simple_anim()
        nan.frames[0].matrices[0] = (float('nan'),) + nan.frames[0].matrices[0][1:]
        self.assertTrue(any('not finite' in e for e in errs(nan)))
        mirrored = simple_anim()
        m = mirrored.frames[1].matrices[1]
        mirrored.frames[1].matrices[1] = (-m[0], -m[1], -m[2]) + m[3:]
        self.assertTrue(any('mirrored' in e for e in errs(mirrored)))
        with self.assertRaises(rio.RtmError):
            rio.write_rtm(mirrored)
        self.assertTrue(any("required bone 'zbran'" in e for e in errs(simple_anim(), require_bones=rio.STOCK_MAN_BONES)))
        self.assertEqual(errs(simple_anim(bones=rio.STOCK_MAN_BONES), require_bones=rio.STOCK_MAN_BONES), [])

    def test_semantic_warnings(self):
        anim = simple_anim()
        anim.frames[0].matrices[0] = (0.0,) * 9 + (0.0, 0.0, 0.0)  # collapsed bone (stock hides with this)
        anim.frames[2].time = 1.2
        anim.trailing = b'x'
        errors, warnings = rio.check_rtm(anim)
        self.assertEqual(errors, [])
        text = ' | '.join(warnings)
        self.assertIn('collapsed', text)
        self.assertIn('outside 0..1', text)
        self.assertIn('after the last phase', text)
        anim = simple_anim(4)
        anim.frames[2].time = 0.9995
        _, warnings = rio.check_rtm(anim, looped=True)
        self.assertTrue(any('drops phase 2' in w for w in warnings))

    def test_per_phase_name_mismatch_is_a_warning(self):
        data = bytearray(self.data)
        data[96:128] = b'zebra' + b'\0' * 27  # phase 0, bone 0 name field
        anim = rio.read_rtm_strict(bytes(data))
        self.assertTrue(any('per-phase name' in w for w in anim.warnings))
        self.assertEqual(anim.bone_names, ['hlava', 'krk'])


class EngineMathTests(unittest.TestCase):
    def frames(self, times):
        return rio.new_animation(['krk'], [(t, [translate((t, 0.0, 0.0))]) for t in times])

    def test_loop_frame_removed_and_wrap(self):
        anim = self.frames([0.0, 0.25, 0.5, 0.75, 1.0])
        looped = reng.EngineAnimation(anim, looped=True)
        self.assertEqual(looped.times, [0.0, 0.25, 0.5, 0.75])
        # between the last kept phase (0.75) and phase 0 (wraps to 1.0)
        m = looped.bone_matrix('krk', 0.875)
        self.assertAlmostEqual(m[9], 0.5 * 0.75 + 0.5 * 0.0)
        once = reng.EngineAnimation(anim, looped=False)
        self.assertEqual(len(once.times), 5)
        self.assertAlmostEqual(once.bone_matrix('krk', 0.875)[9], 0.875)
        self.assertAlmostEqual(once.bone_matrix('krk', 1.0)[9], 1.0)
        self.assertAlmostEqual(once.bone_matrix('krk', 0.0)[9], 0.0)

    def test_find_matches_engine_cases(self):
        e = reng.EngineAnimation(self.frames([0.0, 0.4, 1.0]), looped=False)
        self.assertEqual(e.find(0.0), (1, 0))
        self.assertEqual(e.find(0.5), (2, 1))
        self.assertEqual(e.find(1.0), (2, 2))
        e = reng.EngineAnimation(self.frames([0.1, 0.4, 0.9]), looped=True)
        self.assertEqual(e.find(0.05), (0, 2))  # before the first phase: wraps to the last
        p, n, w = e.interpolation(0.05)
        self.assertAlmostEqual(w, (0.05 - (0.9 - 1)) / (0.1 - (0.9 - 1)))

    def test_linear_blend_is_element_wise(self):
        a = rot_y(0.0)
        b = rot_y(math.pi / 2)
        anim = rio.new_animation(['krk'], [(0.0, [a]), (1.0, [b])])
        m = reng.EngineAnimation(anim, looped=False).bone_matrix('krk', 0.5)
        # element-wise lerp, not slerp: the blended aside is shorter than 1
        self.assertAlmostEqual(math.hypot(m[0], m[2]), math.sqrt(0.5), places=6)

    def test_skeleton_bones_missing_from_file_stay_identity(self):
        anim = rio.new_animation(['krk'], [(0.0, [translate((1, 2, 3))])])
        e = reng.EngineAnimation(anim, skeleton=['hlava', 'krk', 'zbran'])
        mats = e.matrices(0.0)
        self.assertEqual(mats[0], rio.IDENTITY)
        self.assertEqual(mats[1][9:], (1.0, 2.0, 3.0))
        self.assertEqual(mats[2], rio.IDENTITY)

    def test_reversed_is_a_half_turn_not_a_mirror(self):
        m = about(rot_axis((0.2, 1, 0.4), 0.8), (0.1, 0.5, -0.2))
        r = reng.reverse_matrix(m)
        self.assertAlmostEqual(rio.mat_det(r), 1.0, places=12)
        self.assertAlmostEqual(rio.mat_det(reng.REVERSE), 1.0)  # diag(-1, 1, -1) is a rotation
        p = (0.3, -0.4, 0.9)
        a = reng.reverse_point(rio.mat_apply(m, p))
        b = rio.mat_apply(r, reng.reverse_point(p))
        for i in range(3):
            self.assertAlmostEqual(a[i], b[i], places=12)

    def test_vertex_weights_quantize_and_normalize(self):
        sel = {'krk': {0: 1.0, 1: 0.5}, 'hlava': {1: 0.5, 2: 0.25}}
        w = reng.vertex_weights(sel, ['hlava', 'krk'], 4)
        self.assertEqual(w[0], [(1, 1.0)])
        self.assertEqual(w[2], [(0, 1.0)])  # one bone: weight forced to 1
        self.assertEqual(w[3], [])
        # vertex 1: bytes 128/255 -> floor(0.50196 * 128) = 64 each; normalized
        # then quantized: entry 1 gets 64/128, entry 0 gets floor((1.001 - 0.5) * 128) / 128
        self.assertEqual(w[1], [(0, 64 / 128), (1, 0.5)])

    def test_vertex_weights_max_four_and_entry_zero_kept(self):
        bones = ['a', 'b', 'c', 'd', 'e']
        # A fifth bone replaces the weakest entry, but AddSelection's `minI > 0`
        # never picks entry 0: when entry 0 is the weakest the fifth bone is dropped.
        sel = {b: {0: wt} for b, wt in zip(bones, (0.4, 0.1, 0.3, 0.2, 0.9))}
        w = reng.vertex_weights(sel, bones, 1)[0]
        self.assertEqual([b for b, _ in w], [0, 4, 2, 3])
        self.assertAlmostEqual(sum(x for _, x in w), 1.0, delta=0.02)
        sel = {b: {0: wt} for b, wt in zip(bones, (0.1, 0.2, 0.3, 0.4, 0.9))}
        w = reng.vertex_weights(sel, bones, 1)[0]
        self.assertEqual([b for b, _ in w], [0, 1, 2, 3])

    def test_skin_point(self):
        mats = [translate((1, 0, 0)), translate((0, 2, 0))]
        self.assertEqual(reng.skin_point((0, 0, 0), [(0, 0.5), (1, 0.5)], mats), (0.5, 1.0, 0.0))
        self.assertEqual(reng.skin_point((1, 2, 3), [], mats), (1, 2, 3))

    def test_estimate_step_from_planted_foot(self):
        # A foot planted from t=0 to t=0.5 slides by -step * dt, then lifts and swings back.
        step = (0.2, 0.0, -1.6)
        ankle = (0.0, -1.0, 0.2)
        frames = []
        for i in range(9):
            t = i / 8
            if t <= 0.5:
                off = (-step[0] * t, 1.0, -step[2] * t)
            else:
                u = (t - 0.5) / 0.5
                off = (-step[0] * 0.5 * (1 - u), 1.2 - 0.0 * u, -step[2] * 0.5 * (1 - u))
            frames.append((t, [translate(off)]))
        anim = rio.new_animation(['lchodidlo'], frames, step)
        est = reng.estimate_step(anim, {'l': [('lchodidlo', ankle)]})
        self.assertAlmostEqual(est[0], step[0], places=6)
        self.assertAlmostEqual(est[2], step[2], places=6)


class RigTests(unittest.TestCase):
    def synthetic_stock(self, pivots, n_frames=40, seed=7):
        """Man-like animations: every bone rotates about its joint, composed down the hierarchy."""
        rnd = random.Random(seed)
        order = rrig.bones_parent_first()
        frames = []
        for f in range(n_frames):
            world = {}
            for b in order:
                parent = rrig.STOCK_PARENTS[b]
                axis = (rnd.uniform(-1, 1), rnd.uniform(-1, 1), rnd.uniform(-1, 1))
                local = about(rot_axis(axis, rnd.uniform(-0.8, 0.8)), pivots.get(b, (0.0, 0.0, 0.0)))
                if parent is None:
                    local = rio.mat_mul(translate((rnd.uniform(-1, 1), 1.0, rnd.uniform(-1, 1))), local)
                    world[b] = local
                else:
                    world[b] = rio.mat_mul(world[parent], local)
            frames.append((f / (n_frames - 1), [world[b] for b in rio.STOCK_MAN_BONES]))
        return rio.new_animation(rio.STOCK_MAN_BONES, frames)

    def test_solve_pivot_recovers_joint(self):
        pivot = (0.12, -0.5, 0.3)
        pairs = []
        for k in range(12):
            parent = rio.mat_mul(translate((k * 0.1, 1, 0)), rot_axis((k, 1, 2), 0.1 * k))
            child = rio.mat_mul(parent, about(rot_axis((1, k, -1), 0.3 + 0.05 * k), pivot))
            pairs.append((parent, child))
        x, rms, _ = rrig.solve_pivot(pairs)
        for i in range(3):
            self.assertAlmostEqual(x[i], pivot[i], places=7)
        self.assertLess(rms, 1e-7)

    def test_rigidly_coupled_bones_fall_back_to_prior(self):
        m = rot_axis((1, 2, 3), 0.4)
        x, rms, _ = rrig.solve_pivot([(m, m)] * 5, prior=(1.0, 2.0, 3.0))
        self.assertEqual([round(v, 9) for v in x], [1.0, 2.0, 3.0])

    def test_derive_rest_joints_and_layout(self):
        rnd = random.Random(3)
        pivots = {b: (rnd.uniform(-0.5, 0.5), rnd.uniform(-1, 0.6), rnd.uniform(-0.3, 0.3))
                  for b in rio.STOCK_MAN_BONES}
        anim = self.synthetic_stock(pivots)
        rest = rrig.derive_rest_joints([anim])
        self.assertEqual(rest['files'], 1)
        for b, p in rest['pivots'].items():
            for i in range(3):
                self.assertAlmostEqual(p[i], pivots[b][i], places=4, msg=b)
            self.assertLess(rest['residual_m'][b], 1e-4)
        layout = rrig.rest_layout(rest)
        self.assertEqual(set(layout), set(rio.STOCK_MAN_BONES))
        self.assertEqual(layout['lloket']['head'], rest['pivots']['lloket'])
        self.assertEqual(layout['lbiceps']['tail'], rest['pivots']['lloket'])
        self.assertIsNone(layout['bricho']['parent'])
        for b, v in layout.items():
            self.assertGreater(math.dist(v['head'], v['tail']), 1e-3, b)

    def test_man_foot_contacts(self):
        rest = {'pivots': {b: (i, 0.0, 0.0) for i, b in enumerate(rio.STOCK_MAN_BONES)}}
        feet = rrig.man_foot_contacts(rest)
        self.assertEqual(sorted(feet), ['l', 'p'])
        self.assertEqual([b for b, _ in feet['l']], ['lchodidlo', 'lprsty'])

    def test_parent_first_order(self):
        order = rrig.bones_parent_first()
        self.assertEqual(sorted(order), sorted(rio.STOCK_MAN_BONES))
        for b in order:
            p = rrig.STOCK_PARENTS[b]
            if p:
                self.assertLess(order.index(p), order.index(b))


@unittest.skipUnless(os.environ.get('UD_RTM_STOCK_DIR'), 'set UD_RTM_STOCK_DIR to check local stock RTMs')
class StockRoundTripTests(unittest.TestCase):
    def test_every_stock_rtm_is_byte_exact(self):
        folder = Path(os.environ['UD_RTM_STOCK_DIR'])
        files = sorted(p for p in folder.rglob('*') if p.suffix.lower() == '.rtm')
        self.assertTrue(files, f'no .rtm files under {folder}')
        for p in files:
            data = p.read_bytes()
            anim = rio.read_rtm(data)
            self.assertEqual(rio.write_rtm(anim, verify=False), data, p.name)
            errors, _ = rio.check_rtm(anim)
            self.assertEqual(errors, [], p.name)
        print(f'\n[stock] {len(files)} RTMs byte exact, 0 strict errors')


if __name__ == '__main__':
    unittest.main()
