#!/usr/bin/env python3
"""Offline behavioral checks for issue #69; fixtures are synthetic, not mod data."""
from pathlib import Path
import copy
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "lobo"))
from repair import ammo
from repair.config import class_block, scalar


def config(ammunition, weapons=""):
    return f"class CfgAmmo {{\n{ammunition}\n}};\nclass CfgWeapons {{\n{weapons}\n}};\n"


def originals():
    # Deliberately different formatting and unrelated nested members establish
    # that the repair follows class/property structure, not global substitutions.
    bullets = """
class LoBo_M240Coax { hit=6; };
class LoBo_50cal { hit=19; };
class loBo_DTMCoax { hit=7; indirectHit=2; indirectHitRange=1; };
class loBo_PKCoax { hit=8; indirectHit=2; indirectHitRange=1; };
class loBo_DShK { hit=19; indirectHit=5; indirectHitRange=1; };
class loBo_KPVT { hit=20; indirectHit=5; indirectHitRange=1; };
class LoBo_25AP { hit=75; indirectHit=5; indirectHitRange=1; };
class LoBo_BMP2_30APFSDS { hit=70; indirectHit=0; indirectHitRange=1; };
class LoBo_zpu4Bullet { hit=9; indirectHit=2; indirectHitRange=0.2; explosive=0; };
class LoBo_M61V_20mm { hit=60; indirectHit=30; indirectHitRange=2; };
class LoBo_M40A1_AIM { explosive=true; };
class LoBo_105Sabot1 { hit=450; };
class LoBo_60HVMSHE { hit=255; indirectHit=15; indirectHitRange=30;
    class Unrelated { indirectHitRange=900; }; };
class LoBo_C4 { hit=1000; indirectHit=900; indirectHitRange=50; };
"""
    ammo_weapons = '''
class LoBo_105Sabot2 { ammo="LoBo_105Sabot2"; count=30; };
class lobo_122D25AP { shortNameMagazine="1225mm AP"; displayName="122mm AP"; };
'''
    ranges = "minRange=1; minRangeProbab=.8; midRange=50; midRangeProbab=.35; maxRange=109; maxRangeProbab=.08;"
    weapon_ammo = "\n".join(
        f"class {name} {{ {ranges} hit=7; audibleFire={noise}; }};"
        for name, noise in (("LoBo762Bullet", 20), ("LoBo556Bullet", 15), ("LoBo556Bullet_SD", .001))
    )
    weapon_classes = '''
class LoBoM249Mag { ammo="LoBo556Bullet"; };
class LoBoNegevMag { ammo="LoBo556Bullet"; };
class LoBoNegevSDMag { ammo="LoBo556Bullet"; };
class RifleExample { class Single { ammo="LoBo556Bullet"; }; };
class LoBoMUZIbase { magazines[]={LoBoMUZImag, LoBoMUZIDDmag}; };
class LoBoMUZIHDmag { count=20; };
class LoBo_m590base { displayNameMagazine="12 Guage Shells"; shortNameMagazine="12 Guage Shells"; };
class LoBo_SPAS12base { displayNameMagazine="12 Guage Shells"; shortNameMagazine="12 Guage Shells"; };
class LoBo_r870base { displayNameMagazine="12 Guage Shells"; shortNameMagazine="12 Guage Shells"; };
// Intentional historical identifier; not a display-string typo.
class LoBo12GuageShell { keep=109; };
'''
    return {
        "loboammo/config.cpp": config(bullets, ammo_weapons),
        "loboweapons/config.cpp": config(weapon_ammo, weapon_classes),
        "lobo_airammo/config.cpp": config('''
class LoBo_MK82 { hit=1000; indirectHit=500; indirectHitRange=15; };
class LoBo_MK82_LDGB { hit=1000; indirectHit=500; indirectHitRange=15; };
class LoBo_GBU12 { hit=500; indirectHit=150; indirectHitRange=10; laserLock=true; cost=2000; };
'''),
        "loboter/config.cpp": config('''
class LoBo_Carbomb { hit=1000; indirectHit=500; indirectHitRange=50; };
class loBo_DShK_Ter { hit=19; indirectHit=5; indirectHitRange=1; };
'''),
        "loboart/config.cpp": config('''
class LoBo_120MortarShellHE { hit=230; indirectHit=45; indirectHitRange=8; };
class LoBo_M160MortarShell { hit=260; indirectHit=45; indirectHitRange=8; };
'''),
    }


def number(document, name, key):
    return float(scalar(document, ("CfgAmmo", name), key))


