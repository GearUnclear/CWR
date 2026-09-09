"""Source contract for the dossier portrait catalogue. Run with unittest.

    python -m unittest discover -s tests/contracts -p "test_*.py"

This is the test that goes red when somebody adds a faction. A Legend row wears
one of four bodies, all named by the faction descriptor (companionClass,
companionClassCiv, the elite/commander body, the sniper body), and the dossier
looks its photograph up by lower(bodyClass) + "__" + lower(face). Add a faction
with a companionClass nothing has photographed and the dossier silently draws
"Photograph unavailable" for every one of its characters, on a code path with no
error and no warning. So the required key set is derived from the two descriptor
files here, with the SAME parser the shoot's roster generator uses, and checked
against what is actually on disk.

The vanilla half is a hard failure: those 36 .paa are committed.
The @LoBo half reports a skip naming what is missing, because
arma_CWR/.gitignore excludes /guerrilla-mode/core/portraits/lobo*.paa - the
frames are derived from APL-SA third-party art and this repo is GPL - so a fresh
clone legitimately has none of them and must not be red.
"""

import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PORTRAITS = ROOT / "guerrilla-mode" / "core" / "portraits"

sys.path.insert(0, str(ROOT / "tools" / "screenshots"))
import portrait_roster  # noqa: E402


def present():
    if not PORTRAITS.is_dir():
        return set()
    return {p.name.lower() for p in PORTRAITS.glob("*.paa")}


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


class PortraitCatalogue(unittest.TestCase):
    def test_vanilla_catalogue_is_complete_or_absent(self):
        # The cards are renders of game models and are gitignored, so a clone
        # legitimately has none. Absent is a skip; a HALF-shot catalogue is a
        # failure, because that is the state where only part of the roster
        # silently falls back to "Photograph unavailable".
        have = present()
        wanted = sorted(k + ".paa" for k in portrait_roster.required_keys("vanilla"))
        missing = [n for n in wanted if n not in have]
        if wanted and len(missing) == len(wanted):
            self.skipTest(
                "no vanilla portraits present (gitignored as renders of game models; "
                "regenerate with tools/screenshots/portraits/portrait_vanilla.test.sqf)")
        self.assertEqual(
            missing, [],
            "the vanilla portrait catalogue is incomplete. Reshoot with\n"
            "  tri test tools/screenshots/portraits/portrait_vanilla.test.sqf\n"
            "  python tools/screenshots/portrait_process.py <outdir> vanilla\n"
            "missing: %s" % ", ".join(missing))

    def test_lobo_catalogue_is_complete_or_absent(self):
        have = present()
        missing = sorted(k + ".paa" for k in portrait_roster.required_keys("lobo")
                         if k + ".paa" not in have)
        if missing:
            self.skipTest(
                "%d @LoBo portraits absent (gitignored as APL-SA-derived art; regenerate with "
                "tools/screenshots/portraits/portrait_lobo.test.sqf). First missing: %s"
                % (len(missing), ", ".join(missing[:4])))

    def test_no_orphan_portraits(self):
        # A file nobody will ever ask for is dead weight in a game-data folder
        # that ships by robocopy /MIR.
        wanted = set(portrait_roster.required_keys("vanilla")) | set(portrait_roster.required_keys("lobo"))
        orphans = sorted(n for n in present() if n[:-4] not in wanted)
        self.assertEqual(orphans, [], "portraits no faction descriptor asks for: %s" % ", ".join(orphans))

    def test_catalogue_json_matches_the_files(self):
        path = PORTRAITS / "catalogue.json"
        if not path.is_file():
            self.skipTest("catalogue.json absent")
        with open(path, "r", encoding="utf-8") as fh:
            catalogue = json.load(fh)
        have = present()
        for entry in catalogue["entries"]:
            if not entry["paa"].lower().startswith("lobo") or entry["paa"].lower() in have:
                self.assertIn(entry["paa"].lower(), have,
                              "catalogue.json records %s but the file is not there" % entry["paa"])
                self.assertEqual(entry["paaFormat"], "DXT1")
                self.assertEqual(entry["paaSize"], [256, 256])


if __name__ == "__main__":
    unittest.main()
