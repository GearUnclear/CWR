"""Synthetic regression fixtures for the 2026-10-11.3 loop repairs (LoBo issue 2, AG-1/3/4/10)."""
from pathlib import Path
import re
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "lobo"))
from repair import loops  # noqa: E402
from repair.scripts import _explicit_exit, script_findings  # noqa: E402

M252 = loops.M252
M109 = ("loboart/scripts/m109start.sqs", "coc_arty/scripts/m109start.sqs")
TAIL = "\r\n\r\n "
# Realistic surroundings: earlier loop labels, the jumps that close a loop,
# and a second goto "GO" (no TAB) that the BMP anchor must not match.
PREFIX = {
    loops.BMP: '? _case == 3: goto "GO"\nexit\n#GO\n',
    loops.F5: "#PollLoop\n",
    loops.AIDLIGHTS: "#ckL\n~2\n",
    **{path: "#PrecisionSet\n#precisionset\n" for path in M252},
    **{path: "#PrecisionSet\n" for path in M109 + ("loboart/scripts/m1064a3start.sqs",)},
}
SUFFIX = {
    loops.MIG25: 'goto "CheckPosLoop"\n',
    loops.A4: 'goto "Mainloop"\n',
    loops.ILSACT: "~_wait\n",
    **{path: 'goto "repeat"\n' for path in loops.SETFLAGS},
}


def _gotos(text):
    active = "\n".join(line for line in text.split("\n") if not line.lstrip().startswith(";"))
    return {label.lower() for label in re.findall(r'\bgoto\s+"([^"\n]+)"', active, re.I)}


def fixture_lf(path):
    rules = loops.RULES[path]
    befores = [rule[1] for rule in rules]
    anchors = "\n".join(rule[1] + "\n" + rule[2] for rule in rules) + PREFIX.get(path, "") + SUFFIX.get(path, "")
    names = sorted({n for n in re.findall(r"\b_\w+", anchors)} - {"_this", "_x", "_time"})
    head = "".join(f"{name} = 0\n" for name in names)
    body = PREFIX.get(path, "") + "\n".join(befores) + "\n" + SUFFIX.get(path, "")
    defined = {label.lower() for label in re.findall(r"(?m)^\s*#(\S+)", body)}
    needed = set().union(*(_gotos(rule[2]) for rule in rules))
    if path in loops.POST:
        needed.discard("exit")  # the post step appends the explicit #Exit label
    labels = "".join(f"#{label}\n" for label in sorted(needed - defined))
    return head + body + labels + "exit\n"


def crlf(text):
    return text.replace("\n", "\r\n") + TAIL


def expected(path):
    text = fixture_lf(path) + TAIL.replace("\r\n", "\n")
    for kind, before, after, *_ in loops.RULES[path]:
        if kind == "sub":
            text = text.replace(before, after)
        else:
            text = "\n".join(after if line == before else line for line in text.split("\n"))
    if path in loops.POST:
        text = loops.POST[path](text)
    return text.replace("\n", "\r\n")


def fixture():
    docs = {path: crlf(fixture_lf(path)) for path in loops.RULES}
    docs["lobo_fab/unrelated.sqs"] = "untouched\r\n"
    return docs


def waitless_back_edges(text):
    """Port of the issue-2 checker: a goto to an earlier label needs a wait in between."""
    lines = text.replace("\r\n", "\n").split("\n")
    labels = {}
    for i, line in enumerate(lines):
        if line.strip().startswith("#"):
            labels[line.strip()[1:].strip().lower()] = i
    found = []
    for i, line in enumerate(lines):
        stripped = line.strip()
        if stripped.startswith(";"):
            continue
        for match in re.finditer(r'goto\s*"([^"]+)"', stripped, re.I):
            label = match.group(1).lower()
            if label in labels and labels[label] <= i:
                span = [x.strip() for x in lines[labels[label]:i + 1]]
                if not any(x.startswith(("~", "@", "&")) for x in span):
                    found.append((i + 1, label))
    return found


