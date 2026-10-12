"""Synthetic fixtures for the 2026-10-11.4 fourth-part repoints (repair.repoints).

No LoBo bytes are used: configs and MLOD models are generated here, shaped like
each real rule (same names, counts, LODs and pinned trailers).
"""
import hashlib
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "lobo"))
from repair import models, repoints, textures  # noqa: E402
import repair_lobo as runner  # noqa: E402

BS = chr(92)
FAB, RADIOS, OB = repoints.FAB, repoints.RADIOS, repoints.OB


def fab_config(state="before", newline="\r\n"):
    picture = "SN" + BS + "Snunit.paa" if state == "after" else "Snunit.paa"
    lines = ["class CfgVehicles", "{", "\tclass loBo_Snunit: BoatW", "\t{",
             '\t\tpicture="' + BS + "LoBo_FAB" + BS + picture + '";', '\t\ticon="ship";', "\t};",
             "\tclass LoBo_IDF_Zodiac: loBo_Snunit", "\t{",
             "\t\tpicture=" + BS + "LoBo_FAB" + BS + "zod" + BS + "icon.paa;", "\t};", "};", ""]
    return newline.join(lines)


def radios_config(state="before", newline="\r\n"):
    # "satcom": only the picture line already repaired (a partial state).
    folder = "icons" if state == "after" else "icon"
    satcom = "icons" if state in ("after", "satcom") else "icon"
    lines = ["class CfgWeapons", "{", "\tclass BAS_Satcom: LAWLauncher", "\t{",
             '\t\tpicture = "' + BS + "LoBoRadios" + BS + satcom + BS + 'BAS_satcom.paa"',
             '\t\tuiPicture = "' + BS + "LoBoRadios" + BS + folder + BS + 'BASirto.paa";', "\t};",
             "\tclass sebprc25: BAS_Satcom", "\t{",
             '\t\tuiPicture="' + BS + "LoBoRadios" + BS + "icons" + BS + 'sebirto.paa";', "\t};", "};", ""]
    return newline.join(lines)


def ob_config(state="before", newline="\n"):
    icon = BS + "Baracken" + BS + "b4.jpg" if state == "after" else BS + "LoBo_Ob" + BS + "bar" + BS + "b4.jpg"
    lines = ["class CfgVehicles", "{", "\tclass LoBo_B_Medic_MDA : Fortress1", "\t{",
             '\t\tmodel="' + BS + "LoBo_Ob" + BS + "Hospitals" + BS + 'LoBo_B_medic_MDA";', "\t\tATTENDANT=1;",
             '\t\ticon="' + icon + '";', "\t};", "};", ""]
    return newline.join(lines)


def configs(state="before"):
    return {FAB: fab_config(state), RADIOS: radios_config(state), OB: ob_config(state)}


def field(name):
    return name[:32] + bytes(32 - len(name[:32]))


def lod(resolution, names, points=3):
    faces = b"".join(field(name) + bytes([7]) * 72 for name in names)
    return (struct.pack("<4s6i", b"SP3X", 28, 153, points, 0, len(names), 0) + bytes(range(16)) * points
            + faces + b"TAGG" + struct.pack("<64sI", b"#EndOfFile#", 0) + struct.pack("<f", resolution))


def model(*lods, trailer=b""):
    return struct.pack("<4sII", b"MLOD", 257, len(lods)) + b"".join(lods) + trailer


# Faces that already name the target in the authored model, per (member, old, LOD slot):
# the M1 Guard LOD 20 wheels and the F-4 Wild Weasel's other MER and dark grey faces.
AUTHORED = {
    (repoints.M1A1_R, b"LoBo_M1\\tex1\\kolo.pac", 1): 58,
    (repoints.M1A2_R, b"LoBo_M1\\tex1\\kolo.pac", 1): 58,
    (repoints.F4_WW, b"LoBo_F4\\idf\\mer.pac", 0): 226,
    (repoints.F4_WW, b"LoBo_F4\\idf\\dark_grey.pac", 0): 263,
}


def real_model(key, state="before"):
    """A synthetic MLOD shaped like a real PLANS member, original or repaired."""
    resolutions, mapping, trailer = repoints.PLANS[key]
    visual = [b"other.paa"] + [new for new, _, _ in mapping.values()]
    lods = [lod(0.0, visual)]
    for slot, resolution in enumerate(resolutions):
        faces = [b"keep.paa"]
        for old, (new, expected, _) in mapping.items():
            count = expected if isinstance(expected, int) else expected[slot]
            kept = AUTHORED.get((key, old, slot), 0)
            faces += [new] * kept + [old if state == "before" else new] * (count - kept)
        lods.append(lod(resolution, faces))
    return model(*lods, lod(1e15, [b"x.paa"]), trailer=trailer or b"")


def names(data, trailer=None):
    return [[f.split(b"\0", 1)[0] for f in fields] for _, fields in textures.face_textures(data, trailer)]


