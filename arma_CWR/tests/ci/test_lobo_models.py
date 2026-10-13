"""No-game tests for LoBo model repair, using only synthetic redistributable data."""
from pathlib import Path
import struct
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'tools' / 'lobo'))
from repair import models
from repair.config import class_block, class_spans, scalar


def tag(name, payload):
    return struct.pack('<64sI', name.encode('ascii'), len(payload)) + payload


def lod(resolution, selections, *, points=4, faces=1):
    # The intentionally opaque geometry is never decoded or altered by repairs.
    geometry = bytes(range(16)) * points + b'F' * (104 * faces)
    return (struct.pack('<4s6i', b'SP3X', 28, 153, points, 0, faces, 0) + geometry + b'TAGG'
            + b''.join(tag(name, payload) for name, payload in selections)
            + tag('#Property#', b'opaque property bytes') + tag('#EndOfFile#', b'')
            + struct.pack('<f', resolution))


def model(*lods):
    return struct.pack('<4sII', b'MLOD', 257, len(lods)) + b''.join(lods)


def part(name, armor, material, selection):
    return f'class {name} {{armor={armor};material={material};name={selection};passThrough=1;}};'


def m1_config():
    standard = [part('HitEngine', '.8', 60, 'engine'), part('HitHull', '.8', 50, 'hull'),
                part('HitTurret', '.8', 51, 'turet'), part('HitGun', '6', 52, 'gun'),
                part('HitLTrack', '.6', 53, 'pasL'), part('HitRTrack', '.6', 54, 'pasP')]
    zones = [(side + 'Hull', armor, 50, side + 'hull')
             for side, armor in [('F', '1.6'), ('B', '0.6'), ('R', '1.25'), ('L', '1.25')]]
    zones += [(side + 'Turret', armor, 51, side + 'turet')
              for side, armor in [('F', '1.5'), ('B', '0.7'), ('L', '1.25'), ('R', '1.25'), ('T', '0.5')]]
    body = ['armor=900;', 'armorStructural=15;'] + standard
    body += [part('Hit' + name, armor, material, selection) for name, armor, material, selection in zones]
    body += [f'armor{name}={armor};' for name, armor, _, _ in zones]
    body += ['weapons[]={testgun};']
    return 'class CfgVehicles {class LoBo_M1 {\n' + '\n'.join(body) + '\n};};'


def merkava_class(name, front, properties):
    body = []
    if front is not None:
        body = [part('HitHull', '0.80', 50, 'hull'),
                part('HitTurretFront', '1.25', 51, 'turetf'),
                part('HitFHull', front, 50, 'fhull'),
                part('HitTurretTop', '1', 51, 'turett'),
                part('HitTurretBack', '0.80', 51, 'turetb')]
    body += [f'{key}={value};' for key, value in zip(
        ['armorFHull', 'armorTurretTop', 'armorTurretBack', 'armorTurretFront'], properties)]
    return f'class {name} {{\n' + '\n'.join(body) + '\n};'


