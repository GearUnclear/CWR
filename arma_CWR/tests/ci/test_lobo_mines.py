"""Synthetic regression fixtures for the placed-mine repairs (LoBo issue 3)."""
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "lobo"))
from repair import mines  # noqa: E402

TAIL = "\r\n\r\n "
FLARES = ("lobo_mines/scripts/leucht_mine.sqs", "jof_objects1/leucht_mine.sqs")
HEAD = ('_L1 = _this select 0\n_X = 0\n_Y = 0\n_Z = 0\n_D = 0\n_J = objNull\n_W = 0\n_M = objNull\n'
        '_Mess = "HeliHEmpty" createVehicle [0,0,0]\n')


def fixture_lf(path):
    rules = mines.RULES[path]
    body = "\n".join(rule[1].rstrip("\n") for rule in rules)
    # Every non-flare mine also carries the pickup exit that must stay as it is.
    tail = '? not alive _L1:exit\n' if path not in FLARES else ""
    return HEAD + body + "\n" + tail + '? _J != _L1 : goto "Zut"\nexit\n'


def crlf(text):
    return text.replace("\n", "\r\n") + TAIL


def fixture():
    docs = {path: crlf(fixture_lf(path)) for path in mines.RULES}
    docs["lobo_mines/scripts/feuer.sqs"] = "untouched\r\n"
    return docs


def expected(path):
    text = fixture_lf(path)
    for _, before, after, _ in mines.RULES[path]:
        text = text.replace(before, after)
    return crlf(text)


class MineRepairTests(unittest.TestCase):
    def test_exact_output_and_fixed_point(self):
        docs = fixture()
        changed = mines.repair(docs)
        self.assertEqual(set(mines.RULES), set(changed))
        for path in mines.RULES:
            with self.subTest(path=path):
                self.assertEqual(expected(path), changed[path])
                self.assertTrue(changed[path].endswith(TAIL))
                self.assertNotIn("\n", changed[path].replace("\r\n", ""))
        self.assertEqual({}, mines.repair(docs | changed))

    def test_guard_present_once_per_polled_script(self):
        changed = mines.repair(fixture())
        for path, text in changed.items():
            lines = text.split("\r\n")
            guard = mines.FLARE_GUARD if path in FLARES else mines.GUARD
            poll = mines.FLARE_POLL if path in FLARES else mines.POLL
            with self.subTest(path=path):
                self.assertEqual(1, lines.count(guard))
                self.assertEqual(lines.index("#Zut") + 1, lines.index(poll))
                self.assertEqual(lines.index(poll) + 1, lines.index(guard))
                self.assertNotIn("~0.001", lines)
                self.assertNotIn("~0.0001", lines)

    def test_pickup_exit_unchanged_outside_flare_mines(self):
        changed = mines.repair(fixture())
        for path, text in changed.items():
            with self.subTest(path=path):
                if path in FLARES:
                    self.assertNotIn("? not alive _L1:exit\r\n", text)
                    self.assertIn("? not alive _L1:deleteVehicle _Mess; exit\r\n", text)
                    self.assertIn("#Ende\r\ndeleteVehicle _Mess\r\n", text)
                    self.assertEqual(3, text.count("deleteVehicle _Mess"))
                else:
                    self.assertIn("? not alive _L1:exit\r\n", text)
                    self.assertNotIn("_Mess;", text)

    def test_partial_flare_repair_is_completed(self):
        for done in range(1, 3):
            docs = fixture()
            for path in FLARES:
                for _, before, after, _ in mines.LEUCHT[:done]:
                    docs[path] = docs[path].replace(before.replace("\n", "\r\n"), after.replace("\n", "\r\n"))
            changed = mines.repair(docs)
            for path in FLARES:
                with self.subTest(done=done, path=path):
                    self.assertEqual(expected(path), changed[path])

    def test_unknown_duplicate_and_missing_rejected(self):
        for path, rules in mines.RULES.items():
            for index, (_, before, after, _) in enumerate(rules):
                for mode in ("missing", "unknown", "duplicate", "mixed_duplicate"):
                    if mode == "missing" and index:
                        continue
                    docs = fixture()
                    if mode == "missing":
                        del docs[path]
                    elif mode == "unknown":
                        docs[path] = docs[path].replace(before.replace("\n", "\r\n"), "unrecognized\r\n")
                    elif mode == "duplicate":
                        docs[path] += before.replace("\n", "\r\n")
                    else:
                        docs[path] += after.replace("\n", "\r\n")
                    with self.subTest(path=path, rule=index, mode=mode), self.assertRaises(ValueError):
                        mines.repair(docs)

    def test_member_set(self):
        self.assertEqual(17, len(mines.RULES))
        self.assertEqual({path.split("/")[0] for path in mines.RULES}, set(mines.ADDONS))


if __name__ == "__main__":
    unittest.main()
