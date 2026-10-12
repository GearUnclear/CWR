"""Synthetic regression tests for the LoBoWeapons references and the hash-pinned HK icon additions.

No HK pack bytes are used: the source archive and its pins are generated here.
"""
import hashlib
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "lobo"))
from repair import weapons  # noqa: E402
from repair.pbo import CPRS  # noqa: E402

BS = chr(92)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def make_pbo(entries):
    """Entries are (name bytes, stored bytes, packing); SHA-1 tail as in the HK pack."""
    header = [name + b"\0" + struct.pack("<5I", packing, len(data) * 2 if packing else 0, 0, 0, len(data))
              for name, data, packing in entries]
    body = b"".join(header) + bytes(21) + b"".join(data for _, data, _ in entries)
    return body + b"\0" + hashlib.sha1(body).digest()


def icon(key):
    return ("synthetic icon " + key).encode() * 3


def config_lf():
    lines = ["class CfgWeapons", "{"]
    for _, old, new, count, *baseline in weapons.RULES[weapons.CONFIG]:
        lines += [old] * count + [new] * (baseline[0] if baseline else 0)
    return "\n".join(lines + ["};", ""])


ICON_PRESENT = frozenset({weapons.MSG90})
MSG90_LOBO = '\t\tpicture="' + BS + "LoBoWeapons" + BS + "inventory" + BS + 'w_msg90.paa";'
MSG90_HK = '\t\tpicture="' + BS + "TheHKpack" + BS + "inventory" + BS + 'w_msg90.paa";'


class WeaponConfigTests(unittest.TestCase):
    def test_config_rules_exact_and_idempotent(self):
        source = config_lf().replace("\n", "\r\n")
        fixed = weapons.repair({weapons.CONFIG: source}, present=ICON_PRESENT)[weapons.CONFIG]
        want = config_lf()
        for _, old, new, *_ in weapons.RULES[weapons.CONFIG]:
            want = "\n".join(new if line == old else line for line in want.split("\n"))
        self.assertEqual(want.replace("\n", "\r\n"), fixed)
        self.assertEqual({}, weapons.repair({weapons.CONFIG: fixed}, present=ICON_PRESENT))
        self.assertEqual(2, fixed.count('model="' + BS + "LoBoWeapons" + BS + 'LoBo_M4A1_m203.p3d";'))
        self.assertIn(MSG90_LOBO + "\r\n", fixed)
        self.assertNotIn("TheHKpack", fixed)
        self.assertNotIn("Ksvk", fixed)
        self.assertNotIn(BS + "LoBoWeapons" + BS + "LoBoWeapons", fixed)

    def test_msg90_keeps_the_authored_bank_without_its_icon(self):
        # TheHKpack.pbo in @LoBo_Extras resolves the authored path; repointing
        # without the added icon would hide it.
        source = config_lf()
        fixed = weapons.repair({weapons.CONFIG: source})[weapons.CONFIG]
        self.assertIn(MSG90_HK + "\n", fixed)
        self.assertNotIn(MSG90_LOBO, fixed)
        self.assertEqual({}, weapons.repair({weapons.CONFIG: fixed}))
        # Every other rule still applies.
        self.assertNotIn("Ksvk", fixed)
        self.assertNotIn("glock18gear", fixed)
        # A repoint made without the icon is put back; with the icon it is kept.
        repointed = weapons.repair({weapons.CONFIG: source}, present=ICON_PRESENT)[weapons.CONFIG]
        self.assertEqual({weapons.CONFIG: fixed}, weapons.repair({weapons.CONFIG: repointed}))
        self.assertEqual({}, weapons.repair({weapons.CONFIG: repointed}, present=ICON_PRESENT))
        # The other icons do not decide the MSG90 line.
        others = frozenset(weapons.ICONS) - ICON_PRESENT
        self.assertEqual({}, weapons.repair({weapons.CONFIG: fixed}, present=others))

    def test_msg90_line_must_occur_once_in_either_spelling(self):
        for present in (frozenset(), ICON_PRESENT):
            for text in (config_lf().replace(MSG90_HK + "\n", ""),
                         config_lf().replace(MSG90_HK + "\n", MSG90_HK + "\n" + MSG90_LOBO + "\n")):
                with self.subTest(present=bool(present)), self.assertRaises(ValueError):
                    weapons.repair({weapons.CONFIG: text}, present=present)

    def test_m4a1_baseline_is_required(self):
        source = config_lf().replace('\t\tmodel="' + BS + "LoBoWeapons" + BS + 'LoBo_M4A1_m203.p3d";\n', "", 1)
        with self.assertRaises(ValueError):
            weapons.repair({weapons.CONFIG: source}, present=ICON_PRESENT)

    def test_missing_config_rejected(self):
        with self.assertRaises(ValueError):
            weapons.repair({})

    def test_pins_are_the_published_hk_175_values(self):
        self.assertEqual("79cdb0bb937e9e05d43781da0337af36f6d3cca93e4d09658e9847a4d70b3ff5", weapons.HK_PACK_SHA256)
        self.assertEqual(7, len(weapons.ICONS))
        self.assertEqual(weapons.ADDED_MEMBERS, tuple(weapons.ICONS))
        self.assertEqual("df3c75bbf03feac8ae6852664561aa351fb6e0941975e5f62ef0a816776bc507",
                         weapons.ICONS["loboweapons/inventory/w_msg90.paa"][2])


class IconAdditionTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory(prefix="lobo-weapons-test-")
        self.addCleanup(temporary.cleanup)
        self.dir = Path(temporary.name)
        self.icons = {key: icon(key) for key in weapons.ICONS}
        entries = [(b"config.cpp", b"class CfgPatches {};", 0)]
        entries += [(member.replace("/", BS).encode(), self.icons[key], 0)
                    for key, (_, member, _) in weapons.ICONS.items()]
        self.source = self.write("TheHKpack.pbo", make_pbo(entries))
        pins = {key: (stored, member, sha(self.icons[key])) for key, (stored, member, _) in weapons.ICONS.items()}
        for active in (patch.dict(weapons.ICONS, pins),
                       patch.object(weapons, "HK_PACK_SHA256", sha(self.source.read_bytes()))):
            active.start()
            self.addCleanup(active.stop)

    def write(self, name, data):
        path = self.dir / name
        path.write_bytes(data)
        return path

    def test_adds_all_missing_in_table_order(self):
        added, skipped = weapons.add_members({}, self.source)
        self.assertEqual([], skipped)
        self.assertEqual(list(weapons.ICONS), list(added))
        for key, (stored, data) in added.items():
            self.assertEqual(self.icons[key], data)
            self.assertEqual(weapons.ICONS[key][0], stored)
            self.assertTrue(stored.startswith("Inventory" + BS))
            self.assertEqual(key.split("/", 1)[1], stored.replace(BS, "/").lower())

    def test_adds_only_the_missing_ones_and_reaches_a_fixed_point(self):
        first = next(iter(weapons.ICONS))
        added, skipped = weapons.add_members({first: self.icons[first]}, self.source)
        self.assertNotIn(first, added)
        self.assertEqual(len(weapons.ICONS) - 1, len(added))
        present = {key: data for key, (_, data) in added.items()} | {first: self.icons[first]}
        self.assertEqual(({}, []), weapons.add_members(present, self.source))
        self.assertEqual(({}, []), weapons.add_members(present, None))

    def test_without_source_reports_one_skip_line(self):
        added, skipped = weapons.add_members({}, None)
        self.assertEqual({}, added)
        self.assertEqual(1, len(skipped))
        self.assertIn("--asset-source", skipped[0])
        self.assertIn("w_msg90.paa", skipped[0])
        self.assertIn("@LoBo_Extras", skipped[0])
        self.assertEqual((added, skipped), weapons.add_members({}, None))

    def test_wrong_source_raises_even_when_nothing_is_missing(self):
        key, (_, member, _) = next(iter(weapons.ICONS.items()))
        entries = [(m.replace("/", BS).encode(), self.icons[k], 0) for k, (_, m, _) in weapons.ICONS.items()]
        sources = {
            "not a pbo": b"synthetic source",
            "trailing garbage": self.source.read_bytes() + b"x",
            "member missing": make_pbo([e for e in entries if e[0] != member.replace("/", BS).encode()]),
            "member mismatch": make_pbo([(n, b"tampered" if n == member.replace("/", BS).encode() else d, c)
                                         for n, d, c in entries]),
        }
        for name, data in sources.items():
            wrong = self.write(name.replace(" ", "-") + ".pbo", data)
            for present in ({}, dict(self.icons)):
                with self.subTest(source=name, present=len(present)), self.assertRaisesRegex(ValueError, "asset-source"):
                    weapons.add_members(present, wrong)

    def test_any_archive_with_the_pinned_members_is_accepted(self):
        # The @LoBo_Extras copy differs from 1.75 only in a patched config.bin.
        entries = [(b"config.bin", b"patched rapified config", 0)]
        entries += [(member.replace("/", BS).encode(), self.icons[key], 0)
                    for key, (_, member, _) in weapons.ICONS.items()]
        extras = self.write("extras-TheHKpack.pbo", make_pbo(entries))
        self.assertNotEqual(sha(extras.read_bytes()), weapons.HK_PACK_SHA256)
        added, skipped = weapons.add_members({}, extras)
        self.assertEqual([], skipped)
        self.assertEqual({key: data for key, data in self.icons.items()}, {k: d for k, (_, d) in added.items()})

    def test_present_but_wrong_bytes_raises(self):
        first = next(iter(weapons.ICONS))
        with self.assertRaisesRegex(ValueError, "present but not the pinned"):
            weapons.add_members({first: b"different"}, None)
        with self.assertRaisesRegex(ValueError, "Unknown added"):
            weapons.add_members({"loboweapons/inventory/other.paa": b"x"}, None)

    def test_compressed_or_mismatching_source_member_raises(self):
        key, (stored, member, _) = next(iter(weapons.ICONS.items()))
        for mode in ("compressed", "mismatch", "absent"):
            entries = [(b"config.cpp", b"class CfgPatches {};", 0)]
            for other, (_, name, _) in weapons.ICONS.items():
                data, packing = self.icons[other], 0
                if other == key and mode == "compressed":
                    packing = CPRS
                elif other == key and mode == "mismatch":
                    data = b"tampered"
                elif other == key and mode == "absent":
                    continue
                entries.append((name.replace("/", BS).encode(), data, packing))
            source = self.write(mode + ".pbo", make_pbo(entries))
            with patch.object(weapons, "HK_PACK_SHA256", sha(source.read_bytes())):
                with self.subTest(mode=mode), self.assertRaises(ValueError):
                    weapons.add_members({}, source)


if __name__ == "__main__":
    unittest.main()
