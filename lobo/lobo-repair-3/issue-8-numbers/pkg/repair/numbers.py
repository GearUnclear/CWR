"""LoBo issue 8: aircraft tail-number scripts, in the form repair .1 gave the LCM.

Each live copy stores the vehicle in _plane but sets textures on the undefined
_unit and jumps to the missing labels n2/n3, so no digit is ever drawn. Slots
follow each class's hiddenSelections[]: num1 is index 2 on the F-15/F-16, 1 on
the F-4 (index 4 is its afterburner flame) and 0 on the IL-28.
"""
import re

from .scripts import _explicit_exit, _replace, script_findings

UNUSED_ARGUMENT = "; Init provides only the vehicle; an unused second argument is unnecessary."
# member: (hiddenSelections index of "num1", init passes only the vehicle)
NUMBER_SCRIPTS = {
    "lobof15/scr/number.sqs": (2, True),
    "lobof16/scr/number.sqs": (2, True),
    "lobof16/scr/number2.sqs": (2, True),
    "loboil28/script/number.sqs": (0, True),
    "lobo_f4/scripts/number.sqs": (1, False),
}
ADDONS = tuple(sorted({path.split("/")[0] for path in NUMBER_SCRIPTS}))
LADDER = re.compile(r'(?m)^(\?_num([123]) >=? \d+: _plane setObjectTexture \[)(\d+)(?=, ")')


def _slots(text: str, base: int) -> str:
    """Map digit ladder N to num1+N-1, accepting only the original or repaired slot."""
    seen = {1: 0, 2: 0, 3: 0}

    def fix(match):
        digit, slot = int(match.group(2)), int(match.group(3))
        if slot not in (digit + 1, base + digit - 1):
            raise ValueError(f"Unexpected texture slot {slot} in digit ladder {digit}")
        seen[digit] += 1
        return f"{match.group(1)}{base + digit - 1}"

    text = LADDER.sub(fix, text)
    if seen != {1: 10, 2: 10, 3: 10}:
        raise ValueError(f"Unexpected digit ladders: {seen}")
    return text


def _number(text: str, base: int, vehicle_only: bool) -> str:
    if vehicle_only:
        text = _replace(text, "_number = _this select 1", UNUSED_ARGUMENT)
    text = _replace(text, "_unit setObjectTexture", "_plane setObjectTexture", 30)
    text = _replace(text, 'goto "n2"', 'goto "Num2"', 10)
    text = _replace(text, 'goto "n3"', 'goto "Num3"', 10)
    text = _replace(text, '], goto "', ']; goto "', 30)
    return _explicit_exit(_slots(text, base))


def repair(documents: dict[str, str]) -> dict[str, str]:
    missing = sorted(set(NUMBER_SCRIPTS) - documents.keys())
    if missing:
        raise ValueError("Missing aircraft number scripts: " + ", ".join(missing))
    changed = {}
    for path, (base, vehicle_only) in NUMBER_SCRIPTS.items():
        source = documents[path]
        newline = "\r\n" if "\r\n" in source else "\n"
        try:
            updated = _number(source.replace("\r\n", "\n"), base, vehicle_only)
            findings = script_findings(updated)
            if findings:
                raise ValueError("; ".join(findings))
        except ValueError as exc:
            raise ValueError(f"{path}: {exc}") from exc
        updated = updated.replace("\n", newline)
        if updated != source:
            changed[path] = updated
    return changed
