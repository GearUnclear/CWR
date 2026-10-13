"""Synthetic fixtures for the 2026-10-11.4 missing-reference repairs (LoBo issue 14)."""
import hashlib
from pathlib import Path
import struct
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "lobo"))
from repair import models, references, textures  # noqa: E402
import repair_lobo as runner  # noqa: E402

BS = chr(92)
MIG = "lobomig/config.cpp"
PILOT = "lobo_pilot/config.cpp"
CRASH_OLD = '\t\tsoundLandCrash[]={"\\LoBoMig\\soundss\\jet_crash.wss",0.006000,1};'
CRASH_NEW = '\t\tsoundLandCrash[]={"\\LoBoMig\\sounds\\jet_crash.wss",0.006000,1};'
CRASH_ABOVE = '\t\tsoundCrash[]={"\\LoBoMig\\sounds\\jet_crash.wss",0.006000,1};'


def mig_config(*crash_lines, newline="\r\n"):
    lines = ["class CfgVehicles", "{", "\tclass LoBo_E_mig21bis_MR1", "\t{", CRASH_ABOVE, *crash_lines,
             "\t};", "};", ""]
    return newline.join(lines)


def pair(folder, name, healthy_ext):
    return f'"\\LoBo_Pilot\\{folder}\\{name}.{healthy_ext}", "\\LoBo_Pilot\\{folder}\\{name}_d.paa"'


# (folder, name, healthy extension before, after): the five repointed pairs and
# the three IAF pairs whose .pac ships.
WOUNDS = (("EAF\\tex", "back", "pac", "paa"), ("EAF\\tex", "legsbk", "pac", "paa"),
          ("IAF\\tex2", "legsbk", "pac", "paa"), ("RJAF\\tex", "back", "pac", "paa"),
          ("RJAF\\tex", "legsbk", "pac", "paa"),
          ("IAF\\tex", "back", "pac", "pac"), ("IAF\\tex", "legsbk", "pac", "pac"),
          ("IAF\\tex2", "back", "pac", "pac"))


def pilot_config(state="before", extra=""):
    def wounds(rows):
        return "wounds[]={" + ", ".join(pair(f, n, b if state == "before" else a) for f, n, b, a in rows) + "};"
    lines = ["class CfgVehicles", "{", "\tclass LoBo_Egypt_Pilot", "\t{",
             "\t\t" + wounds(WOUNDS[0:2]), "\t};", "\tclass LoBo_IAF_Pilot", "\t{",
             "\t\t" + wounds(WOUNDS[5:7]), "\t};", "\tclass LoBo_IAF_Pilot_grey", "\t{",
             "\t\t" + wounds((WOUNDS[7], WOUNDS[2])), "\t};", "\tclass LoBo_Jordan_Pilot", "\t{",
             "\t\t" + wounds(WOUNDS[3:5]) + extra, "\t};", "};", ""]
    return "\r\n".join(lines)


def field(name):
    return name[:32] + bytes(32 - len(name[:32]))


def lod(resolution, names, points=3):
    faces = b"".join(field(name) + bytes([7]) * 72 for name in names)
    return (struct.pack("<4s6i", b"SP3X", 28, 153, points, 0, len(names), 0) + bytes(range(16)) * points
            + faces + b"TAGG" + struct.pack("<64sI", b"#EndOfFile#", 0) + struct.pack("<f", resolution))


def model(*lods, trailer=b""):
    return struct.pack("<4sII", b"MLOD", 257, len(lods)) + b"".join(lods) + trailer


def names(data, trailer=None):
    return [[f.split(b"\0", 1)[0] for f in fields] for _, fields in textures.face_textures(data, trailer)]


OLD = b"Pack\\t\\old_name.pac"
NEW = b"Pack\\t\\old name.pac"
OTHER = b"Pack\\t\\other.pac"
OTHER_NEW = b"Pack\\t\\other.paa"
PIN = bytes.fromhex("000000000000000000000000e03ea1000d000000f8fb1e0100b5a000b4f3c600")


