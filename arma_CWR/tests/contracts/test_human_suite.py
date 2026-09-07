"""Source contract for the human suite. Run with Python's unittest runner.

This checks production reuse and observer ownership, not player acceptance.
Runtime action/ledger/predicate tests live in integration/scripting.
"""

import json
import re
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MODE = ROOT / "guerrilla-mode"
MISSION = MODE / "mission/Showcase.Abel"
HUMAN = MISSION / "human"


def executable(text):
    """Remove SQS full-line comments and quoted text, preserving code values."""
    text = "\n".join(line for line in text.splitlines() if not line.lstrip().startswith(";"))
    return re.sub(r'"(?:[^"\n]|"")*"', '""', text)


def observer_violations(text):
    code = executable(text)
    bad = []
    for name in re.findall(r"\b([A-Za-z_]\w*)\s*=(?!=)", code):
        if not name.startswith("HT_"):
            bad.append(name)
    for name in re.findall(r"\b([A-Za-z_]\w*)\s+set\s*\[", code):
        if not name.startswith("HT_"):
            bad.append(name)
    # Even invoking a production helper can write state or own an event slot.
    bad += re.findall(r"\b(?:call\s+GM_\w+|gm\w*(?:Set|Register|OnEvent|ForceSpawn|ForceDespawn|Establish|Lock|Release)|setPos|setDammage|setDamage|setSkill|setRank|addRating|addWeapon|addMagazine|createUnit|createVehicle|setCaptive|allowDammage|disableAI|setAccTime|saveGame|loadGame)\b", code, re.I)
    return bad


class HumanSuiteContract(unittest.TestCase):
    def test_actual_campaign_bootstrap_and_data(self):
        active = [line for line in (MISSION / "init.sqs").read_text().splitlines()
                  if line.strip() and not line.lstrip().startswith(";")]
        self.assertEqual(active, ['[] exec "\\gmcore\\init.sqs"'])
        config = (MISSION / "description.ext").read_text(encoding="utf-8-sig")
        self.assertIn('#include "Missions/Guerrilla.Abel/description.ext"', config)
        self.assertNotRegex(config, r"\bclass\s+CfgGuerrilla")
        self.assertFalse((MISSION / "scripts").exists())

    def test_every_core_and_native_system_has_human_cases(self):
        manifest = (HUMAN / "cases.sqs").read_text()
        ids = re.findall(r'HT_CASES = HT_CASES \+ \[\["([a-z_]+)"', manifest)
        self.assertEqual(len(ids), len(set(ids)))
        self.assertGreaterEqual(len(ids), 41)
        coverage = json.loads((HUMAN / "coverage.json").read_text())
        systems = coverage["systems"]
        for path, cases in systems.items():
            self.assertTrue((MODE / path).is_file(), path)
            self.assertTrue(cases, path)
            self.assertLessEqual(set(cases), set(ids), path)
        self.assertEqual({p.name for p in (MODE / "core/scripts").glob("*.sqs")},
                         {Path(p).name for p in systems if p.startswith("core/")})
        native = ROOT / "engine/Poseidon/Game/Guerrilla"
        listed = {Path(p).name for p in systems if p.endswith(".cpp")}
        excluded = coverage["tooling_exclusions"]
        self.assertTrue(all(excluded.values()))
        self.assertFalse(listed & set(excluded))
        self.assertEqual({p.name for p in native.glob("*.cpp")}, listed | set(excluded))

    def test_observers_cannot_drive_or_recreate_gameplay_state(self):
        for path in HUMAN.glob("*.sqs"):
            if path.name == "prepare_roads.sqs":
                continue
            self.assertEqual(observer_violations(path.read_text()), [], str(path))
            targets = re.findall(r'\bexec\s+"([^"]+)"', path.read_text())
            self.assertTrue(all(t.startswith("human/") for t in targets), targets)

    def test_road_fixture_only_seeds_data_and_calls_the_native_service(self):
        text = (HUMAN / "prepare_roads.sqs").read_text()
        self.assertEqual(observer_violations(text), ["gmZoneSet", "gmTrafficForceSpawn"])
        code = executable(text)
        self.assertNotRegex(code, r"\b(?:doMove|commandMove|createVehicle|createUnit|setDammage|setSkill)\b")
        self.assertNotRegex(code, r"\bHT_(?:RESULTS|EVIDENCE|STEP|HISTORY)\s*(?:=|set)")
        self.assertIn('? HT_ACTIVE : exit', text)
        self.assertIn('HT_SETUP_HISTORY = HT_SETUP_HISTORY +', text)

    def test_guard_rejects_bypasses_even_inside_code_values(self):
        for text in ['GM_COMP_XP set [0, 100]', 'HT_fake = {gmResources = 999}',
                     'player setDammage 1', '[] call GM_fnCompDie',
                     'gmZoneSet [0, "owner", "GUER"]', 'gmTrafficForceSpawn []']:
            self.assertTrue(observer_violations(text), text)
        self.assertFalse(observer_violations('HT_hint = "Do not setDammage"; HT_pos = getPos player'))


if __name__ == "__main__":
    unittest.main()
