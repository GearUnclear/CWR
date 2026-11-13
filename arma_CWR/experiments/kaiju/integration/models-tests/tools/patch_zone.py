r"""The model round's proposed change to the combat feature's hit zones, as exact text replacements.

    python patch_zone.py <path to kj\combat\init.sqf>

Applied here only to the model round's own copy of the script set (models-tests\kj). The integration's
set (integration\kj) is the other agent's: FOR-ENGINE.md asks for it there.

What it changes. The combat feature tells the place of a hit from its position in his frame:
  legs     under 0.45 of his height
  head     over 0.88 of his height
  furnace  within 0.2 of his height of the furnace point (in the pose, kjPoint)
On A that works. On D it does not: the hound's head hangs at 24 to 36 m of 39, so a hit on the head
is "body", and what is over 0.88 of D's height is the dead lizard's neck. After the patch the three
numbers come from the creature's own model (the config entries gen_config.py writes from the model
build) and the head is a ball round its own memory point, followed in the pose like the furnace:
  kjLegsUp      metres: under this is legs (the hip joints)
  kjHeadPoint   memory point at the middle of the skull; kjHeadR metres round it is head
  kjHeadRight / kjHeadFwd / kjHeadUp   the same point in the rest pose, for a binary without kjPoint
  kjFurnaceR    metres round the furnace point that is furnace
A class without the entries keeps the old rule, number for number.
"""
import sys

EDITS = [
    # 1. defaults
    ('KJCB_furnacePoint = "zamerny";\n',
     'KJCB_furnacePoint = "zamerny";\n'
     '// [models] the creature\'s own hit regions (set by KJCB_fnSetup from its class); 0 or "" = the old rule\n'
     'KJCB_furnaceR = 8;\nKJCB_legsUp = 18;\nKJCB_headR = 0;\nKJCB_headPoint = "";\nKJCB_head = [0, 0, 36];\n'),
    # 2. the head point follows the pose, like the furnace point
    ('    if ((count _p) == 3) then { KJCB_furnace = [_this, _p] call KJCB_fnToLocal };\n    KJCB_furnace\n',
     '    if ((count _p) == 3) then { KJCB_furnace = [_this, _p] call KJCB_fnToLocal };\n'
     '    // [models] and the head point\n'
     '    if (KJCB_native and (KJCB_headPoint != "")) then { _p = kjPoint [_this, KJCB_headPoint]; if ((count _p) == 3) then { KJCB_head = [_this, _p] call KJCB_fnToLocal } };\n'
     '    KJCB_furnace\n'),
    # 3. the zone rule
    ('    if (_u < (KJCB_LEGS_UP * KJCB_H)) then { _z = "legs"; _f = KJCB_Z_LEGS; _i = 3 };\n'
     '    if (_u >= (KJCB_HEAD_UP * KJCB_H)) then { _z = "head"; _f = KJCB_Z_HEAD; _i = 1 };\n',
     '    // [models] legs: under the hip joints of this creature; head: a ball round its own head point\n'
     '    if (_u < KJCB_legsUp) then { _z = "legs"; _f = KJCB_Z_LEGS; _i = 3 };\n'
     '    if (KJCB_headR > 0) then {\n'
     '        _d2 = ((_this select 0) - (KJCB_head select 0)) ^ 2 + ((_this select 1) - (KJCB_head select 1)) ^ 2 + (_u - (KJCB_head select 2)) ^ 2;\n'
     '        if (_d2 <= (KJCB_headR ^ 2)) then { _z = "head"; _f = KJCB_Z_HEAD; _i = 1 }\n'
     '    } else {\n'
     '        if (_u >= (KJCB_HEAD_UP * KJCB_H)) then { _z = "head"; _f = KJCB_Z_HEAD; _i = 1 }\n'
     '    };\n'),
    ('    if (_d2 <= ((KJCB_FURNACE_R * KJCB_H) ^ 2)) then {\n',
     '    if (_d2 <= (KJCB_furnaceR ^ 2)) then {\n'),
    # 4. read the creature's numbers
    ('    KJCB_furnace = [0, [_k, "kjFurnaceFwd", 0.132 * KJCB_H] call KJCB_fnCfg, [_k, "kjFurnaceUp", 0.79 * KJCB_H] call KJCB_fnCfg];\n',
     '    KJCB_furnace = [0, [_k, "kjFurnaceFwd", 0.132 * KJCB_H] call KJCB_fnCfg, [_k, "kjFurnaceUp", 0.79 * KJCB_H] call KJCB_fnCfg];\n'
     '    // [models] the hit regions of this creature\'s own model\n'
     '    KJCB_furnaceR = [_k, "kjFurnaceR", KJCB_FURNACE_R * KJCB_H] call KJCB_fnCfg;\n'
     '    KJCB_legsUp = [_k, "kjLegsUp", KJCB_LEGS_UP * KJCB_H] call KJCB_fnCfg;\n'
     '    KJCB_headR = [_k, "kjHeadR", 0] call KJCB_fnCfg;\n'
     '    KJCB_headPoint = [_k, "kjHeadPoint", ""] call KJCB_fnCfgStr;\n'
     '    KJCB_head = [[_k, "kjHeadRight", 0] call KJCB_fnCfg, [_k, "kjHeadFwd", 0] call KJCB_fnCfg, [_k, "kjHeadUp", 0.92 * KJCB_H] call KJCB_fnCfg];\n'
     '    logInfo format ["KJCOMBAT regions: legs under %1 m, head %2 m round %3 (%4), furnace %5 m round its point", KJCB_legsUp, KJCB_headR, KJCB_headPoint, KJCB_head, KJCB_furnaceR];\n'),
]


def main():
    path = sys.argv[1]
    raw = open(path, "rb").read()
    crlf = b"\r\n" in raw
    text = raw.decode("utf-8").replace("\r\n", "\n")
    if "[models]" in text:
        print("already patched: %s" % path)
        return 0
    for old, new in EDITS:
        if text.count(old) != 1:
            raise SystemExit("patch_zone: expected exactly one of:\n%s\nfound %d" % (old, text.count(old)))
        text = text.replace(old, new)
    if crlf:
        text = text.replace("\n", "\r\n")
    open(path, "wb").write(text.encode("utf-8"))
    print("patched %s (%d edits)" % (path, len(EDITS)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