def probability(document, name, distance):
    points = [(number(document, name, key), number(document, name, key+"Probab"))
              for key in ("minRange", "midRange", "maxRange")]
    if distance < points[0][0] or distance > points[-1][0]:
        return 0
    left, right = points[:2] if distance < points[1][0] else points[1:]
    fraction = (distance-left[0])/(right[0]-left[0])
    return left[1] + fraction*(right[1]-left[1])


class AmmoRepairTests(unittest.TestCase):
    def setUp(self):
        self.original = originals()
        self.updated = self.original | ammo.repair(self.original)

    def test_missing_m833_projectile_resolves_without_m900_alias(self):
        doc = self.updated["loboammo/config.cpp"]
        link = scalar(doc, ("CfgWeapons", "LoBo_105Sabot2"), "ammo").strip('"')
        class_block(doc, ("CfgAmmo", link))
        self.assertEqual(number(doc, link, "hit"), 400)
        self.assertLess(number(doc, link, "hit"), number(doc, "LoBo_105Sabot1", "hit"))
        self.assertEqual(number(doc, link, "indirectHit"), 0)
        self.assertIn("class LoBo_105Sabot2: Shell105", doc)

    def test_mg_ranges_and_rifle_roles_diverge_intentionally(self):
        old, new = (d["loboweapons/config.cpp"] for d in (self.original, self.updated))
        self.assertEqual(probability(old, "LoBo762Bullet", 300), 0)
        self.assertAlmostEqual(probability(new, "LoBo762Bullet", 300), .24285714285714288)
        self.assertAlmostEqual(probability(new, "LoBo556MGBullet", 300), .24285714285714288)
        self.assertEqual(probability(new, "LoBo556MGBullet", 451), 0)
        self.assertEqual(probability(new, "LoBo556Bullet", 300), 0)
        self.assertEqual(number(new, "LoBo556Bullet", "maxRange"), 250)
        self.assertEqual(number(new, "LoBo556Bullet_SD", "maxRange"), 250)
        self.assertEqual(number(new, "LoBo556Bullet_SD", "audibleFire"), .001)
        for name in ("LoBoM249Mag", "LoBoNegevMag"):
            self.assertEqual(scalar(new, ("CfgWeapons", name), "ammo"), '"LoBo556MGBullet"')
        self.assertEqual(scalar(new, ("CfgWeapons", "LoBoNegevSDMag"), "ammo"), '"LoBo556Bullet_SD"')
        self.assertEqual(scalar(new, ("CfgWeapons", "RifleExample", "Single"), "ammo"), '"LoBo556Bullet"')

    def test_ball_ap_bullets_have_no_splash_but_he_keeps_it(self):
        doc = self.updated["loboammo/config.cpp"]
        for name in ("LoBo_M240Coax", "LoBo_50cal", "loBo_DTMCoax", "loBo_PKCoax",
                     "loBo_DShK", "loBo_KPVT", "LoBo_25AP", "LoBo_BMP2_30APFSDS", "LoBo_zpu4Bullet"):
            for key in ("indirectHit", "indirectHitRange"):
                self.assertEqual(number(doc, name, key), 0, (name, key))
        self.assertEqual(number(doc, "LoBo_M61V_20mm", "indirectHit"), 30)
        self.assertEqual(scalar(doc, ("CfgAmmo", "LoBo_M40A1_AIM"), "explosive"), "true")
        self.assertEqual(number(doc, "loBo_KPVT", "hit"), 20)
        self.assertEqual(number(self.updated["loboter/config.cpp"], "loBo_DShK_Ter", "indirectHit"), 0)

    def test_blast_policy_preserves_contact_strength_and_mortar_baseline(self):
        doc = self.updated["loboammo/config.cpp"]
        self.assertEqual(number(doc, "LoBo_60HVMSHE", "indirectHitRange"), 4)
        self.assertEqual(number(doc, "LoBo_60HVMSHE", "hit"), 255)
        self.assertEqual(number(doc, "LoBo_60HVMSHE", "indirectHit"), 15)
        self.assertEqual(scalar(doc, ("CfgAmmo", "LoBo_60HVMSHE", "Unrelated"), "indirectHitRange"), "900")
        self.assertEqual(number(doc, "LoBo_C4", "indirectHitRange"), 10)
        self.assertEqual(number(doc, "LoBo_C4", "indirectHit"), 900)
        self.assertEqual(self.updated["loboart/config.cpp"], self.original["loboart/config.cpp"])
        self.assertEqual(number(self.updated["loboter/config.cpp"], "LoBo_Carbomb", "indirectHitRange"), 10)

    def test_mk82_warhead_equivalence_preserves_guidance(self):
        doc = self.updated["lobo_airammo/config.cpp"]
        for key in ("hit", "indirectHit", "indirectHitRange"):
            self.assertEqual(number(doc, "LoBo_GBU12", key), number(doc, "LoBo_MK82", key))
            self.assertEqual(number(doc, "LoBo_MK82_LDGB", key), number(doc, "LoBo_MK82", key))
        self.assertEqual(scalar(doc, ("CfgAmmo", "LoBo_GBU12"), "laserLock"), "true")
        self.assertEqual(number(doc, "LoBo_GBU12", "cost"), 2000)

    def test_magazine_and_display_typos_preserve_stable_identifiers(self):
        doc = self.updated["loboweapons/config.cpp"]
        self.assertNotIn("LoBoMUZIDDmag", doc)
        self.assertEqual(doc.count('"12 Gauge Shells"'), 6)
        class_block(doc, ("CfgWeapons", "LoBo12GuageShell"))
        self.assertEqual(scalar(self.updated["loboammo/config.cpp"],
                               ("CfgWeapons", "lobo_122D25AP"), "shortNameMagazine"), '"122mm AP"')

    def test_idempotence_and_input_immutability(self):
        self.assertEqual(self.original, originals())
        self.assertEqual(ammo.repair(self.updated), {})
        crlf = {k: v.replace("\n", "\r\n") for k, v in self.original.items()}
        repaired = crlf | ammo.repair(crlf)
        self.assertEqual(ammo.repair(repaired), {})
        for doc in repaired.values():
            self.assertNotIn("\n", doc.replace("\r\n", ""))

    def test_unknown_modified_values_fail_before_any_mutation(self):
        cases = [
            ("loboammo/config.cpp", "indirectHitRange=30", "indirectHitRange=31"),
            ("loboweapons/config.cpp", "maxRange=109", "maxRange=110"),
            ("lobo_airammo/config.cpp", "hit=1000", "hit=999"),
            ("loboart/config.cpp", "indirectHit=45", "indirectHit=46"),
        ]
        for key, old, new in cases:
            documents = copy.deepcopy(self.original)
            documents[key] = documents[key].replace(old, new, 1)
            before = copy.deepcopy(documents)
            with self.assertRaises(ValueError):
                ammo.repair(documents)
            self.assertEqual(documents, before)
        documents = self.updated.copy()
        documents["loboammo/config.cpp"] = documents["loboammo/config.cpp"].replace("hit=400", "hit=401")
        with self.assertRaises(ValueError):
            ammo.repair(documents)

    def test_missing_sources_and_duplicate_classes_fail_closed(self):
        documents = self.original.copy()
        del documents["loboart/config.cpp"]
        with self.assertRaises(ValueError):
            ammo.repair(documents)
        documents = self.original.copy()
        documents["loboammo/config.cpp"] = documents["loboammo/config.cpp"].replace(
            "class LoBo_C4", "class LoBo_C4 {}; class LoBo_C4", 1)
        with self.assertRaises(ValueError):
            ammo.repair(documents)


