"""Synthetic regression fixtures for the aircraft tail-number repairs (LoBo issue 8)."""
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "lobo"))
from repair import numbers, scripts  # noqa: E402
from repair.scripts import script_findings  # noqa: E402

BS = chr(92)
F4 = "lobo_f4/scripts/number.sqs"
IL28 = "loboil28/script/number.sqs"


def ladder(base=None, *, keep=True, repaired=False):
    """Authored ladder shape; repaired=True renders the expected result for `base`."""
    lines = ["_plane = _this select 0"]
    if keep or not repaired:
        lines.append("_number = _this select 1")
    else:
        lines.append(numbers.UNUSED_ARGUMENT)
    unit = "_plane" if repaired else "_unit"
    sep = ";" if repaired else ","
    for digit in (1, 2, 3):
        lines += [f"#Num{digit}", f"_num{digit} = random(100)"]
        nxt = {1: "Num2" if repaired else "n2", 2: "Num3" if repaired else "n3", 3: "exit"}[digit]
        slot = base + digit - 1 if repaired else digit + 1
        for value in range(9, -1, -1):
            test = f">= 0" if value == 0 else f"> {value * 10}"
            texture = BS + "LoBoTest" + BS + "nums" + BS + f"w{value}.paa"
            lines.append(f'?_num{digit} {test}: {unit} setObjectTexture [{slot}, "{texture}"]{sep} goto "{nxt}"')
        lines.append("")
    lines.append("exit")
    text = "\n".join(lines) + "\n"
    return scripts._explicit_exit(text) if repaired else text


def fixture():
    docs = {path: ladder().replace("\n", "\r\n") for path in numbers.NUMBER_SCRIPTS}
    docs["lobo_fab/lcm/scripts/number.sqs"] = "untouched .1 LCM path\r\n"
    return docs


class NumberRepairTests(unittest.TestCase):
    def test_exact_output_per_slot_map(self):
        changed = numbers.repair(fixture())
        self.assertEqual(set(numbers.NUMBER_SCRIPTS), set(changed))
        for path, (base, vehicle_only) in numbers.NUMBER_SCRIPTS.items():
            with self.subTest(path=path, base=base):
                want = ladder(base, keep=not vehicle_only, repaired=True).replace("\n", "\r\n")
                self.assertEqual(want, changed[path])
                self.assertEqual([], script_findings(changed[path].replace("\r\n", "\n")))

    def test_slot_bases(self):
        self.assertEqual({path: base for path, (base, _) in numbers.NUMBER_SCRIPTS.items()},
                         {"lobof15/scr/number.sqs": 2, "lobof16/scr/number.sqs": 2, "lobof16/scr/number2.sqs": 2,
                          IL28: 0, F4: 1})
        fixed = numbers.repair(fixture())
        self.assertIn('?_num1 > 90: _plane setObjectTexture [1, "', fixed[F4])
        self.assertNotIn("setObjectTexture [4,", fixed[F4])  # index 4 is the F-4 afterburner flame
        self.assertIn('?_num3 >= 0: _plane setObjectTexture [2, "', fixed[IL28])

    def test_f4_keeps_number_argument_others_get_comment(self):
        changed = numbers.repair(fixture())
        self.assertIn("_number = _this select 1", changed[F4])
        self.assertNotIn(numbers.UNUSED_ARGUMENT, changed[F4])
        for path in set(numbers.NUMBER_SCRIPTS) - {F4}:
            self.assertNotIn("_this select 1", changed[path])
            self.assertIn(numbers.UNUSED_ARGUMENT, changed[path])

    def test_idempotent_and_partial_completion(self):
        docs = fixture()
        changed = numbers.repair(docs)
        self.assertEqual({}, numbers.repair(docs | changed))
        for path, (base, vehicle_only) in numbers.NUMBER_SCRIPTS.items():
            partial = docs[path].replace("_unit setObjectTexture", "_plane setObjectTexture", 7)
            partial = partial.replace('goto "n2"', 'goto "Num2"', 3)
            if base != 2:
                partial = partial.replace("setObjectTexture [2,", f"setObjectTexture [{base},", 4)
            with self.subTest(path=path):
                self.assertEqual(changed[path], numbers.repair(docs | {path: partial})[path])

    def test_crlf_preserved_and_lf_supported(self):
        changed = numbers.repair(fixture())
        for text in changed.values():
            self.assertNotIn("\n", text.replace("\r\n", ""))
        lf = {path: ladder() for path in numbers.NUMBER_SCRIPTS}
        for path, text in numbers.repair(lf).items():
            self.assertNotIn("\r", text)

    def test_unexpected_sources_rejected(self):
        cases = {
            "unexpected slot": lambda t: t.replace("setObjectTexture [3,", "setObjectTexture [7,", 1),
            "wrong ladder count": lambda t: t.replace('?_num2 > 50: _unit setObjectTexture [3, "', ';removed [3, "', 1),
            "foreign variable": lambda t: t.replace("_unit setObjectTexture", "_heli setObjectTexture", 1),
            "unknown label": lambda t: t.replace('goto "n3"', 'goto "n4"', 1),
        }
        for name, mutate in cases.items():
            for path in numbers.NUMBER_SCRIPTS:
                docs = fixture()
                docs[path] = mutate(docs[path])
                with self.subTest(case=name, path=path), self.assertRaises(ValueError):
                    numbers.repair(docs)
        for path in numbers.NUMBER_SCRIPTS:
            docs = fixture()
            del docs[path]
            with self.subTest(missing=path), self.assertRaisesRegex(ValueError, "Missing"):
                numbers.repair(docs)

    def test_lcm_paths_are_not_listed(self):
        self.assertFalse(set(scripts.NUMBER_PATHS) & set(numbers.NUMBER_SCRIPTS))


if __name__ == "__main__":
    unittest.main()