def real_model(key, state="before"):
    """A synthetic MLOD shaped like each real PLANS member, original or repaired."""
    resolutions, mapping, trailer = references.PLANS[key]
    visual = []
    for old, (new, _, _) in mapping.items():
        visual += [new, b"other.paa"]
    lods = [lod(0.0 if key != references.AIM9 else 1.0, visual)]
    for slot, resolution in enumerate(resolutions):
        faces = [b"keep.paa"]
        for old, (new, expected, _) in mapping.items():
            count = expected if isinstance(expected, int) else expected[slot]
            faces += [old if state == "before" else new] * count
        lods.append(lod(resolution, faces))
    return model(*lods, lod(1e15, [b"x.paa"]), trailer=trailer or b"")


def make_pbo(members):
    header = b"".join(name.encode("latin1") + b"\0" + struct.pack("<5I", 0, 0, 0, 0, len(data))
                      for name, data in members.items())
    body = header + bytes(21) + b"".join(members.values())
    return body + b"\0" + hashlib.sha1(body).digest()


class TextRuleTests(unittest.TestCase):
    def test_mig_crash_sound_line(self):
        changed = references.repair({MIG: mig_config(CRASH_OLD), PILOT: pilot_config()})
        self.assertEqual(mig_config(CRASH_NEW), changed[MIG])
        self.assertEqual(len(mig_config(CRASH_OLD)) - 1, len(changed[MIG]))
        self.assertEqual(changed[MIG].count("\n"), changed[MIG].count("\r\n"))
        self.assertNotIn(MIG, references.repair({MIG: changed[MIG], PILOT: pilot_config()}))
        self.assertEqual({}, references.repair({MIG: mig_config(CRASH_NEW), PILOT: pilot_config("after")}))
        for lines in ((), (CRASH_OLD, CRASH_OLD), (CRASH_OLD, CRASH_NEW), (CRASH_OLD.replace("\t\t", "\t"),)):
            with self.subTest(lines=lines), self.assertRaises(ValueError):
                references.repair({MIG: mig_config(*lines), PILOT: pilot_config()})

    def test_five_wound_pairs_change_and_the_shipped_pac_pairs_stay(self):
        before, after = pilot_config(), pilot_config("after")
        changed = references.repair({MIG: mig_config(CRASH_NEW), PILOT: before})
        self.assertEqual({PILOT: after}, changed)
        self.assertEqual(len(before), len(after))
        self.assertEqual(5, sum(1 for x, y in zip(before, after) if x != y))
        for folder, name, _, ext in WOUNDS[5:]:
            self.assertEqual("pac", ext)
            self.assertIn(pair(folder, name, "pac"), after)
        for folder, name, _, _ in WOUNDS[:5]:
            self.assertNotIn(pair(folder, name, "pac"), after)
            self.assertEqual(1, after.count(pair(folder, name, "paa")))
        self.assertEqual({}, references.repair({MIG: mig_config(CRASH_NEW), PILOT: after}))

    def test_partial_wound_state_completes_and_duplicates_are_rejected(self):
        partial = pilot_config().replace(pair("EAF\\tex", "back", "pac"), pair("EAF\\tex", "back", "paa"))
        self.assertEqual(pilot_config("after"), references.repair({MIG: mig_config(CRASH_NEW), PILOT: partial})[PILOT])
        for extra in (", " + pair("RJAF\\tex", "legsbk", "pac"), ", " + pair("RJAF\\tex", "legsbk", "paa")):
            with self.subTest(extra=extra), self.assertRaises(ValueError):
                references.repair({MIG: mig_config(CRASH_NEW), PILOT: pilot_config(extra=extra)})
        missing = pilot_config().replace(pair("IAF\\tex2", "legsbk", "pac"), pair("IAF\\tex2", "legs", "pac"))
        with self.assertRaises(ValueError):
            references.repair({MIG: mig_config(CRASH_NEW), PILOT: missing})
        with self.assertRaisesRegex(ValueError, "Missing members"):
            references.repair({MIG: mig_config(CRASH_OLD)})

    def test_pilot_rules_count_inside_their_class(self):
        # repair.wounds copies the repaired IAF tex2 legsbk pair (on its _dr)
        # into grey2 and grey3; an unrepaired .pac pair elsewhere still fails.
        def with_copies(text, copy):
            classes = "".join(f"\tclass LoBo_IAF_Pilot_grey{n}: LoBo_IAF_Pilot_grey\r\n\t{{\r\n"
                              f"\t\twounds[]={{{copy}}};\r\n\t}};\r\n" for n in (2, 3))
            return text.replace("\tclass LoBo_Jordan_Pilot", classes + "\tclass LoBo_Jordan_Pilot")
        copy = pair("IAF\\tex2", "legsbk", "paa").replace("_d.paa", "_dr.paa")
        before, after = with_copies(pilot_config(), copy), with_copies(pilot_config("after"), copy)
        self.assertEqual(after, references.repair({MIG: mig_config(CRASH_NEW), PILOT: before})[PILOT])
        self.assertEqual({}, references.repair({MIG: mig_config(CRASH_NEW), PILOT: after}))
        stray = with_copies(pilot_config("after"), pair("IAF\\tex2", "legsbk", "pac"))
        with self.assertRaisesRegex(ValueError, "outside its class"):
            references.repair({MIG: mig_config(CRASH_NEW), PILOT: stray})
        self.assertEqual(tuple(rule for rules in references.PILOT_CLASSES.values() for rule in rules),
                         references.RULES[PILOT])