class BlastPolicyTests(unittest.TestCase):
    def test_plateau_inverse_fourth_curve_cover_and_armor(self):
        self.assertEqual(ammo.blast_sample(15, 4, 4), 15)
        self.assertAlmostEqual(ammo.blast_sample(15, 4, 8), 15/16)
        self.assertAlmostEqual(ammo.blast_sample(15, 4, 10), .384)
        self.assertAlmostEqual(ammo.blast_sample(900, 10, 20), 56.25)
        self.assertAlmostEqual(ammo.blast_sample(900, 10, 20, unshielded=.25, armor=10), 1.40625)
        self.assertEqual(ammo.blast_sample(900, 10, 1, unshielded=0), 0)
        self.assertEqual(ammo.blast_sample(0, 0, 1), 0)

    def test_comparisons_are_bounded_reductions_and_keep_mortars(self):
        rows = ammo.blast_comparisons()
        self.assertEqual(len(rows), 144)
        for row in rows:
            self.assertGreaterEqual(row["after"], 0)
            self.assertLessEqual(row["after"], row["before"])
            self.assertLessEqual(row["search_radius_after"], 40)
            if "mortar" in row["family"]:
                self.assertEqual(row["before"], row["after"])
            if row["distance"] == 0:
                self.assertEqual(row["before"], row["after"])

    def test_invalid_scenarios_fail(self):
        for args in ((1, 1, -1), (1, -1, 1), (-1, 1, 1), (1, 1, float("nan"))):
            with self.assertRaises(ValueError):
                ammo.blast_sample(*args)
        with self.assertRaises(ValueError):
            ammo.blast_sample(1, 1, 1, armor=0)
        with self.assertRaises(ValueError):
            ammo.blast_sample(1, 1, 1, unshielded=1.01)


if __name__ == "__main__":
    unittest.main()