class SelectionUnionTests(unittest.TestCase):
    def setUp(self):
        self.sources = [('front', b'\x01\x80\x00\x00\x01'), ('back', b'\x00\x01\x01\x00\x00')]
        self.original = model(lod(1, self.sources), lod(5e15, self.sources), lod(7e15, self.sources))
        self.unions = {'ud_turret': ('front', 'back')}

    def test_union_is_unique_and_only_hitpoint_lod_changes(self):
        fixed = models.add_hitpoint_unions(self.original, self.unions)
        before, after = models._lods(self.original), models._lods(fixed)
        self.assertEqual(after[1].selections['ud_turret'], b'\x01\x80\x01\x00\x01')
        self.assertEqual(len(after[1].selections), len(before[1].selections) + 1)
        for i in [0, 2]:
            self.assertEqual(self.original[before[i].start:before[i].end], fixed[after[i].start:after[i].end])
        # Removing precisely the appended tag recovers every original byte.
        insertion = len(fixed) - len(self.original)
        self.assertEqual(fixed[:before[1].end_tag] + fixed[before[1].end_tag+insertion:], self.original)
        self.assertEqual(models.add_hitpoint_unions(fixed, self.unions), fixed)

    def test_preserves_verified_legacy_trailer(self):
        trailer = b'data\\\0' + bytes(range(26))
        fixed = models.add_hitpoint_unions(self.original + trailer, self.unions)
        self.assertEqual(fixed[-32:], trailer)

    def test_any_oxygen_default_path_trailer_is_preserved(self):
        # LoBo_MD500D_R.p3d ends with 'H:\Codemast' + NUL + 20 bytes, not 'data\'.
        trailer = b'H:\\Codemast\0' + bytes(range(1, 21))
        self.assertEqual(32, len(trailer))
        fixed = models.add_hitpoint_unions(self.original + trailer, self.unions)
        self.assertEqual(fixed[-32:], trailer)
        self.assertEqual(len(models._lods(self.original + trailer)), 3)

    def test_non_path_32_byte_trailers_are_still_rejected(self):
        for trailer in (bytes(32), b'\x01' * 32, b'path without a nul byte'.ljust(32, b'x'),
                        b'\x7fbad\0'.ljust(32, b'\0'), b'H:\\Codemast\0' + bytes(21)):
            with self.subTest(trailer=trailer[:12]), self.assertRaisesRegex(ValueError, 'trailing MLOD'):
                models._lods(self.original + trailer)

    def test_pinned_trailer_must_match_exactly(self):
        # The Hilux models end with an empty Oxygen path: a NUL first, then stale memory.
        pin = bytes(12) + bytes(range(1, 21))
        self.assertEqual(32, len(pin))
        self.assertEqual(3, len(models._lods(self.original + pin, pin)))
        self.assertEqual(3, len(models._lods(self.original, pin)))
        other = pin[:-1] + b'\x7f'
        for tail, trailer in ((other, pin), (pin, other), (pin, None), (bytes(32), pin)):
            with self.subTest(tail=tail.hex(), trailer=trailer), self.assertRaisesRegex(ValueError, 'trailing MLOD'):
                models._lods(self.original + tail, trailer)
        # A printable path trailer still passes beside a pin, and the default is unchanged.
        path = b'data\\\0' + bytes(range(26))
        self.assertEqual(3, len(models._lods(self.original + path, pin)))
        self.assertEqual(3, len(models._lods(self.original + path)))
        for bad in (pin[:31], pin + b'\0', 'not bytes'):
            with self.subTest(pin=bad), self.assertRaisesRegex(ValueError, 'exactly 32 bytes'):
                models._lods(self.original, bad)

    def test_existing_hull_name_and_all_other_bytes_survive_union(self):
        original = model(lod(5e15, [('Hull', self.sources[0][1]), ('FHull', self.sources[1][1])]))
        fixed = models.add_hitpoint_unions(original, {'hull': ('Hull', 'FHull')})
        old, new = models._lods(original)[0], models._lods(fixed)[0]
        self.assertEqual(list(old.selections), list(new.selections))
        self.assertEqual(new.selections['hull'], b'\x01\x80\x01\x00\x01')
        offset = old.payload_offsets['hull']
        self.assertEqual(original[:offset], fixed[:offset])
        self.assertEqual(original[offset+5:], fixed[offset+5:])
        self.assertEqual(models.add_hitpoint_unions(fixed, {'hull': ('Hull', 'FHull')}), fixed)

    def test_missing_empty_duplicate_and_conflicting_selections_fail(self):
        for sources in [self.sources[:1], [('front', b'\0'*5), self.sources[1]],
                        self.sources + [('Front', self.sources[0][1])],
                        self.sources + [('ud_turret', b'\1'*5)]]:
            with self.subTest(sources=sources), self.assertRaises(ValueError):
                models.add_hitpoint_unions(model(lod(5e15, sources)), self.unions)

    def test_wrong_lod_cannot_satisfy_selection_reference(self):
        source = model(lod(1, self.sources), lod(5e15, self.sources[:1]))
        with self.assertRaises(ValueError):
            models.add_hitpoint_unions(source, self.unions)

    def test_malformed_inputs_fail_closed(self):
        cases = [b'ODOL' + self.original[4:], self.original[:25], self.original[:-1],
                 self.original + b'unknown trailer', model(lod(1, self.sources)),
                 model(lod(5e15, self.sources), lod(5e15, self.sources))]
        # A hostile count must not drive allocations or skip beyond input.
        hostile = bytearray(self.original)
        struct.pack_into('<i', hostile, 24, 2_000_000_000)
        cases.append(bytes(hostile))
        for data in cases:
            with self.subTest(size=len(data)), self.assertRaises(ValueError):
                models.add_hitpoint_unions(data, self.unions)

    def test_does_not_touch_unlisted_model_members(self):
        self.assertEqual(models.repair_models({'other/model.p3d': self.original}), {})


