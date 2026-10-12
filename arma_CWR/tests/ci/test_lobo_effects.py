"""Synthetic regression fixtures for the wreck-glow classes (LoBo issue 9) and the
helicopter wreck burn guard that a defined KioGlowFire makes reachable."""
from pathlib import Path
import re
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "lobo"))
from repair import effects  # noqa: E402
from repair.scripts import script_findings  # noqa: E402

KEY = effects.CONFIG
HEAD = ("#define TEast 0\nclass CfgPatches\n{\n\tclass LoBo_airammo\n\t{\n\t\tunits[]={};\n\t};\n};\n"
        "class CfgAmmo\n{\n\tclass Default {};\n\tclass LoBoMk20Bomblet: Default {};\n")
TAIL = "class CfgNonAIVehicles\n{\n\tclass ProxyWeapon {};\n};"  # no trailing newline, as installed

# Synthetic stand-in with the layout of the BAS helicopter fire.sqs (not its bytes).
HELI = """; synthetic BAS-style helicopter wreck fire
? (mpgame):exit
_lifeTime = 150
_object = _this select 0
@ (getdammage _object)>=0.9
_soundloop = 0
_delay = 0.25
_lifeTicks = _lifeTime / _delay
_lifeTick = _lifeTicks
_ouch = 0


?! (local _object):goto "sound"

_glow = "KioGlowFire" createvehicle [0,0,0]
_glow inflame true

#sound
_soundloop = 0
_glow setpos getpos _object

#Begin
? ! (local _object):goto "client"

#client

;burn player
? player distance _glow <10: player setdammage ((getdammage player) +0.04),goto "scream"

#fire
~_delay

_lifeTick = _lifeTick - 1
_soundloop = _soundloop +1
? _soundloop ==4: _glow say "fire";goto "sound"

?_lifeTick > 0 : goto "Begin"

deletevehicle _glow
exit

#scream
? _ouch == 1:goto "fire"
_ouch = _ouch +1
player groupchat "MEDIC! I'm getting burnt here!"
goto "Begin"
"""
# Written out by hand: the expected result is not derived from effects.RULES.
HELI_FIXED = """; synthetic BAS-style helicopter wreck fire
? (mpgame):exit
_lifeTime = 150
_object = _this select 0
@ (getdammage _object)>=0.9
_soundloop = 0
_delay = 0.25
_lifeTicks = _lifeTime / _delay
_lifeTick = _lifeTicks
_ouch = 0
_fire = 0


?! (local _object):goto "sound"

_glow = "KioGlowFire" createvehicle [0,0,0]
_glow inflame true

#sound
_soundloop = 0
_glow setpos getpos _object

#Begin
? ! (local _object):goto "client"

#client
? !(local _object): goto "fire"

;burn player
? player != (vehicle player): goto "fire"
? local player && alive player && player distance _object < 10 && _ouch != 1: player setdammage ((getdammage player) +0.04); goto "scream"

#fire
~_delay

_lifeTick = _lifeTick - 1
_soundloop = _soundloop +1
_fire = _fire + 1
? _fire >= 40: _fire = 0; _ouch = 0
? _soundloop ==4: _glow say "fire";goto "sound"

?_lifeTick > 0 : goto "Begin"

deletevehicle _glow
exit

#scream
? _ouch == 1:goto "fire"
_ouch = _ouch +1
player groupchat "MEDIC! I'm getting burnt here!"
goto "Begin"
"""
HELI_MEMBERS = ("loboeghelo/engine/fire.sqs", "loboeghelo/engine2/fire.sqs", "loboeghelo/engine3/fire.sqs",
                "loboidfhelo/ch53/engine/fire.sqs", "loboidfhelo/uh1/engine/fire.sqs")


def source(text=None):
    return (HEAD + "};\n" + TAIL if text is None else text).replace("\n", "\r\n")


def documents(config=None, heli=HELI):
    docs = {KEY: source(config)}
    docs.update({key: heli.replace("\n", "\r\n") for key in HELI_MEMBERS})
    return docs


def parents(block):
    return dict(re.findall(r"(?m)^\tclass (\w+)(?:: (\w+))?", block))


