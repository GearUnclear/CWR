"""Source contracts for the optional developer portrait-shoot roster.

No generated image or catalogue manifest is a source dependency. Production
face resolution and print formatting are covered by C++ synthetic-input tests.
"""
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "screenshots"))
import portrait_roster  # noqa: E402


class PortraitRoster(unittest.TestCase):
    def test_parser_finds_both_descriptor_files(self):
        for lane, filename in portrait_roster.LANES.items():
            path = Path(portrait_roster.CONFIG_DIR) / filename
            self.assertTrue(path.is_file(), "%s descriptor missing: %s" % (lane, path))
            self.assertTrue(portrait_roster.lane_roster(lane), "%s parsed to no factions" % lane)

    def test_outer_block_is_not_treated_as_a_faction(self):
        # split_factions walks the INNER classes; CfgGuerrillaFactions itself is
        # the container, and counting it would demand portraits for a faction
        # that does not exist.
        names = [name for name, _ in portrait_roster.lane_roster("vanilla")]
        self.assertNotIn("CfgGuerrillaFactions", names)
        self.assertEqual(sorted(names), ["EAST", "GUER", "WEST"])

    def test_every_faction_names_at_least_a_companion(self):
        for lane in portrait_roster.LANES:
            for faction, classes in portrait_roster.lane_roster(lane):
                self.assertTrue(classes, "%s/%s resolved to no Legend bodies" % (lane, faction))

    def test_sibling_config_classes_are_not_factions(self):
        text = '''
        class Before { class NotAFaction {}; };
        class CfgGuerrillaFactions {
            class WEST { companionClass = "SoldierWB"; tiers[] = {"SoldierWB"}; };
            class EAST : WEST { companionClass = "SoldierEB"; };
        };
        class CfgMarkerColors { class ColorGmNeutral {}; };
        '''
        self.assertEqual([name for name, _ in portrait_roster.split_factions(text)], ["WEST", "EAST"])
        self.assertEqual(portrait_roster.split_factions('class CfgMarkerColors { class ColorGmNeutral {}; };'), [])


if __name__ == "__main__":
    unittest.main()
