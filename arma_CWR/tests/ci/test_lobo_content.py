"""Synthetic regression fixtures for AG-5, AG-6, AG-11 and the bwm_boot3 crew (CI-6)."""
from pathlib import Path
import sys
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "lobo"))
from repair import content  # noqa: E402

BS = chr(92)


def fixture_lf(path):
    lines = ["class CfgVehicles", "{"]
    for _, old, new, count in content.RULES[path]:
        for i in range(count):
            lines += [f"\tclass Thing{len(lines)}: Default", "\t{", old, "\t};"]
    return "\n".join(lines + ["};", ""])


def expected(path):
    text = fixture_lf(path)
    for _, old, new, _ in content.RULES[path]:
        text = "\n".join(new if line == old else line for line in text.split("\n"))
    return text.replace("\n", "\r\n")


def fixture():
    return {path: fixture_lf(path).replace("\n", "\r\n") for path in content.RULES}


class ContentRepairTests(unittest.TestCase):
    def test_each_rule_exact_and_idempotent(self):
        docs = fixture()
        changed = content.repair(docs)
        self.assertEqual(set(content.RULES), set(changed))
        for path in content.RULES:
            with self.subTest(path=path):
                self.assertEqual(expected(path), changed[path])
        self.assertEqual({}, content.repair(docs | changed))

    def test_values(self):
        changed = content.repair(fixture())
        self.assertEqual(9, changed["rksl-hangers/config.cpp"].count("\t\tarmor="))
        self.assertNotIn("armour", changed["rksl-hangers/config.cpp"])
        viaduct = changed["tmyk_bridges/config.cpp"]
        self.assertIn('        model = "' + BS + "TMYK_Bridges" + BS + 'TMYK_Viaduct.p3d";\r\n', viaduct)
        self.assertIn("icons" + BS + "TMYK_ViaductCenter.pac", viaduct)
        self.assertIn('\t\tmodel="para";', changed["lobois/config.cpp"])
        self.assertIn('\t\tcrew="SoldierWB";    \r\n', changed[content.BWM])

    def test_stuffpbo_member_keeps_its_length(self):
        docs = fixture()
        fixed = content.repair(docs)[content.BWM]
        self.assertEqual(len(docs[content.BWM]), len(fixed))
        self.assertEqual(len('\t\tcrew="BWMOD_Soldier";'), len('\t\tcrew="SoldierWB";    '))

    def test_non_padded_variant_rejected(self):
        rule = ("line", '\t\tcrew="BWMOD_Soldier";', '\t\tcrew="SoldierWB";', 1)
        with patch.dict(content.RULES, {content.BWM: (rule,)}):
            with self.assertRaisesRegex(ValueError, "keep its length"):
                content.repair(fixture())

    def test_missing_member_and_unknown_source_rejected(self):
        for path, rules in content.RULES.items():
            docs = fixture()
            del docs[path]
            with self.subTest(missing=path), self.assertRaises(ValueError):
                content.repair(docs)
            docs = fixture()
            docs[path] = docs[path].replace(rules[0][1], rules[0][1] + " ", 1)
            with self.subTest(unknown=path), self.assertRaises(ValueError):
                content.repair(docs)


if __name__ == "__main__":
    unittest.main()
