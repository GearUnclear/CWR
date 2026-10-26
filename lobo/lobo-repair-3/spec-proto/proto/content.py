"""SPEC PROTOTYPE: small content defects from the audit-gap pass (AG-5, AG-6,
AG-11) and the bwm_boot3 crew class (CI-6, same-length because the archive has
a StuffPBO trailer).
"""
from .common import apply_rules, p

BWM = "bwm_boot3/config.cpp"
RULES = {
    # AG-5: 'armour' is not a key the engine reads; hangars fell back to Building armor=150.
    "rksl-hangers/config.cpp": (("line", "\t\tarmour=1000;", "\t\tarmor=1000;", 3),
                                ("line", "\t\tarmour=2000;", "\t\tarmor=2000;", 6)),
    # AG-6: model and icon basenames swapped (neither file exists); restore the missing ';'.
    "tmyk_bridges/config.cpp": (
        ("line", '        model = "' + p("TMYK_Bridges", "TMYK_ViaductCenter.p3d") + '"',
         '        model = "' + p("TMYK_Bridges", "TMYK_Viaduct.p3d") + '";', 1),
        ("line", '        icon = "' + p("TMYK_Bridges", "icons", "TMYK_Viaduct.pac") + '";',
         '        icon = "' + p("TMYK_Bridges", "icons", "TMYK_ViaductCenter.pac") + '";', 1)),
    # AG-11: LoBo_para.p3d has an unsupported MLOD version; use the inherited vanilla chute.
    "lobois/config.cpp": (("line", '\t\tmodel="' + p("LoBoIs", "LoBo_para.p3d") + '";', '\t\tmodel="para";', 1),),
    # CI-6: undefined crew; SoldierWB is what BoatW gives. Same length (StuffPBO archive).
    BWM: (("line", '\t\tcrew="BWMOD_Soldier";', '\t\tcrew="SoldierWB";    ', 1),),
}
SAME_LENGTH = (BWM,)
ADDONS = tuple(sorted({path.split("/")[0] for path in RULES}))


def repair(documents):
    changed = apply_rules(documents, RULES)
    for key in SAME_LENGTH:
        if key in changed and len(changed[key]) != len(documents[key]):
            raise ValueError(f"{key}: StuffPBO member must keep its length")
    return changed