def make_pbo(members):
    header = b"".join(name.encode("latin1") + b"\0" + struct.pack("<5I", 0, 0, 0, 0, len(data))
                      for name, data in members.items())
    body = header + bytes(21) + b"".join(members.values())
    return body + b"\0" + hashlib.sha1(body).digest()


class TextRuleTests(unittest.TestCase):
    def test_each_config_line_is_repointed_once_and_reaches_a_fixed_point(self):
        before, after = configs(), configs("after")
        self.assertEqual(after, repoints.repair(before))
        self.assertEqual({}, repoints.repair(after))
        for key in before:
            with self.subTest(key=key):
                self.assertEqual(before[key].count("\r\n"), after[key].count("\r\n"))
                self.assertEqual(before[key].count("\n"), after[key].count("\n"))
        # Only the four paths change: Snunit gains SN\, both radio paths gain s, b4.jpg moves.
        self.assertEqual(len(before[FAB]) + 3, len(after[FAB]))
        self.assertEqual(len(before[RADIOS]) + 2, len(after[RADIOS]))
        self.assertIn('icon="' + BS + "Baracken" + BS + 'b4.jpg";', after[OB])

    def test_partial_state_completes(self):
        partial = configs()
        partial[RADIOS] = radios_config("satcom")
        self.assertNotEqual(partial[RADIOS], radios_config())
        self.assertEqual(radios_config("after"), repoints.repair(partial)[RADIOS])

    def test_unexpected_sources_fail_closed(self):
        bad = {
            "missing line": fab_config().replace("Snunit.paa", "Other.paa"),
            "duplicate line": fab_config().replace('\t\ticon="ship";', fab_config().split("\r\n")[4]),
            "original next to repaired": fab_config() + fab_config("after"),
        }
        for message, text in bad.items():
            with self.subTest(message=message), self.assertRaises(ValueError):
                repoints.repair({**configs(), FAB: text})
        with self.assertRaisesRegex(ValueError, "Missing members"):
            repoints.repair({FAB: fab_config(), RADIOS: radios_config()})
        twice = radios_config().replace("\t};\r\n\tclass sebprc25", "\t\tuiPicture = \"" + BS + "LoBoRadios" + BS
                                        + "icon" + BS + "BASirto.paa\";\r\n\t};\r\n\tclass sebprc25")
        with self.assertRaises(ValueError):
            repoints.repair({**configs(), RADIOS: twice})


