"""Read-only prototype of the issue-8 rule. Imports the worktree's repair.scripts
helpers, applies the LCM form to the four live aircraft copies, remaps the two
wrong hidden-selection ladders, and verifies findings, idempotency and shape.
Writes candidate outputs only under this scratch directory."""
import difflib
import hashlib
import re
import sys
from pathlib import Path

sys.path.insert(0, r"D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo")
from repair import scripts  # noqa: E402
from repair.scripts import _replace, _explicit_exit, script_findings  # noqa: E402

root = Path(sys.argv[1])
out = Path(sys.argv[2])

# First hiddenSelections[] index of "num1" in every class that starts the script.
LIVE = {
    "lobof15/scr/number.sqs": ("LoBoF15/scr/number.sqs", 2, True),
    "lobof16/scr/number.sqs": ("LoBoF16/scr/number.sqs", 2, True),
    "lobof16/scr/number2.sqs": ("LoBoF16/scr/number2.sqs", 2, True),
    "loboil28/script/number.sqs": ("LoBoIL28/script/number.sqs", 0, True),
    "lobo_f4/scripts/number.sqs": ("LoBo_F4/scripts/number.sqs", 1, False),
}
LADDER = re.compile(r'(?m)^(\?_num([123]) >=? \d+: _plane setObjectTexture \[)(\d+)(?=, ")')


def slots(text, base):
    seen = {1: 0, 2: 0, 3: 0}

    def fix(m):
        digit, slot = int(m.group(2)), int(m.group(3))
        if slot not in (digit + 1, base + digit - 1):
            raise ValueError(f"unexpected slot {slot} on ladder {digit}")
        seen[digit] += 1
        return f"{m.group(1)}{base + digit - 1}"

    text = LADDER.sub(fix, text)
    if seen != {1: 10, 2: 10, 3: 10}:
        raise ValueError(f"unexpected ladder lines {seen}")
    return text


def number(text, base, drop_arg):
    if drop_arg:
        text = _replace(text, "_number = _this select 1",
                        "; Init provides only the vehicle; an unused second argument is unnecessary.")
    text = _replace(text, "_unit setObjectTexture", "_plane setObjectTexture", 30)
    text = _replace(text, 'goto "n2"', 'goto "Num2"', 10)
    text = _replace(text, 'goto "n3"', 'goto "Num3"', 10)
    text = _replace(text, '], goto "', ']; goto "', 30)
    text = slots(text, base)
    return _explicit_exit(text)


def shape(t):
    t = re.sub(r'"\\[^"]+?\.paa"', '"P"', t)
    t = re.sub(r"setObjectTexture \[\d+", "setObjectTexture [N", t)
    return t.replace("; Init provides only the vehicle; an unused second argument is unnecessary.",
                     "_number = _this select 1")


lcm = (root / "LoBo_FAB/lcm/scripts/number.sqs").read_bytes().decode("latin1").replace("\r\n", "\n")
for key, (rel, base, drop) in LIVE.items():
    raw = (root / rel).read_bytes()
    src = raw.decode("latin1")
    nl = "\r\n" if "\r\n" in src else "\n"
    norm = src.replace("\r\n", "\n")
    before = script_findings(norm)
    upd = number(norm, base, drop)
    again = number(upd, base, drop)
    after = script_findings(upd)
    # The LCM rule itself (scripts._number) must give identical text when no remap/arg exception applies.
    same_as_lcm_rule = (scripts._number(norm) == upd) if (base == 2 and drop) else None
    final = upd.replace("\n", nl).encode("latin1")
    dst = out / rel
    dst.parent.mkdir(parents=True, exist_ok=True)
    dst.write_bytes(final)
    used = sorted(set(int(s) for s in re.findall(r"setObjectTexture \[(\d+)", upd)))
    print(f"== {key}")
    print(f"   before findings: {before}")
    print(f"   after findings:  {after}; idempotent={again == upd}; same_as_scripts._number={same_as_lcm_rule}")
    print(f"   shape==repaired LCM: {shape(upd) == shape(lcm)}; slots used {used}; base {base}; drop_arg {drop}")
    print(f"   bytes {len(raw)} -> {len(final)}; sha256 {hashlib.sha256(raw).hexdigest()} -> {hashlib.sha256(final).hexdigest()}")
    diff = list(difflib.unified_diff(norm.split("\n"), upd.split("\n"), lineterm="", n=0))
    print("   changed lines:", sum(1 for d in diff if d.startswith("+") and not d.startswith("+++")))
