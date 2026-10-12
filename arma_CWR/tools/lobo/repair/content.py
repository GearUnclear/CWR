"""Small content defects: AG-5, AG-6, AG-11 and the bwm_boot3 crew (CI-6).

bwm_boot3.pbo carries a legacy StuffPBO trailer, so its config keeps its exact
length and the archive is patched in place (see pbo.Archive._in_place).
"""
from .rules import apply_rules, p

BWM = "bwm_boot3/config.cpp"
RULES = {
    # AG-5: the engine reads 'armor', never 'armour'; the hangars fell back to
    # Building armor 150.
    "rksl-hangers/config.cpp": (("line", "\t\tarmour=1000;", "\t\tarmor=1000;", 3),
                                ("line", "\t\tarmour=2000;", "\t\tarmor=2000;", 6)),
    # AG-6: model and icon basenames were swapped (neither file exists) and the
    # model line lacked its ';'.
    "tmyk_bridges/config.cpp": (
        ("line", '        model = "' + p("TMYK_Bridges", "TMYK_ViaductCenter.p3d") + '"',
         '        model = "' + p("TMYK_Bridges", "TMYK_Viaduct.p3d") + '";', 1),
        ("line", '        icon = "' + p("TMYK_Bridges", "icons", "TMYK_Viaduct.pac") + '";',
         '        icon = "' + p("TMYK_Bridges", "icons", "TMYK_ViaductCenter.pac") + '";', 1)),
    # AG-11: LoBo_para.p3d has an MLOD version the engine rejects; use the
    # vanilla ParachuteBase model the class would inherit.
    "lobois/config.cpp": (("line", '\t\tmodel="' + p("LoBoIs", "LoBo_para.p3d") + '";', '\t\tmodel="para";', 1),),
    # CI-6: BWMOD_Soldier is defined nowhere in the core mod; SoldierWB is the
    # crew BoatW gives and the base of BWMod's own BWMOD_Soldier. Padded with
    # four spaces to keep the StuffPBO member length.
    BWM: (("line", '\t\tcrew="BWMOD_Soldier";', '\t\tcrew="SoldierWB";    ', 1),),
}
SAME_LENGTH = (BWM,)
ADDONS = tuple(sorted({path.split("/")[0] for path in RULES}))


def repair(documents: dict[str, str]) -> dict[str, str]:
    changed = apply_rules(documents, RULES)
    for key in SAME_LENGTH:
        if key in changed and len(changed[key]) != len(documents[key]):
            raise ValueError(f"{key}: StuffPBO member must keep its length")
    return changed