class ModelPlanTests(unittest.TestCase):
    def test_real_plans_repair_synthetic_models_to_a_fixed_point(self):
        before = {key: real_model(key) for key in repoints.MODEL_MEMBERS}
        after = {key: real_model(key, "after") for key in repoints.MODEL_MEMBERS}
        self.assertEqual(after, repoints.repair_models(before))
        self.assertEqual({}, repoints.repair_models(after))
        for key in repoints.MODEL_MEMBERS:
            trailer = repoints.PLANS[key][2]
            with self.subTest(key=key):
                self.assertEqual(len(before[key]), len(after[key]))
                with self.assertRaisesRegex(ValueError, "Missing model"):
                    repoints.repair_models({k: v for k, v in before.items() if k != key})
                with self.assertRaisesRegex(ValueError, key):
                    repoints.repair_models({**before, key: before[key][:len(before[key]) - len(trailer or b"")]
                                            + bytes(32)})
                # Only texture fields change.
                stripped, original = bytearray(after[key]), bytearray(before[key])
                for data in (stripped, original):
                    for lod_ in models._lods(bytes(data), trailer):
                        start, faces = textures._face_table(bytes(data), lod_)
                        for i in range(faces):
                            data[start + i * 104:start + i * 104 + 32] = bytes(32)
                self.assertEqual(original, stripped)

    def test_authored_target_faces_are_counted_exactly(self):
        # M1 Guard LOD 20 holds 58 authored kolo.paa wheel faces and 20
        # main_side.paa faces; LOD 15 84 kolo.pac and 117 main_side.paa.
        kolo_pac, kolo_paa = b"LoBo_M1\\tex1\\kolo.pac", b"LoBo_M1\\tex1\\kolo.paa"
        side_paa, side_pac = b"LoBo_M1\\tex1\\main_side.paa", b"LoBo_M1\\tex1\\main_side.pac"

        def guard(lod20):
            return model(lod(0.0, [kolo_paa, side_pac]), lod(15.0, [kolo_pac] * 84 + [side_paa] * 117),
                         lod(20.0, lod20 + [side_paa] * 20), lod(1e15, [b"x.paa"]))
        others = {k: real_model(k) for k in repoints.MODEL_MEMBERS}
        good = guard([kolo_paa] * 58)
        fixed = repoints.repair_models({**others, repoints.M1A1_R: good})[repoints.M1A1_R]
        self.assertEqual([[kolo_paa, side_pac], [kolo_paa] * 84 + [side_pac] * 117,
                          [kolo_paa] * 58 + [side_pac] * 20, [b"x.paa"]], names(fixed))
        for label, lod20 in (("57", [kolo_paa] * 57), ("59", [kolo_paa] * 59),
                             ("stray kolo.pac", [kolo_paa] * 58 + [kolo_pac])):
            with self.subTest(label=label), self.assertRaisesRegex(ValueError, "expected"):
                repoints.repair_models({**others, repoints.M1A1_R: guard(lod20)})

    def test_old_name_outside_the_listed_lods_fails(self):
        key = repoints.F4_WW
        source = real_model(key)
        bad = source.replace(field(b"x.paa"), field(b"LoBo_F4\\idf\\mer.pac"))
        with self.assertRaisesRegex(ValueError, "outside the listed LODs"):
            repoints.repair_models({**{k: real_model(k) for k in repoints.MODEL_MEMBERS}, key: bad})

    def test_f5_trailers_are_pinned_per_member(self):
        pins = {key: repoints.PLANS[key][2] for key in (repoints.F5_AA, repoints.F5_AT, repoints.F5_MIR)}
        self.assertEqual(3, len(set(pins.values())))
        swapped = {key: real_model(key) for key in repoints.MODEL_MEMBERS}
        swapped[repoints.F5_AA] = swapped[repoints.F5_AA][:-32] + pins[repoints.F5_AT]
        with self.assertRaisesRegex(ValueError, repoints.F5_AA + ": Unexpected trailing MLOD"):
            repoints.repair_models(swapped)

    def test_plans_are_well_formed(self):
        self.assertEqual(set(repoints.MODEL_MEMBERS), set(repoints.PLANS))
        targets = set()
        for key, (resolutions, mapping, trailer) in repoints.PLANS.items():
            with self.subTest(key=key):
                self.assertTrue(trailer is None or len(trailer) == 32)
                for old, (new, count, raw32) in mapping.items():
                    self.assertLessEqual(len(new), 31)
                    self.assertFalse(raw32)
                    self.assertTrue(isinstance(count, int) or len(count) == len(resolutions))
                    targets.add(new.decode("ascii").lower().replace(BS, "/"))
        self.assertEqual(5, len(targets))
        self.assertLessEqual(targets, set(repoints.REQUIRED_MEMBERS))
        for keys in (repoints.REQUIRED_MEMBERS, repoints.PLANS, repoints.RULES):
            self.assertLessEqual({key.split("/", 1)[0] for key in keys}, set(repoints.ADDONS))
        self.assertEqual(tuple(sorted(repoints.ADDONS)), repoints.ADDONS)


class PlanTests(unittest.TestCase):
    STORED = {"baracken": "Baracken", "lobo_f4": "LoBo_F4", "lobo_fab": "LoBo_FAB", "lobo_kfir": "LoBo_Kfir",
              "lobo_m1": "LoBo_M1", "lobo_ob": "LoBo_Ob", "lobof5": "LoboF5", "loboradios": "LoBoRadios"}

    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="lobo-repoints-test-")
        self.addCleanup(self.temporary.cleanup)
        self.mod = Path(self.temporary.name)
        (self.mod / "addons").mkdir()
        self.members = {stem: {} for stem in repoints.ADDONS}
        for key in repoints.MODEL_MEMBERS:
            stem, name = key.split("/", 1)
            self.members[stem][name] = real_model(key)
        for key in repoints.REQUIRED_MEMBERS:
            stem, name = key.split("/", 1)
            self.members[stem][name.replace("/", BS)] = b"shipped " + name.encode()
        for key, text in configs().items():
            stem, name = key.split("/", 1)
            self.members[stem][name] = text.encode("latin1")
        for stem in self.members:
            self.write(stem, self.members[stem])

    def write(self, stem, entries):
        (self.mod / "addons" / (self.STORED[stem] + ".pbo")).write_bytes(make_pbo(entries))

    def test_plan_end_to_end_and_required_members_fail_closed(self):
        with patch.object(runner, "modules", return_value=[repoints]):
            changed = runner.plan(self.mod, None)[2]
            expected = {key: real_model(key, "after") for key in repoints.MODEL_MEMBERS}
            expected.update({key: text.encode("latin1") for key, text in configs("after").items()})
            self.assertEqual(expected, changed)
            for key in repoints.REQUIRED_MEMBERS:
                stem, name = key.split("/", 1)
                with self.subTest(missing=key):
                    self.write(stem, {k: v for k, v in self.members[stem].items() if k != name.replace("/", BS)})
                    with self.assertRaisesRegex(ValueError, "Required members missing"):
                        runner.plan(self.mod, None)
                    self.write(stem, self.members[stem])
            (self.mod / "addons" / "Baracken.pbo").unlink()
            with self.assertRaisesRegex(ValueError, "Required addons missing"):
                runner.plan(self.mod, None)


if __name__ == "__main__":
    unittest.main()
