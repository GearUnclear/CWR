"""Synthetic regression fixtures for the undefined config identifiers (CI-1..5, 7, 8, 9)
and the shared line_rule / apply_rules helpers."""
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "lobo"))
from repair import identifiers  # noqa: E402
from repair.rules import apply_rules, line_rule, split_newline  # noqa: E402

BS = chr(92)


def line_fixture(rules):
    """Config lines for whole-line rules: every original spelling, the authored-correct
    baseline lines, a commented copy and a longer line that merely contains the anchor."""
    lines = ["class CfgVehicles", "{"]
    for _, old, new, count, *baseline in rules:
        olds = (old,) if isinstance(old, str) else old
        for i in range(count):
            lines += ["\tclass Unit%d" % len(lines), "\t{", olds[i % len(olds)], "\t};"]
        lines += [new] * (baseline[0] if baseline else 0)
        lines += ["//" + olds[0], olds[0] + " // longer line"]
    lines += ["};", ""]
    return "\n".join(lines)


def line_expected(rules, text):
    for _, old, new, *_ in rules:
        olds = (old,) if isinstance(old, str) else old
        text = "\n".join(new if line in olds else line for line in text.split("\n"))
    return text


class LineRuleTests(unittest.TestCase):
    def test_tuple_originals_and_baseline_accounting(self):
        text = "a=VAir;\nb\na=\"VAir\";\na=2;\n"
        fixed = line_rule(text, ("a=VAir;", 'a="VAir";'), "a=2;", 2, 1)
        self.assertEqual("a=2;\nb\na=2;\na=2;\n", fixed)
        self.assertEqual(fixed, line_rule(fixed, ("a=VAir;", 'a="VAir";'), "a=2;", 2, 1))

    def test_longer_line_containing_the_anchor_is_ignored(self):
        text = "\t\ttype=VArmor;\n//\t\ttype=VArmor;\n\t\ttype=VArmor; // note\n"
        fixed = line_rule(text, "\t\ttype=VArmor;", "\t\ttype=1;", 1)
        self.assertEqual("\t\ttype=1;\n//\t\ttype=VArmor;\n\t\ttype=VArmor; // note\n", fixed)

    def test_partial_state_is_completed(self):
        text = "x=VAir;\nx=2;\nx=VAir;\n"
        self.assertEqual("x=2;\nx=2;\nx=2;\n", line_rule(text, "x=VAir;", "x=2;", 3))

    def test_wrong_totals_rejected(self):
        for text, count, baseline in (("x=VAir;\n", 2, 0), ("x=VAir;\nx=VAir;\n", 1, 0),
                                      ("x=VAir;\n", 1, 1), ("x=2;\nx=2;\n", 1, 0),
                                      ("x=VAir;\nx=VAir;\n", 1, 1)):
            with self.subTest(text=text, count=count, baseline=baseline), self.assertRaises(ValueError):
                line_rule(text, "x=VAir;", "x=2;", count, baseline)
        with self.assertRaises(ValueError):
            line_rule("x=2;\n", "x=2;", "x=2;", 1)

    def test_apply_rules_keeps_crlf_and_reports_only_changes(self):
        docs = {"a/config.cpp": "x=VAir;\r\ny=1;\r\n", "b/config.cpp": "x=2;\r\n"}
        rules = {path: (("line", "x=VAir;", "x=2;", 1),) for path in docs}
        self.assertEqual({"a/config.cpp": "x=2;\r\ny=1;\r\n"}, apply_rules(docs, rules))
        self.assertEqual(("\r\n", "a\nb"), split_newline("a\r\nb"))
        with self.assertRaisesRegex(ValueError, "Missing members"):
            apply_rules({}, rules)
        with self.assertRaisesRegex(ValueError, "contained"):
            apply_rules({"a.sqs": "~0.0010\n"}, {"a.sqs": (("sub", "~0.0010", "~0.001", 1),)})
        with self.assertRaisesRegex(ValueError, "Unknown rule kind"):
            apply_rules(docs, {"a/config.cpp": (("regex", "x", "y", 1),)})


