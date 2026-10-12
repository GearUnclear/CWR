"""Undefined config identifiers that read as 0 at runtime (CI-1..5, 7, 8, 9).

No #define in the file, not in the CONFIG.BIN variable table, or quoted so a
macro never expands: each value token becomes the literal the standard BIS
define gives, which behaves identically in UD, the official Remaster exe and
Classic 1.99. Exact whole-line rules only. A tuple of originals means several
spellings that all become one line; the count is then the total number of
sites. The fifth field counts lines that were already authored correctly.
"""
from .rules import apply_rules

BS = chr(92)
T2 = "\t\t"
T3 = "\t\t\t"
SP16 = " " * 16
CANSEE_TYPO = "CanseeCompass+CanSeeRadar+CanSeeEye+CanSeeEar;"
# Macro-body continuation lines must keep their trailing space and backslash.
SPEED_MACRO = "\t\t\t\tspeed=SPEED_STATIC; " + BS
SPEED_MACRO_FIXED = "\t\t\t\tspeed=1e10; " + BS
VAIR = T2 + "type=VAir;"
VAIR_Q = T2 + 'type="VAir";'
TYPE2 = T2 + "type=2;"
TYPE1 = T2 + "type=1;"
RULES = {
    # CI-1 crew perception: CanSeeRadar 1, Eye 2, Optics 4, Ear 8, Compass 16, All 31.
    # CI-3 armor kind VArmor = 1. CI-4 crew move states SPEED_STATIC 1e10, RestDuty -1.
    "coc_arty/config.cpp": (
        ("line", T2 + "driverCanSee = " + CANSEE_TYPO, T2 + "driverCanSee = 27;", 1),
        ("line", T2 + "drivercansee = " + CANSEE_TYPO, T2 + "drivercansee = 27;", 1),
        ("line", T2 + "type=Varmor;", TYPE1, 1),
        ("line", SPEED_MACRO, SPEED_MACRO_FIXED, 2),
    ),
    "loboart/config.cpp": (
        ("line", T2 + "driverCanSee = " + CANSEE_TYPO, T2 + "driverCanSee = 27;", 1),
        ("line", T2 + "drivercansee = " + CANSEE_TYPO, T2 + "drivercansee = 27;", 2),
        ("line", T2 + "type=Varmor;", TYPE1, 2),
        ("line", T3 + 'speed="SPEED_STATIC";', T3 + "speed=1e10;", 3),
        ("line", T3 + 'duty="RestDuty";', T3 + "duty=-1;", 2),
        # CI-5: WeaponHardMounted 65536; 0 hid the mortars from `weapons`.
        ("line", T2 + "weaponType=WeaponHardMounted;", T2 + "weaponType=65536;", 2),
    ),
    "lobohizballcar/config.cpp": (
        ("line", T2 + "gunnerCanSee = CanSeeOptics+CanSeeEar+CanSeeCompass+CanSeeRadar;", T2 + "gunnerCanSee = 29;", 1),
        ("line", T2 + "driverCanSee = CanSeeOptics+CanSeeEar+CanSeeCompass+CanSeeRadar;", T2 + "driverCanSee = 29;", 1),
    ),
    "lobo_110/config.cpp": (("line", T2 + "driverCanSee=CanSeeEye+CanSeeEar;", T2 + "driverCanSee=10;", 1),),
    "lobo_btr60/config.cpp": (
        ("line", SP16 + "driverCanSee=CanSeeEye+CanSeeOptics+CanSeeCompass;", SP16 + "driverCanSee=22;", 1),
        ("line", SP16 + "gunnerCanSee=CanSeeEye+CanSeeOptics+CanSeeCompass;", SP16 + "gunnerCanSee=22;", 1),
    ),
    "loboidfhelo/config.cpp": (
        ("line", T2 + "gunnerCanSee = CanSeeAll;", T2 + "gunnerCanSee = 31;", 1),
        ("line", T2 + "gunnerCanSee = CanSeeEye+CanSeeCompass+CanSeeOptics;", T2 + "gunnerCanSee = 22;", 1),
        ("line", T2 + "driverCanSee = CanSeeEye+CanSeeCompass;", T2 + "driverCanSee = 18;", 1),
        ("line", (VAIR, VAIR_Q), TYPE2, 8, 2),
        ("line", SPEED_MACRO, SPEED_MACRO_FIXED, 5),
        ("line", T3 + "duty = RestDuty;", T3 + "duty = -1;", 1),
    ),
    "lobojorhelo/config.cpp": (
        ("line", T2 + "gunnerCanSee = CanSeeAll;", T2 + "gunnerCanSee = 31;", 1),
        ("line", VAIR, TYPE2, 1, 2),
    ),
    "lobomerkava2/config.cpp": (("line", T2 + 'gunnerCanSee="CanSeeAll";', T2 + "gunnerCanSee=31;", 1),),
    "lobomerkava4/config.cpp": (("line", T2 + 'gunnerCanSee="CanSeeAll";', T2 + "gunnerCanSee=31;", 1),),
    # CI-2 aircraft kind VAir = 2, the value Plane/Helicopter would have given.
    "bde_mir3/config.cpp": (("line", T2 + "type \t\t\t\t= VAir;", T2 + "type \t\t\t\t= 2;", 1),),
    "csjairlines/config.cpp": (("line", VAIR, TYPE2, 1),),
    "csjcitation/config.cpp": (("line", VAIR, TYPE2, 1),),
    "loboc130/config.cpp": (("line", VAIR, TYPE2, 1),),
    "loboeghelo/config.cpp": (("line", (VAIR, VAIR_Q), TYPE2, 6),),
    "lobof15/config.cpp": (("line", VAIR_Q, TYPE2, 9),),
    "lobof16/config.cpp": (("line", VAIR, TYPE2, 13),),
    "loboil28/config.cpp": (("line", VAIR, TYPE2, 5),),
    "lobomig/config.cpp": (("line", (VAIR, VAIR_Q), TYPE2, 10),),
    "lobo_a4/config.cpp": (("line", VAIR, TYPE2, 1),),
    "lobo_f4/config.cpp": (("line", VAIR, TYPE2, 5),),
    "lobo_kfir/config.cpp": (("line", T2 + "type \t= VAir;", T2 + "type \t= 2;", 1),),
    "lobof5/config.cpp": (("line", VAIR, TYPE2, 3),),
    # CI-3 armor kind VArmor = 1, the value Tank/M1Abrams/Ship would have given.
    "lobocen/config.cpp": (("line", T2 + "type=VArmor;", TYPE1, 1),),
    "lobochallen/config.cpp": (("line", T2 + "type=VArmor;", TYPE1, 1),),
    "lobo_fab/config.cpp": (("line", T2 + "type=VArmor;", TYPE1, 1, 4),
                            # CI-9: 'o' for 0, no semicolon as authored.
                            ("line", T2 + "hasCommander=o", T2 + "hasCommander=0", 1, 2)),
    "map_oiladdon_script/config.cpp": (("line", T2 + 'type="VArmor";', TYPE1, 2),),
    # CI-4 crew move states.
    "loboegapc/config.cpp": (("line", T3 + 'speed="SPEED_STATIC";', T3 + "speed=1e10;", 33),
                             ("line", T3 + 'duty="RestDuty";', T3 + "duty=-1;", 32)),
    # CI-7 torpedo launcher lock, LockYes = 2.
    "coc_torpedoes/config.cpp": (("line", T2 + "canLock=LockYes;", T2 + "canLock=2;", 1),),
    # CI-8 ZU-23 gunner HUD: the token 'Helicopter,' evaluated to nil, read as 0
    # (UnitInfoSoldier), the panel players always saw and the vanilla static
    # weapon convention (M2StaticMG). Keep it; only the warning goes. The first
    # 2026-10-11.3 draft wrote 1 (UnitInfoTank, batch 20261012T034354Z-0532e837
    # on the build machine); that state is accepted and moved to 0 as well.
    "lobo_zu23/config.cpp": (("line", (T2 + "UnitInfoType=Helicopter,", T2 + "UnitInfoType=1;"),
                              T2 + "UnitInfoType=0;", 1),),
}
ADDONS = tuple(sorted({path.split("/")[0] for path in RULES}))


def repair(documents: dict[str, str]) -> dict[str, str]:
    return apply_rules(documents, RULES)