class EffectsRepairTests(unittest.TestCase):
    def test_insertion_before_anchor_preserves_every_other_byte(self):
        original = source()
        fixed = effects.repair(documents())[KEY]
        block = effects.BLOCK.replace("\n", "\r\n")
        self.assertEqual(original.replace("};\r\nclass CfgNonAIVehicles", "};\r\n" + block + "class CfgNonAIVehicles"), fixed)
        self.assertEqual(len(original) + len(block), len(fixed))
        self.assertNotIn("\n", fixed.replace("\r\n", ""))
        self.assertTrue(fixed.endswith(TAIL.replace("\n", "\r\n")))

    def test_already_applied_is_a_noop(self):
        fixed = effects.repair(documents())
        self.assertEqual({KEY, *HELI_MEMBERS}, set(fixed))
        again = documents() | fixed
        self.assertEqual({}, effects.repair(again))
        lf = {key: value.replace("\r\n", "\n") for key, value in again.items()}
        self.assertEqual({}, effects.repair(lf))

    def test_class_chain_and_names(self):
        chain = parents(effects.BLOCK)
        self.assertEqual({"All": "", "Static": "All", "Building": "Static", "NonStrategic": "Building",
                          "Fire": "NonStrategic", "LoBo_WreckGlow": "Fire", "M1Smoker": "LoBo_WreckGlow",
                          "M60Smoker": "LoBo_WreckGlow", "Smoker": "LoBo_WreckGlow",
                          "KioGlowFire": "LoBo_WreckGlow"}, chain)
        for base in ("All", "Static", "Building", "NonStrategic", "Fire", "M1Smoker", "M60Smoker", "Smoker", "KioGlowFire"):
            self.assertRegex(effects.BLOCK, r"(?m)^\tclass " + base + r"(?:: \w+)? \{\};$")
        self.assertIn('\t\tmodel="empty";\n', effects.BLOCK)
        self.assertIn("\t\tscope=1;\n", effects.BLOCK)
        self.assertNotIn(chr(0x2014), effects.BLOCK)  # no em dash in shipped comments

    def test_partial_or_foreign_state_rejected(self):
        block = effects.BLOCK
        partial = block.replace("\tclass KioGlowFire: LoBo_WreckGlow {};\n", "")
        cases = {
            "partial block": HEAD + "};\n" + partial + TAIL,
            "foreign CfgVehicles": HEAD + "};\nclass CfgVehicles\n{\n\tclass Car {};\n};\n" + TAIL,
            "pre-declared Smoker": HEAD + "\tclass Smoker: Default {};\n};\n" + TAIL,
            "comment fragment": HEAD + "};\n" + "\n".join(block.split("\n")[7:10]) + "\n" + TAIL,
            "missing anchor": HEAD + "};\n\n" + TAIL,
            "duplicated anchor": HEAD + "};\n" + TAIL + "\n};\n" + TAIL,
            "block twice": HEAD + "};\n" + block + block + TAIL,
        }
        for name, text in cases.items():
            with self.subTest(case=name), self.assertRaises(ValueError):
                effects.repair(documents(text))
        with self.assertRaises(ValueError):
            effects.repair({})


class HelicopterBurnTests(unittest.TestCase):
    def test_exact_guarded_burn_and_crlf(self):
        fixed = effects.repair(documents())
        for key in HELI_MEMBERS:
            self.assertEqual(HELI_FIXED.replace("\n", "\r\n"), fixed[key])
        self.assertEqual([], script_findings(HELI_FIXED))
        self.assertEqual({"loboeghelo", "loboidfhelo", "lobo_airammo"}, set(effects.ADDONS))

    def test_burn_policy_matches_the_ground_scripts(self):
        lines = HELI_FIXED.split("\n")
        burn = next(i for i, line in enumerate(lines) if "setdammage" in line)
        # The seated-player test runs right before the burn, and the old
        # unguarded glow-distance burn is gone.
        self.assertEqual('? player != (vehicle player): goto "fire"', lines[burn - 1])
        self.assertTrue(lines[burn].startswith("? local player && alive player && player distance _object < 10 && _ouch != 1:"))
        self.assertNotIn("distance _glow", HELI_FIXED)
        self.assertNotIn("+0.04),goto", HELI_FIXED)
        self.assertIn("\n? _fire >= 40: _fire = 0; _ouch = 0\n", HELI_FIXED)
        self.assertIn("\n_ouch = 0\n_fire = 0\n", HELI_FIXED)
        self.assertEqual(1, HELI_FIXED.count("setdammage"))

    def test_partial_repair_is_completed(self):
        partial = HELI.replace("_lifeTick = _lifeTicks\n_ouch = 0\n", "_lifeTick = _lifeTicks\n_ouch = 0\n_fire = 0\n")
        fixed = effects.repair(documents(heli=partial))
        for key in HELI_MEMBERS:
            self.assertEqual(HELI_FIXED.replace("\n", "\r\n"), fixed[key])

    def test_missing_member_unknown_or_duplicate_anchor_rejected(self):
        docs = documents()
        del docs[HELI_MEMBERS[3]]
        with self.assertRaisesRegex(ValueError, "Missing members"):
            effects.repair(docs)
        for name, text in {
            "reworded burn": HELI.replace("+0.04),goto", "+0.05),goto"),
            "duplicated burn": HELI.replace("\n#fire\n", '\n? player distance _glow <10: player setdammage ((getdammage player) +0.04),goto "scream"\n#fire\n'),
            "duplicated client block": HELI + "#client\n\n;burn player\n? player distance _glow <10: player setdammage ((getdammage player) +0.04),goto \"scream\"\n",
            "no tick counter anchor": HELI.replace("_soundloop = _soundloop +1\n", "_soundloop = _soundloop + 1\n"),
        }.items():
            with self.subTest(case=name), self.assertRaises(ValueError):
                effects.repair(documents(heli=text))


if __name__ == "__main__":
    unittest.main()