class IdentifierRepairTests(unittest.TestCase):
    def fixture(self):
        return {path: line_fixture(rules).replace("\n", "\r\n") for path, rules in identifiers.RULES.items()}

    def test_every_member_exact_and_fixed_point(self):
        docs = self.fixture()
        changed = identifiers.repair(docs)
        self.assertEqual(set(identifiers.RULES), set(changed))
        for path, rules in identifiers.RULES.items():
            with self.subTest(path=path):
                want = line_expected(rules, line_fixture(rules)).replace("\n", "\r\n")
                self.assertEqual(want, changed[path])
        self.assertEqual({}, identifiers.repair(docs | changed))

    def test_rule_shapes(self):
        changed = identifiers.repair(self.fixture())
        lines = {path: text.split("\r\n") for path, text in changed.items()}
        self.assertIn("\t\ttype \t\t\t\t= 2;", lines["bde_mir3/config.cpp"])
        self.assertIn("\t\ttype \t= 2;", lines["lobo_kfir/config.cpp"])
        self.assertIn("\t\tgunnerCanSee=31;", lines["lobomerkava2/config.cpp"])
        self.assertNotIn('\t\ttype="VAir";', lines["lobof15/config.cpp"])
        self.assertEqual(9, lines["lobof15/config.cpp"].count("\t\ttype=2;"))
        self.assertEqual(10, lines["loboidfhelo/config.cpp"].count("\t\ttype=2;"))
        macro = [line for line in lines["coc_arty/config.cpp"] if "speed=1e10;" in line]
        self.assertEqual(["\t\t\t\tspeed=1e10; " + BS] * 2, macro)
        self.assertIn("\t\thasCommander=0", lines["lobo_fab/config.cpp"])
        # The nil token read as 0 (UnitInfoSoldier); the repair keeps that panel.
        self.assertIn("\t\tUnitInfoType=0;", lines["lobo_zu23/config.cpp"])
        self.assertIn("//\t\ttype=Varmor;", lines["coc_arty/config.cpp"])

    def test_zu23_draft_value_converges_to_the_soldier_panel(self):
        key = "lobo_zu23/config.cpp"
        for line in ("\t\tUnitInfoType=Helicopter,", "\t\tUnitInfoType=1;"):
            text = "class LoBo_ZU23: ZSU\r\n{\r\n" + line + "\r\n\t\thideUnitInfo=false;\r\n};\r\n"
            with self.subTest(line=line):
                fixed = identifiers.repair(self.fixture() | {key: text})[key]
                self.assertEqual(text.replace(line, "\t\tUnitInfoType=0;"), fixed)
                self.assertNotIn(key, identifiers.repair(self.fixture() | {key: fixed}))
        both = "\t\tUnitInfoType=Helicopter,\r\n\t\tUnitInfoType=1;\r\n"
        with self.assertRaises(ValueError):
            identifiers.repair(self.fixture() | {key: both})

    def test_newline_terminated_has_commander_and_commented_varmor(self):
        text = "\t\thasCommander=o\n\t\thasCommander=0\n\t\thasCommander=0\n//\t\ttype=VArmor;\n" \
               + "\t\ttype=VArmor;\n" + "\t\ttype=1;\n" * 4
        rules = identifiers.RULES["lobo_fab/config.cpp"]
        fixed = identifiers.repair(self.fixture() | {"lobo_fab/config.cpp": text})["lobo_fab/config.cpp"]
        self.assertEqual(line_expected(rules, text), fixed)
        self.assertIn("//\t\ttype=VArmor;\n", fixed)

    def test_wrong_counts_and_missing_member_rejected(self):
        for path, rules in identifiers.RULES.items():
            _, old, new, *_ = rules[0]
            first = old if isinstance(old, str) else old[0]
            for mode in ("missing", "extra", "fewer"):
                docs = self.fixture()
                if mode == "missing":
                    del docs[path]
                elif mode == "extra":
                    docs[path] += first + "\r\n"
                else:
                    docs[path] = docs[path].replace("\r\n" + first + "\r\n", "\r\n", 1)
                with self.subTest(path=path, mode=mode), self.assertRaises(ValueError):
                    identifiers.repair(docs)

    def test_addons(self):
        self.assertEqual(29, len(identifiers.RULES))
        self.assertEqual({path.split("/")[0] for path in identifiers.RULES}, set(identifiers.ADDONS))


if __name__ == "__main__":
    unittest.main()