class ConfigTests(unittest.TestCase):
    def test_diver_roles_identifiers_and_loadouts_survive(self):
        source = r'''class CfgVehicles {
            class CoC_Diverdes {model="\seals\KMMwood.p3d";displayName="CQB";weapons[]={CQB};};
            class CoC_DiverSaboteurj {model="\seals\KMMwood1.p3d";displayName="Jungle Demolition";magazines[]={pipebomb};};
        };'''
        key = 'coc_diver/config.cpp'
        fixed = models.repair({key: source})[key]
        for name in ['CoC_Diverdes', 'CoC_DiverSaboteurj']:
            self.assertEqual(scalar(fixed, ('CfgVehicles', name), 'model'), r'"\CoC_Diver\CoC_Diver.p3d"')
        self.assertIn('displayName="Jungle Demolition";magazines[]={pipebomb}', fixed)
        self.assertIn('displayName="CQB";weapons[]={CQB}', fixed)
        self.assertEqual(models.repair({key: fixed}), {})
        with self.assertRaises(ValueError):
            models.repair({key: source.replace('KMMwood.p3d', 'unexpected.p3d')})

    def test_m1_uses_six_registered_counters_and_verified_bindings(self):
        key = 'lobo_m1/config.cpp'
        source = m1_config()
        fixed = models.repair({key: source})[key]
        a, b = class_block(fixed, ('CfgVehicles', 'LoBo_M1'))
        self.assertEqual({n.lower() for n, _, _ in class_spans(fixed[a:b])}, models._SUPPORTED)
        for component, expected in [('HitHull', 'hull'), ('HitTurret', 'turet'),
                                    ('HitLTrack', 'pasL'), ('HitRTrack', 'pasP')]:
            self.assertEqual(scalar(fixed, ('CfgVehicles', 'LoBo_M1', component), 'name'), expected)
        for preserved in ['armor=900;', 'armorStructural=15;', 'weapons[]={testgun};', 'armor=.6;', 'armor=.8;']:
            self.assertIn(preserved, fixed)
        self.assertNotIn('armorFHull', fixed)
        self.assertEqual(models.repair({key: fixed}), {})

    def test_unknown_authoring_and_mixed_state_fail(self):
        key = 'lobo_m1/config.cpp'
        source = m1_config()
        cases = [source.replace('armorFHull=1.6;', 'armorFHull=3;'),
                 source.replace('name=Fhull;', 'name=other;'),
                 source.replace('class HitFHull', 'class OtherHit'),
                 source.replace('class HitFHull', 'class HitFHullExtra'),
                 source.replace('armor=1.6;', 'armor=1.6; customProtection=1;')]
        for text in cases:
            with self.subTest(source=text[:50]), self.assertRaises(ValueError):
                models.repair({key: text})

    def test_merkava_inheritance_uses_each_models_actual_selections(self):
        docs = {
            'lobomerkava2/config.cpp': 'class CfgVehicles {' +
                merkava_class('LoBoMerkava1', '1.0', ['1.0', '0.7', '0.75', '1.0']) +
                merkava_class('LoBoMerkava2', '1.25', ['1.5', '0.7', '0.75', '1.5']) +
                merkava_class('LoBoMerkava2C', None, ['1.5', '2', '0.75', '1.5']) + '};',
            'lobomerkava4/config.cpp': 'class CfgVehicles {' +
                merkava_class('LoBoMerkava4DT', '1.25', ['1.5', '1.5', '1.5', '1.5']) + '};',
        }
        fixed = models.repair(docs)
        for key, name, selection in [('lobomerkava2/config.cpp', 'LoBoMerkava1', 'turet'),
                                      ('lobomerkava2/config.cpp', 'LoBoMerkava2', 'turet'),
                                      ('lobomerkava4/config.cpp', 'LoBoMerkava4DT', 'turet')]:
            path = ('CfgVehicles', name, 'HitTurret')
            self.assertEqual(scalar(fixed[key], path, 'name'), selection)
            self.assertEqual(scalar(fixed[key], path, 'armor'), '1')
            self.assertNotIn('class HitTurretFront', fixed[key])
            self.assertNotIn('armorTurretFront=', fixed[key])
        self.assertEqual(models.repair(fixed), {})


if __name__ == '__main__':
    unittest.main()
