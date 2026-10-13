"""Synthetic fixtures for the 2026-10-11.4 IL-28 tail turret repair (LoBo issue 13)."""
import hashlib
import inspect
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "lobo"))
from repair import tailgun  # noqa: E402
from repair.scripts import script_findings  # noqa: E402
import repair_lobo as runner  # noqa: E402

KEY = "loboil28/script/tailgun.sqs"
GUARD_OLD = '? count (crew _plane) ==0: goto "crtGun"'
GUARD_NEW = '? isNull (driver _plane): goto "crtGun"'
GUNNER = "_gunner = _tunits select (count(_tunits) -1)"
EMPTY = '? count _tunits == 0: goto "crtGun"'
BAILOUT = ('? (!(driver _plane in _plane) || !(alive driver _plane)) and _exit == 0 '
           'and (position _plane select 2) > 20: goto "loop2"')
EJECT_GUN = ('if (alive gunner _gun) then {_gunner = gunner _gun; _gunner action ["EJECT",_gun];'
             '_para = "ParachuteEast" createVehicle [0,0,0];_para setpos getpos _camera;_gunner moveindriver _para;}')
EJECT_PLANE = 'if (alive gunner _plane) then {gunner _plane action ["EJECT",_plane]}'


def script(guard=GUARD_OLD, gunner=(GUNNER,), bailout=(BAILOUT,), eject_gun=(EJECT_GUN,),
           eject_plane=(EJECT_PLANE,)):
    """A CRLF tail turret script with the authored structure; each tuple is the lines at that site."""
    lines = ["; synthetic tail turret fixture", "_plane = _this select 0",
             '_camera = "camera" camcreate getpos _plane', "_exit = 0", ""]
    lines += list(guard) if isinstance(guard, tuple) else [guard]
    lines += ["", "_pilot = driver _plane", "_pilot_grp = group _pilot",
              '"LoBo_Egypt_Pilot" createunit [getpos _plane, _pilot_grp,"", 0.7, "PRIVATE"]',
              "_tunits = units _pilot_grp", "; the gunner is the last of the group"]
    lines += list(gunner)
    lines += ["", "#crtGun", '_gun = "LoBo_rearturret" createvehicle[0,0,0]', "_gunner moveingunner _gun",
              "", "#MainLoop", "@ (true)", ""]
    lines += list(bailout)
    lines += ['goto "MainLoop"', "", "#loop2", "", "_exit = 1", ""]
    lines += list(eject_gun) + list(eject_plane)
    lines += ['goto "MainLoop"', " ", ""]
    return "\r\n".join(lines)


ORIGINAL = script()
REPAIRED = script(guard=GUARD_NEW, gunner=(EMPTY, GUNNER))


class TailgunTests(unittest.TestCase):
    def test_exact_change_keeps_crlf_and_is_clean_and_idempotent(self):
        changed = tailgun.repair({KEY: ORIGINAL})
        self.assertEqual({KEY: REPAIRED}, changed)
        self.assertEqual(REPAIRED.count("\n"), REPAIRED.count("\r\n"))
        self.assertEqual(ORIGINAL.count("\r\n") + 1, REPAIRED.count("\r\n"))
        self.assertEqual([], script_findings(REPAIRED.replace("\r\n", "\n")))
        self.assertEqual({}, tailgun.repair(changed))
        # Exactly two edits: the guard line, and one line inserted before the select.
        before, after = ORIGINAL.split("\r\n"), REPAIRED.split("\r\n")
        at = before.index(GUNNER)
        self.assertEqual(1, before.count(GUARD_OLD))
        self.assertEqual(before[:at], [GUARD_OLD if line == GUARD_NEW else line for line in after[:at]])
        self.assertEqual(EMPTY, after[at])
        self.assertEqual(before[at:], after[at + 1:])

    def test_partial_states_converge(self):
        tg1_only = script(guard=GUARD_NEW)
        tg2_only = script(gunner=(EMPTY, GUNNER))
        for source in (tg1_only, tg2_only):
            with self.subTest(source=source[150:220]):
                self.assertEqual({KEY: REPAIRED}, tailgun.repair({KEY: source}))
        self.assertEqual({}, tailgun.repair({KEY: REPAIRED}))
        # LF members stay LF.
        lf = ORIGINAL.replace("\r\n", "\n")
        self.assertEqual(REPAIRED.replace("\r\n", "\n"), tailgun.repair({KEY: lf})[KEY])

    def test_unknown_states_are_rejected(self):
        changed_bailout = BAILOUT.replace("> 20", "> 30")
        cases = {
            "missing guard": script(guard=()),
            "duplicate guard": script(guard=(GUARD_OLD, GUARD_OLD)),
            "guard old and new": script(guard=(GUARD_OLD, GUARD_NEW)),
            "spacing variant": script(guard='? count (crew _plane) == 0: goto "crtGun"'),
            "missing select": script(gunner=()),
            "duplicate select": script(gunner=(GUNNER, GUNNER)),
            "duplicate TG-2 guard": script(gunner=(EMPTY, EMPTY, GUNNER)),
            "changed bailout": script(bailout=(changed_bailout,)),
            "missing bailout": script(bailout=()),
            "duplicate bailout": script(bailout=(BAILOUT, BAILOUT)),
            "changed turret eject": script(eject_gun=(EJECT_GUN.replace("ParachuteEast", "ParachuteWest"),)),
            "missing turret eject": script(eject_gun=()),
            "duplicate turret eject": script(eject_gun=(EJECT_GUN, EJECT_GUN)),
            "changed plane eject": script(eject_plane=(EJECT_PLANE.replace('"EJECT"', '"GETOUT"'),)),
            "missing plane eject": script(eject_plane=()),
            "duplicate plane eject": script(eject_plane=(EJECT_PLANE, EJECT_PLANE)),
        }
        for name, source in cases.items():
            with self.subTest(case=name), self.assertRaises(ValueError):
                tailgun.repair({KEY: source})
        with self.assertRaisesRegex(ValueError, "Missing members"):
            tailgun.repair({})

    def test_tables_and_profile_neutrality(self):
        self.assertEqual(("loboil28",), tailgun.ADDONS)
        self.assertEqual(["documents"], list(inspect.signature(tailgun.repair).parameters))
        self.assertEqual((BAILOUT, EJECT_GUN, EJECT_PLANE), tailgun.PINNED)

    def test_ud_and_classic_plans_give_the_same_bytes(self):
        payload = ORIGINAL.encode("latin1")
        header = b"".join(name + b"\0" + struct.pack("<5I", 0, 0, 0, 0, len(data))
                          for name, data in ((b"script\\tailgun.sqs", payload),))
        body = header + bytes(21) + payload
        with tempfile.TemporaryDirectory(prefix="lobo-tailgun-test-") as tmp:
            addons = Path(tmp) / "addons"
            addons.mkdir()
            (addons / "LoBoIL28.pbo").write_bytes(body + b"\0" + hashlib.sha1(body).digest())
            with patch.object(runner, "modules", return_value=[tailgun]):
                ud = runner.plan(Path(tmp), None, compatibility="ud")[2]
                classic = runner.plan(Path(tmp), None, compatibility="classic")[2]
        self.assertEqual({KEY: REPAIRED.encode("latin1")}, ud)
        self.assertEqual(ud, classic)


if __name__ == "__main__":
    unittest.main()
