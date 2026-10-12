"""Synthetic regression fixtures for the cluster-bomb repairs (LoBo issues 4 and 5)."""
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "lobo"))
from repair import clusters, scripts  # noqa: E402

TAIL = "\r\n\r\n "
HEAD = '_plane = _this select 0\n_ammoname = _this select 4\n_missobj = objNull\n'
A2 = "lobo_a4/scripts/firemissiles_a2.sqs"


def config(*names):
    body = "".join(f"\tclass {name}: Default {{}};\n" for name in names)
    return "class CfgAmmo\n{\n\tclass Default {};\n" + body + "};\n"


def fixture_lf(path):
    rules = clusters.RULES[path]
    # Real order in the IL-28 fired script: nearestobject, then the exec.
    body = "\n".join(rule[1] for rule in (reversed(rules) if path == clusters.IL28_FIRED else rules))
    return HEAD + body + ' [_plane]\n' + ("" if path in clusters.POST else 'goto "Exit"\n#Exit\n') + "exit\n"


def crlf(text):
    return text.replace("\n", "\r\n") + TAIL


def fixture():
    docs = {path: crlf(fixture_lf(path)) for path in clusters.RULES}
    docs[clusters.ROCKEYE] = "_bomb = _this select 0\r\nexit\r\n"
    docs[clusters.AIRAMMO_CONFIG] = config(*clusters.REQUIRED_CLASSES).replace("\n", "\r\n")
    return docs


def expected(path):
    text = fixture_lf(path) + TAIL.replace("\r\n", "\n")
    for _, before, after, _ in clusters.RULES[path]:
        text = text.replace(before, after)
    if path in clusters.POST:
        text = clusters.POST[path](text)
    return text.replace("\n", "\r\n")


def a4_missile_fixture():
    """Shape accepted by scripts._missile for the A-4 cluster scripts (two helpers)."""
    return (
        '_plane = _this select 0\n_ammoname = "TestProjectile"\n'
        '_missobj = nearestobject[_plane, _ammoname]\n'
        '_sensor = "EmptyDetector" createVehicle [0,0,0]\n'
        '_sensor setpos [getpos _plane select 0, getpos _plane select 1];\n'
        '_result = getpos _sensor select 2;\n'
        '[_missobj] ' + clusters.F16_EXEC + '\n'
        '_bpos = [1,2,3]\n_sensor = "EmptyDetector" createVehicle [0,0,0]\n'
        '_sensor setpos [_bpos select 0, _bpos select 1];\n'
        '_result = getpos _sensor select 2;\ndeleteVehicle _sensor\ngoto "Exit"\n#Exit\nexit\n'
    )


class ClusterRepairTests(unittest.TestCase):
    def test_exact_changes_and_fixed_point(self):
        docs = fixture()
        changed = clusters.repair(docs)
        self.assertEqual(set(clusters.RULES), set(changed))
        for path in clusters.RULES:
            with self.subTest(path=path):
                self.assertEqual(expected(path), changed[path])
                self.assertNotIn("\n", changed[path].replace("\r\n", ""))
        self.assertEqual({}, clusters.repair(docs | changed))

    def test_callers_reach_the_consolidated_script(self):
        changed = clusters.repair(fixture())
        for path in ("lobo_a4/scripts/firemissiles_a2.sqs", "lobo_a4/scripts/firemissiles_a3.sqs",
                     "lobo_f4/scripts/firemissiles_ww.sqs", clusters.IL28_FIRED):
            text = changed[path]
            with self.subTest(path=path):
                self.assertNotIn("LoBoF16", text)
                self.assertNotIn("LoBoF15", text)
                reference = text.split('exec "', 1)[1].split('"', 1)[0]
                self.assertEqual(clusters.ROCKEYE, reference.lstrip(chr(92)).replace(chr(92), "/").lower())

    def test_il28_owner_guard_and_explicit_exit_are_idempotent(self):
        docs = fixture()
        fixed = clusters.repair(docs)[clusters.IL28_FIRED]
        self.assertIn('? isNull _missobj: goto "Exit"\r\n? !(local _missobj): goto "Exit"', fixed)
        self.assertLess(fixed.index("? !(local _missobj)"), fixed.index('exec "'))
        self.assertEqual(1, fixed.count("#Exit"))
        self.assertTrue(fixed.endswith("#Exit\r\nexit\r\n"))
        docs[clusters.IL28_FIRED] = fixed
        self.assertNotIn(clusters.IL28_FIRED, clusters.repair(docs))

    def test_missing_or_empty_destination_rejected(self):
        for mode in ("missing", "empty"):
            docs = fixture()
            if mode == "missing":
                del docs[clusters.ROCKEYE]
            else:
                docs[clusters.ROCKEYE] = " \r\n"
            with self.subTest(mode=mode), self.assertRaisesRegex(ValueError, "destination"):
                clusters.repair(docs)
        docs = fixture()
        del docs[clusters.AIRAMMO_CONFIG]
        with self.assertRaises(ValueError):
            clusters.repair(docs)

    def test_each_missing_config_class_rejected(self):
        for name in clusters.REQUIRED_CLASSES:
            docs = fixture()
            others = [n for n in clusters.REQUIRED_CLASSES if n != name]
            docs[clusters.AIRAMMO_CONFIG] = config(*others).replace("\n", "\r\n") + "// class " + name + " is only a comment\r\n"
            with self.subTest(name=name), self.assertRaisesRegex(ValueError, "ammo class"):
                clusters.repair(docs)

    def test_unknown_and_duplicate_sources_rejected(self):
        for path, rules in clusters.RULES.items():
            for _, before, after, _ in rules:
                for mode in ("unknown", "duplicate", "mixed_duplicate"):
                    docs = fixture()
                    if mode == "unknown":
                        docs[path] = docs[path].replace(before, "unrecognized")
                    elif mode == "duplicate":
                        docs[path] += before
                    else:
                        docs[path] += after
                    with self.subTest(path=path, before=before[:20], mode=mode), self.assertRaises(ValueError):
                        clusters.repair(docs)

    def test_composes_with_the_missile_rules_on_an_a4_fixture(self):
        source = a4_missile_fixture()
        both = []
        for order in ("missile_first", "cluster_first"):
            text = source
            docs = fixture()
            if order == "missile_first":
                text = scripts._missile(text, A2)
            docs[A2] = text
            text = clusters.repair(docs).get(A2, text)
            if order == "cluster_first":
                text = scripts._missile(text, A2)
            self.assertEqual([], scripts.script_findings(text))
            docs[A2] = text
            self.assertNotIn(A2, clusters.repair(docs))
            self.assertEqual(text, scripts._missile(text, A2))
            both.append(text)
        self.assertEqual(both[0], both[1])
        self.assertIn(clusters.AIRAMMO_EXEC, both[0])
        self.assertIn('? !(local _missobj): goto "Exit"', both[0])


if __name__ == "__main__":
    unittest.main()
