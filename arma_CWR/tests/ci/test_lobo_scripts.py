"""No-game regression tests; fixtures are synthetic, not distributed mod data."""

from pathlib import Path
import re
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "lobo"))
from repair import scripts  # noqa: E402


def missile_fixture(path=""):
    branches = ""
    if path in ("lobomig/scripts/firemissiles_ic1.sqs", "lobomig/scripts/firemissiles_ic2.sqs"):
        branches = '? true: goto "sp4away"\n#sw4away\n'
    elif path in ("lobof5/scripts/firemissiles2.sqs", "lobof5/scripts/firemissiles3.sqs"):
        branches = '? true: goto "aa2way"\n#aa2away\n'
    elif path == "lobomig/mig25/sqs/firemissiles_cap1.sqs":
        branches = (
            'goto "R60_3away"\ngoto "R60_4away"\n'
            '#R60_1away\n_plane setobjecttexture[3, ""]\n'
            '#R60_2away\n_plane setobjecttexture[4, ""]\n'
            '#R60_1away\n_plane setobjecttexture[5, ""]\n'
            '#R60_2away\n_plane setobjecttexture[6, ""]\n'
        )
    return (
        '_plane = _this select 0\n_ammoname = "TestProjectile"\n'
        + branches
        + '_missobj = nearestobject[_plane, _ammoname]\n'
        '_sensor = "EmptyDetector" createVehicle [0,0,0]\n'
        '; retain terrain coordinate behavior\n'
        '_sensor setpos [getpos _plane select 0, getpos _plane select 1];\n'
        '_result = getpos _sensor select 2;\n'
        '_missobj setpos [1, 2, _result]\ngoto "Exit"\n'
    )


def number_fixture():
    parts = ['_plane = _this select 0', '_number = _this select 1']
    for digit in range(1, 4):
        parts += [f"#Num{digit}", f"_num{digit} = random 100"]
        target = f"n{digit + 1}" if digit < 3 else "exit"
        for value in range(10):
            parts.append(f'? _num{digit} >= {value}: _unit setObjectTexture [{digit + 1}, "digit{value}.paa"], goto "{target}"')
    return "\n".join(parts) + "\nexit\n"


def fire_fixture():
    return (
        '_object = _this select 0\n_ouch = 0\n_ouchAI = 0\n'
        '?! (local _object):goto "sound"\n'
        '_glow = "TestGlow" createvehicle [0,0,0]\n'
        '_glow setpos [getpos _object select 0, getpos _object select 1,1]\n'
        '#sound\n_soundloop = 0\n'
        '_glow setpos [getpos _object select 0, getpos _object select 1,1]\n'
        '#client\n? player != (vehicle player): goTo "fire"\n'
        '? player distance _glow < 10 && _ouch != 1: player setDammage (damage player + 0.02); goTo "scream"\n'
        '#here\n_unitsAI = [getPos _object, 10] call _AI\n'
        '"_x setDammage (damage _x + 0.02)" forEach _unitsAI\n'
        '#fire\n~0.25\ndeletevehicle _glow\nexit\n'
        '#scream\n_ouch = 1\ngoto "here"\n'
    )