class ModelRepointTests(unittest.TestCase):
    def setUp(self):
        self.original = model(lod(0.0, [NEW, OTHER_NEW]),
                              lod(1.0, [OLD, OLD, OTHER, b"keep.paa", OLD]),
                              lod(5.0, [OLD, b"keep.paa"]),
                              lod(1e15, [b"x.paa"]))
        self.mapping = {OLD: (NEW, (3, 1), False), OTHER: (OTHER_NEW, (1, 0), False)}

    def test_tuple_counts_are_exact_per_lod_and_only_fields_change(self):
        fixed = textures.repoint(self.original, (1.0, 5.0), self.mapping)
        self.assertEqual(len(self.original), len(fixed))
        self.assertEqual([[NEW, OTHER_NEW], [NEW, NEW, OTHER_NEW, b"keep.paa", NEW], [NEW, b"keep.paa"], [b"x.paa"]],
                         names(fixed))
        self.assertEqual(fixed, textures.repoint(fixed, (1.0, 5.0), self.mapping))
        stripped, original = bytearray(fixed), bytearray(self.original)
        for data in (stripped, original):
            for lod_ in models._lods(bytes(data)):
                start, faces = textures._face_table(bytes(data), lod_)
                for i in range(faces):
                    data[start + i * 104:start + i * 104 + 32] = bytes(32)
        self.assertEqual(original, stripped)
        # Only one byte per repointed face differs ('_' to ' ', 'c' to 'a').
        self.assertEqual(5, sum(1 for x, y in zip(self.original, fixed) if x != y))
        for counts in ((2, 1), (3, 2), (4, 1)):
            with self.subTest(counts=counts), self.assertRaisesRegex(ValueError, "expected"):
                textures.repoint(self.original, (1.0, 5.0), {OLD: (NEW, counts, False)})

    def test_zero_entry_rejects_a_stray_original_or_repaired_face(self):
        for stray in (OTHER, OTHER_NEW):
            source = model(lod(1.0, [OLD, OTHER]), lod(5.0, [OLD, stray]))
            with self.subTest(stray=stray), self.assertRaisesRegex(ValueError, "expected 0"):
                textures.repoint(source, (1.0, 5.0), {OLD: (NEW, (1, 1), False), OTHER: (OTHER_NEW, (1, 0), False)})

    def test_invalid_count_tuples_raise(self):
        for counts in ((3,), (3, 1, 0), (3, -1), (0, 0), (3, 1.0), (3, True), [3, 1]):
            with self.subTest(counts=counts), self.assertRaisesRegex(ValueError, "counts"):
                textures.repoint(self.original, (1.0, 5.0), {OLD: (NEW, counts, False)})

    def test_listed_resolution_must_match_exactly_one_lod(self):
        with self.assertRaisesRegex(ValueError, "matches 0 LODs"):
            textures.repoint(self.original, (1.0, 5.0, 2.0), {OLD: (NEW, (3, 1, 0), False)})
        with self.assertRaisesRegex(ValueError, "matches 0 LODs"):
            textures.repoint(self.original, (1.0, 5.0, 2.0), {OLD: (NEW, 3, False)})
        twice = model(lod(1.0, [OLD]), lod(1.0, [OLD]))
        with self.assertRaisesRegex(ValueError, "matches 2 LODs"):
            textures.repoint(twice, (1.0,), {OLD: (NEW, 1, False)})
        with self.assertRaisesRegex(ValueError, "more than one listed"):
            textures.repoint(self.original, (1.0, 1.0, 5.0), {OLD: (NEW, (3, 3, 1), False)})

    def test_pinned_trailer_is_exact_and_preserved(self):
        source = self.original + PIN
        fixed = textures.repoint(source, (1.0, 5.0), self.mapping, PIN)
        self.assertEqual(PIN, fixed[-32:])
        self.assertEqual(fixed, textures.repoint(fixed, (1.0, 5.0), self.mapping, PIN))
        self.assertEqual(4, len(textures.face_textures(fixed, PIN)))
        other = bytearray(PIN)
        other[13] ^= 1
        for tail, trailer in ((bytes(other), PIN), (PIN, bytes(other)), (PIN, None), (bytes(32), PIN)):
            with self.subTest(tail=tail.hex(), trailer=trailer), self.assertRaisesRegex(ValueError, "trailing MLOD"):
                textures.repoint(self.original + tail, (1.0, 5.0), self.mapping, trailer)
        with self.assertRaisesRegex(ValueError, "trailing MLOD"):
            textures.face_textures(source)
        # A printable Oxygen path trailer is still accepted beside a pin; no tail is too.
        codemast = b"H:" + BS.encode() + b"Codemast\0" + bytes(range(1, 21))
        self.assertEqual(codemast, textures.repoint(self.original + codemast, (1.0, 5.0), self.mapping, PIN)[-32:])
        self.assertEqual(len(self.original), len(textures.repoint(self.original, (1.0, 5.0), self.mapping, PIN)))


