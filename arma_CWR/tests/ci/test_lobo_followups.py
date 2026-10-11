"""Synthetic regression fixtures for the three deadline-safe script fixes."""
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "lobo"))
from repair import followups


def fixture():
    docs = {path: "\r\r\n".join(before for before, _ in rules) + "\r\r\n"
            for path, rules in followups.RULES.items()}
    docs[followups.AP_PICKUP] = "exit\n"
    docs[followups.INSERTION_TARGET] = "exit\n"
    docs["lobomig/unrelated.sqs"] = "untouched\r\n"
    return docs


class FollowupRepairTests(unittest.TestCase):
    def test_exact_changes_and_fixed_point(self):
        docs = fixture()
        changed = followups.repair(docs)
        self.assertEqual(set(followups.RULES), set(changed))
        for path, rules in followups.RULES.items():
            expected = docs[path]
            for before, after in rules:
                expected = expected.replace(before, after)
            self.assertEqual(expected, changed[path])
        self.assertEqual({}, followups.repair(docs | changed))

    def test_corrected_paths_resolve_to_required_destinations(self):
        changed = followups.repair(fixture())
        for source, target in ((followups.AP_MINE, followups.AP_PICKUP),
                               (followups.FAST_INSERTION, followups.INSERTION_TARGET)):
            reference = changed[source].strip().strip('"').lstrip(chr(92))
            self.assertEqual(target, reference.replace(chr(92), '/').lower())

    def test_partial_clamp_repair_is_completed(self):
        docs = fixture()
        before, after = followups.RULES[followups.SWING_WING][0]
        docs[followups.SWING_WING] = docs[followups.SWING_WING].replace(before, after)
        fixed = followups.repair(docs)[followups.SWING_WING]
        self.assertIn('?(_fraction > 1)', fixed)
        self.assertIn('?(_fraction < 0)', fixed)

    def test_missing_or_empty_destination_rejected(self):
        for target in (followups.AP_PICKUP, followups.INSERTION_TARGET):
            for empty in (False, True):
                docs = fixture()
                if empty:
                    docs[target] = '  '
                else:
                    del docs[target]
                with self.subTest(target=target, empty=empty), self.assertRaises(ValueError):
                    followups.repair(docs)

    def test_missing_unknown_and_duplicate_source_rejected(self):
        for path, rules in followups.RULES.items():
            for mode in ('missing', 'unknown', 'duplicate', 'mixed_duplicate'):
                docs = fixture()
                before, after = rules[0]
                if mode == 'missing':
                    del docs[path]
                elif mode == 'unknown':
                    docs[path] = docs[path].replace(before, 'unrecognized source')
                elif mode == 'duplicate':
                    docs[path] += before
                else:
                    docs[path] += after
                with self.subTest(path=path, mode=mode), self.assertRaises(ValueError):
                    followups.repair(docs)


if __name__ == '__main__':
    unittest.main()