class ScriptRepairTests(unittest.TestCase):
    def test_all_shot_targets_resolve_uniquely(self):
        paths = [
            "lobomig/scripts/firemissiles_ic1.sqs",
            "lobomig/scripts/firemissiles_ic2.sqs",
            "lobof5/scripts/firemissiles2.sqs",
            "lobof5/scripts/firemissiles3.sqs",
            "lobomig/mig25/sqs/firemissiles_cap1.sqs",
        ]
        for path in paths:
            with self.subTest(path=path):
                fixed = scripts._missile(missile_fixture(path), path)
                self.assertEqual([], scripts.script_findings(fixed))
                self.assertEqual(fixed, scripts._missile(fixed, path))
        fixed = scripts._missile(missile_fixture(paths[-1]), paths[-1])
        for label, slot in zip(range(1, 5), range(3, 7)):
            self.assertIn(f'#R60_{label}away\n_plane setobjecttexture[{slot}, ""]', fixed)

    def test_helper_is_deleted_immediately_after_last_read(self):
        original = missile_fixture()
        fixed = scripts._missile(original, "lobof15/scr/fire_e.sqs")
        self.assertIn("_result = getpos _sensor select 2;\ndeleteVehicle _sensor", fixed)
        lines = fixed.splitlines()
        read = lines.index("_result = getpos _sensor select 2;")
        self.assertFalse(any("_sensor" in line for line in lines[read + 2:]))
        self.assertLess(fixed.index("? isNull _missobj"), fixed.index('"EmptyDetector"'))
        self.assertLess(fixed.index("? !(local _missobj)"), fixed.index('"EmptyDetector"'))
        self.assertEqual(1, fixed.count("deleteVehicle _sensor"))

    def test_cluster_second_helper_keeps_existing_cleanup(self):
        source = missile_fixture() + (
            '_bpos = [1,2,3]\n_sensor = "EmptyDetector" createVehicle [0,0,0]\n'
            '_sensor setpos [_bpos select 0, _bpos select 1];\n'
            '_result = getpos _sensor select 2;\n; existing cleanup\ndeleteVehicle _sensor\n'
        )
        path = "lobo_a4/scripts/firemissiles_a2.sqs"
        fixed = scripts._missile(source, path)
        self.assertEqual(2, fixed.count("deleteVehicle _sensor"))
        self.assertEqual(fixed, scripts._missile(fixed, path))

    def test_unexpected_helper_lifetime_is_rejected(self):
        for extra in ('? true: goto "Exit"\n', '~0.1\n', '_other = _sensor\n'):
            source = missile_fixture().replace("_result =", extra + "_result =")
            with self.subTest(extra=extra), self.assertRaises(ValueError):
                scripts._missile(source, "lobof15/scr/fire_e.sqs")
        with self.assertRaises(ValueError):
            scripts._missile(missile_fixture() + "_later = _sensor\n", "lobof15/scr/fire_e.sqs")

    def test_numbering_has_defined_vehicle_and_reachable_digits(self):
        fixed = scripts._number(number_fixture())
        self.assertEqual([], scripts.script_findings(fixed))
        self.assertNotIn("_unit ", fixed)
        self.assertNotIn("_this select 1", fixed)
        self.assertNotIn("], goto", fixed)
        for digit in (2, 3, 4):
            self.assertEqual(10, fixed.count(f"setObjectTexture [{digit},"))
        self.assertEqual(fixed, scripts._number(fixed))

    def test_nonowner_fire_path_has_valid_anchor_without_moving_wreck(self):
        fixed = scripts._fire(fire_fixture())
        self.assertLess(fixed.index("_glow = _object"), fixed.index('?! (local _object):goto "sound"'))
        after_sound = fixed.split("#sound\n", 1)[1]
        for line in after_sound.splitlines():
            if "_glow setpos" in line or "deletevehicle _glow" in line:
                self.assertTrue(line.startswith("? _glow != _object && local _glow:"))
        self.assertIn('#client\n? !(local _object): goto "fire"', fixed)
        self.assertEqual([], scripts.script_findings(fixed))
        self.assertEqual(fixed, scripts._fire(fixed))

    def test_burn_query_covers_remote_victims_without_double_hitting_player(self):
        fixed = scripts._fire(fire_fixture())
        self.assertLess(fixed.index("_AI ="), fixed.index("call _AI"))
        self.assertIn('nearestObjects [_this select 0, ["Man"], _this select 1]', fixed)
        self.assertIn("alive _x && vehicle _x == _x && _x != player", fixed)
        self.assertNotIn("local _x", fixed)
        self.assertIn('? player != (vehicle player): goTo "here"', fixed)
        self.assertIn("? local player && alive player && player distance _object", fixed)
        self.assertIn('#here\n? _ouchAI == 1: goto "fire"', fixed)

    def test_classic_profile_is_explicit_bounded_and_switchable(self):
        original = fire_fixture()
        classic = scripts._fire(original, "classic")
        ud = scripts._fire(original, "ud")
        self.assertNotIn("nearestObjects", classic)
        self.assertNotIn("createTrigger", classic)
        self.assertIn("(units group player) + (units group _object)", classic)
        self.assertIn("_x distance _object < (_this select 1)", classic)
        self.assertNotIn("distance (_this select 0)", classic)
        self.assertIn("!(_x in _units)", classic)
        self.assertEqual([], scripts.script_findings(classic))
        self.assertEqual(classic, scripts._fire(classic, "classic"))
        self.assertEqual(classic, scripts._fire(ud, "classic"))
        self.assertEqual(ud, scripts._fire(classic, "ud"))
        with self.assertRaises(ValueError):
            scripts._fire(original, "unknown")
        with self.assertRaises(ValueError):
            scripts._fire(ud + "\n" + scripts.AI_QUERY_CLASSIC, "classic")
        legacy = classic.replace(scripts.AI_QUERY_CLASSIC, scripts.AI_QUERY_CLASSIC_LEGACY)
        with self.assertRaises(ValueError):
            scripts._fire(legacy + "\n" + scripts.AI_QUERY_CLASSIC, "classic")

    def test_legacy_classic_query_is_switched_in_place(self):
        # A copy prepared with the earlier Classic query (Object distance Array,
        # rejected by the official Remaster exe) is upgraded, not duplicated.
        original = fire_fixture()
        classic = scripts._fire(original, "classic")
        ud = scripts._fire(original, "ud")
        legacy = classic.replace(scripts.AI_QUERY_CLASSIC, scripts.AI_QUERY_CLASSIC_LEGACY)
        self.assertNotEqual(classic, legacy)
        self.assertEqual(classic, scripts._fire(legacy, "classic"))
        self.assertEqual(ud, scripts._fire(legacy, "ud"))
        self.assertEqual(1, scripts._fire(legacy, "classic").count("_AI ="))

    def test_classic_query_uses_only_ofp_199_object_forms(self):
        # The classic literal names only OFP 1.85+ core commands; distance has
        # object operands on both sides (_x and the caller's wreck _object).
        query = scripts.AI_QUERY_CLASSIC
        code = re.sub(r'"[^"]*"', '""', query)
        words = {w for w in re.findall(r"\b[A-Za-z_]\w*\b", code) if not w.startswith("_")}
        self.assertEqual({"private", "units", "group", "player", "alive", "vehicle", "distance",
                          "in", "if", "then", "forEach", "select"}, words)
        self.assertEqual(["_x distance _object"], re.findall(r"\S+ distance \S+", query))
        self.assertNotIn("nearestObjects", query)
        # UD keeps its nearestObjects query byte for byte.
        self.assertEqual(
            '_AI = {private ["_near", "_units"]; '
            '_near = nearestObjects [_this select 0, ["Man"], _this select 1]; '
            '_units = []; {if (alive _x && vehicle _x == _x && _x != player) then '
            '{_units = _units + [_x]}} forEach _near; _units}', scripts.AI_QUERY)

    def test_display_repair_preserves_asset_identifiers(self):
        source = 'model="LoBo_SA9missle_F.p3d";\ndisplayNameMagazine="Hawk Missle";'
        fixed = scripts._display(source, "lobosa6/config.cpp")
        self.assertIn('model="LoBo_SA9missle_F.p3d";', fixed)
        self.assertIn('displayNameMagazine="Hawk Missile";', fixed)
        self.assertEqual(fixed, scripts._display(fixed, "lobosa6/config.cpp"))

    def test_unknown_and_ambiguous_source_is_rejected(self):
        with self.assertRaises(ValueError):
            scripts._number(number_fixture().replace("_unit setObjectTexture", "_other setObjectTexture", 1))
        with self.assertRaises(ValueError):
            scripts._fire(fire_fixture().replace("call _AI", "call _custom").replace("_object = _this select 0", "_object = objNull"))
        with self.assertRaisesRegex(ValueError, "Missing required"):
            scripts.repair({})
        with self.assertRaises(ValueError):
            scripts._display('displayNameMagazine="Hawk Missle";\ndisplayNameMagazine="Hawk Missle";', "lobosa6/config.cpp")

    def test_linter_catches_labels_and_unassigned_locals_without_comment_noise(self):
        findings = scripts.script_findings(
            '; goto "ignored" _comment\n_one = _this select 0\n'
            'goto "missing"\n#same\n#SAME\n_bad setpos [0,0,0]\n'
            '_asset = "folder/_not_a_variable.paa"\n'
        )
        self.assertEqual([
            "missing label: missing",
            "duplicate label: same",
            "never-assigned local: _bad",
        ], findings)
        self.assertEqual([], scripts.script_findings('goto "END"\n#end\nexit\n'))

    def test_repair_preserves_windows_newlines_and_returns_changes_only(self):
        source = number_fixture().replace("\n", "\r\n")
        # Exercise the dispatch and newline preservation without requiring actual
        # third-party assets: provide a synthetic member for each manifest path.
        documents = {}
        for path in scripts.MISSILE_PATHS:
            text = missile_fixture(path)
            if path in ("lobo_a4/scripts/firemissiles_a2.sqs", "lobo_a4/scripts/firemissiles_a3.sqs"):
                text += ('_sensor = "EmptyDetector" createVehicle [0,0,0]\n'
                         '_sensor setpos [0,0];\n_result = getpos _sensor select 2;\ndeleteVehicle _sensor\n')
            documents[path] = text
        documents.update({path: fire_fixture() for path in scripts.FIRE_PATHS})
        documents.update({path: source for path in scripts.NUMBER_PATHS})
        documents["lobojordan/config.cpp"] = "\n".join(
            f'displayName="Jordanian Security Assualt {role}";'
            for role in ("Operator", "Team Leader", "Radioman", "Medic", "Breacher", "Sniper")
        )
        documents["lobosa6/config.cpp"] = 'displayNameMagazine="Hawk Missle";'
        documents["loboweapat/config.cpp"] = 'displayNameMagazine="Dragon Missle";'
        documents["loboter/unrelated.sqs"] = "exit\n"
        fixed = scripts.repair(documents)
        self.assertNotIn("loboter/unrelated.sqs", fixed)
        self.assertEqual(62, len(fixed))
        self.assertFalse(re.search(r"(?<!\r)\n", fixed[scripts.NUMBER_PATHS[0]]))
        self.assertEqual({}, scripts.repair(documents | fixed))
        classic = scripts.repair(documents | fixed, compatibility="classic")
        self.assertEqual(set(scripts.FIRE_PATHS), set(classic))
        self.assertEqual({}, scripts.repair(documents | fixed | classic, compatibility="classic"))
        switched_back = scripts.repair(documents | fixed | classic)
        self.assertEqual({key: fixed[key] for key in scripts.FIRE_PATHS}, switched_back)
        # A Classic copy made with the earlier query: only the 16 fire scripts
        # change, to the current Classic bytes.
        legacy = {key: value.replace(scripts.AI_QUERY_CLASSIC, scripts.AI_QUERY_CLASSIC_LEGACY)
                  for key, value in classic.items()}
        self.assertEqual(classic, scripts.repair(documents | fixed | legacy, compatibility="classic"))
        self.assertEqual(switched_back, scripts.repair(documents | fixed | legacy))


if __name__ == "__main__":
    unittest.main()