class PlanTests(unittest.TestCase):
    def test_real_plans_repair_synthetic_models_to_a_fixed_point(self):
        before = {key: real_model(key) for key in references.MODEL_MEMBERS}
        after = {key: real_model(key, "after") for key in references.MODEL_MEMBERS}
        self.assertEqual(after, references.repair_models(before))
        self.assertEqual({}, references.repair_models(after))
        for key in references.MODEL_MEMBERS:
            with self.subTest(key=key):
                with self.assertRaisesRegex(ValueError, "Missing model"):
                    references.repair_models({k: v for k, v in before.items() if k != key})
                with self.assertRaisesRegex(ValueError, key):
                    references.repair_models({**before, key: before[key] + b"\0" * 32})

    def test_hilux_trailers_are_pinned_per_member(self):
        hilux, hilux_mg = references.PLANS[references.HILUX][2], references.PLANS[references.HILUX_MG][2]
        swapped = {key: real_model(key) for key in references.MODEL_MEMBERS}
        swapped[references.HILUX] = swapped[references.HILUX][:-32] + hilux_mg
        self.assertNotEqual(hilux, hilux_mg)
        with self.assertRaisesRegex(ValueError, references.HILUX + ": Unexpected trailing MLOD"):
            references.repair_models(swapped)

    def test_real_plans_are_well_formed(self):
        self.assertEqual(set(references.MODEL_MEMBERS), set(references.PLANS))
        self.assertEqual(4, len(references.MODEL_MEMBERS))
        targets = set()
        for key, (resolutions, mapping, trailer) in references.PLANS.items():
            with self.subTest(key=key):
                self.assertTrue(trailer is None or len(trailer) == 32)
                for old, (new, count, raw32) in mapping.items():
                    self.assertLessEqual(len(new), 31)
                    self.assertFalse(raw32)
                    self.assertTrue(isinstance(count, int) or len(count) == len(resolutions))
                    targets.add(new.decode("ascii").lower().replace(BS, "/"))
        self.assertEqual(4, len(targets))
        self.assertLessEqual(targets, set(references.REQUIRED_MEMBERS))
        self.assertEqual(("lobo_airammo", "lobo_pilot", "lobomig", "loboter"), references.ADDONS)
        self.assertLessEqual({key.split("/", 1)[0] for key in references.REQUIRED_MEMBERS}, set(references.ADDONS))
        self.assertLessEqual({key.split("/", 1)[0] for key in references.PLANS}, set(references.ADDONS))
        self.assertLessEqual({key.split("/", 1)[0] for key in references.RULES}, set(references.ADDONS))


class RequiredMemberTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="lobo-references-test-")
        self.addCleanup(self.temporary.cleanup)
        self.mod = Path(self.temporary.name)
        (self.mod / "addons").mkdir()

    def write(self, stem, members):
        (self.mod / "addons" / (stem + ".pbo")).write_bytes(make_pbo(members))

    def module(self, required):
        return SimpleNamespace(ADDONS=("first",), REQUIRED_MEMBERS=required, repair=lambda documents: {},
                               __name__="synthetic-required")

    def test_missing_required_member_fails_plan(self):
        self.write("first", {"config.cpp": b"class CfgPatches {};"})
        with patch.object(runner, "modules", return_value=[self.module(("first/tex/needed.paa",))]):
            with self.assertRaisesRegex(ValueError, "Required members missing: \\['first/tex/needed.paa'\\]"):
                runner.plan(self.mod, None)
            self.write("first", {"config.cpp": b"class CfgPatches {};", "Tex\\Needed.paa": b"texture"})
            self.assertEqual({}, runner.plan(self.mod, None)[2])

    def test_required_member_outside_addons_or_not_normalized_fails(self):
        self.write("first", {"config.cpp": b"x", "tex\\needed.paa": b"t"})
        self.write("second", {"tex\\needed.paa": b"t"})
        for required, message in ((("second/tex/needed.paa",), "outside the repaired addons"),
                                  (("first/Tex/needed.paa",), "normalized"),
                                  (("first",), "normalized")):
            with self.subTest(required=required), \
                    patch.object(runner, "modules", return_value=[self.module(required)]), \
                    self.assertRaisesRegex(ValueError, message):
                runner.plan(self.mod, None)

    def test_references_plan_end_to_end_on_a_synthetic_mod(self):
        members = {stem: {} for stem in references.ADDONS}
        stored = {"lobomig": "LoBoMig", "loboter": "LoBoTer", "lobo_pilot": "LoBo_Pilot", "lobo_airammo": "LoBo_airammo"}
        for key in references.MODEL_MEMBERS:
            stem, name = key.split("/", 1)
            members[stem][name] = real_model(key)
        for key in references.REQUIRED_MEMBERS:
            stem, name = key.split("/", 1)
            members[stem][name.replace("/", BS)] = b"shipped " + name.encode()
        members["lobomig"]["config.cpp"] = mig_config(CRASH_OLD).encode("latin1")
        members["lobo_pilot"]["config.cpp"] = pilot_config().encode("latin1")
        for stem, entries in members.items():
            self.write(stored[stem], entries)
        with patch.object(runner, "modules", return_value=[references]):
            changed = runner.plan(self.mod, None)[2]
            expected = {key: real_model(key, "after") for key in references.MODEL_MEMBERS}
            expected[MIG] = mig_config(CRASH_NEW).encode("latin1")
            expected[PILOT] = pilot_config("after").encode("latin1")
            self.assertEqual(expected, changed)
            for key in references.REQUIRED_MEMBERS:
                stem, name = key.split("/", 1)
                with self.subTest(missing=key):
                    self.write(stored[stem], {k: v for k, v in members[stem].items() if k != name.replace("/", BS)})
                    with self.assertRaisesRegex(ValueError, "Required members missing"):
                        runner.plan(self.mod, None)
                    self.write(stored[stem], members[stem])


if __name__ == "__main__":
    unittest.main()