class LoopRepairTests(unittest.TestCase):
    def test_exact_output_and_fixed_point(self):
        docs = fixture()
        changed = loops.repair(docs)
        self.assertEqual(set(loops.RULES), set(changed))
        for path in loops.RULES:
            with self.subTest(path=path):
                self.assertEqual(expected(path), changed[path])
        self.assertEqual({}, loops.repair(docs | changed))

    def test_crlf_and_trailing_bytes_preserved(self):
        changed = loops.repair(fixture())
        for path, text in changed.items():
            with self.subTest(path=path):
                self.assertNotIn("\n", text.replace("\r\n", ""))
                if path not in loops.POST:
                    self.assertTrue(text.endswith(TAIL))

    def test_every_back_edge_waits_after_repair(self):
        docs = fixture()
        changed = loops.repair(docs)
        for path, text in changed.items():
            with self.subTest(path=path):
                self.assertEqual([], waitless_back_edges(text))
        # The originals really spin: frame loops and the MaxWind self-jump.
        for path in (loops.FLOAT, loops.DOLPHIN, loops.MIG25, loops.BMP, loops.F5) + M252 + M109:
            with self.subTest(original=path):
                self.assertTrue(waitless_back_edges(docs[path]))

    def test_bmp_gets_explicit_exit_once(self):
        fixed = loops.repair(fixture())[loops.BMP]
        self.assertTrue(fixed.endswith("\r\n\r\n#Exit\r\nexit\r\n"))
        self.assertEqual(1, fixed.count("#Exit"))
        self.assertEqual(fixed, _explicit_exit(fixed.replace("\r\n", "\n")).replace("\n", "\r\n"))
        self.assertEqual(2, fixed.count('goto "GO"'))

    def test_maxwind_keeps_exactly_its_duplicate_label_finding(self):
        changed = loops.repair(fixture())
        for path in M252:
            with self.subTest(path=path):
                self.assertEqual(["duplicate label: precisionset"], script_findings(changed[path].replace("\r\n", "\n")))
                self.assertIn(':Goto "WindSet"', changed[path])
        for path in M109:
            self.assertEqual([], script_findings(changed[path].replace("\r\n", "\n")))
        for path in M252:
            docs = fixture()
            docs[path] = docs[path].replace("exit\r\n", "#WindSet\r\nexit\r\n", 1)
            with self.subTest(extra_finding=path), self.assertRaisesRegex(ValueError, "static findings"):
                loops.repair(docs)

    def test_partial_repair_of_two_rule_scripts_is_completed(self):
        for path in (loops.DOLPHIN, loops.MIG25, loops.A4):
            docs = fixture()
            _, before, after, _ = loops.RULES[path][0]
            docs[path] = docs[path].replace(before.replace("\n", "\r\n"), after.replace("\n", "\r\n"))
            with self.subTest(path=path):
                self.assertEqual(expected(path), loops.repair(docs)[path])

    def test_unknown_duplicate_and_missing_members_rejected(self):
        for path, rules in loops.RULES.items():
            kind, before, after = rules[0][:3]
            for mode in ("missing", "unknown", "duplicate", "mixed_duplicate"):
                docs = fixture()
                if mode == "missing":
                    del docs[path]
                elif mode == "unknown":
                    docs[path] = docs[path].replace(before.replace("\n", "\r\n"), "unrecognized source")
                elif mode == "duplicate":
                    docs[path] += "\r\n" + before.replace("\n", "\r\n") + "\r\n"
                else:
                    docs[path] += "\r\n" + after.replace("\n", "\r\n") + "\r\n"
                with self.subTest(path=path, mode=mode), self.assertRaises(ValueError):
                    loops.repair(docs)

    def test_addons_cover_every_member(self):
        self.assertEqual({path.split("/")[0] for path in loops.RULES}, set(loops.ADDONS))
        self.assertEqual(17, len(loops.RULES))


if __name__ == "__main__":
    unittest.main()
