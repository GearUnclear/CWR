"""SPEC PROTOTYPE: LoBo issues 4 and 5. Repoints the four orphaned Mk-20
callers at the consolidated LoBo_airammo rockeye script, guards the IL-28
fired script like every .1 missile script, and fixes two class literals.
Fails closed when a destination script or referenced ammo class is missing.
"""
import re

from .common import _explicit_exit, apply_rules, p

ROCKEYE = "lobo_airammo/scripts/cbu/rockeye.sqs"
AIRAMMO_CONFIG = "lobo_airammo/config.cpp"
AIRAMMO_EXEC = 'exec "' + p("LoBo_airammo", "Scripts", "cbu", "rockeye.sqs") + '"'
F16_EXEC = 'exec "' + p("LoBoF16", "scr", "rockeye.sqs") + '"'
F15_EXEC = 'exec "' + p("LoBoF15", "scr", "rockeye.sqs") + '"'
DEPLOYED = ("sub", '"LoBoMk20Deployed" createvehicle', '"LoBo_mk20Deployed" createvehicle', 1)
NEAREST = "_missobj = nearestobject[_plane, _ammoname]"
IL28_FIRED = "loboil28/script/fire_mk20.sqs"
RULES = {
    "lobo_a4/scripts/firemissiles_a2.sqs": (("sub", F16_EXEC, AIRAMMO_EXEC, 1),),
    "lobo_a4/scripts/firemissiles_a3.sqs": (("sub", F16_EXEC, AIRAMMO_EXEC, 1),),
    "lobo_f4/scripts/firemissiles_ww.sqs": (("sub", F16_EXEC, AIRAMMO_EXEC, 1),),
    IL28_FIRED: (("sub", F15_EXEC, AIRAMMO_EXEC, 1),
                 ("sub", NEAREST, NEAREST + '\n? isNull _missobj: goto "Exit"\n? !(local _missobj): goto "Exit"', 1)),
    "lobof5/scripts/rockeye.sqs": (DEPLOYED,),
    "loboil28/script/rockeye.sqs": (DEPLOYED,),
    "lobo_airammo/scripts/cbu/rbk500.sqs": (("sub", '"LoBoRBKBomblet" createvehicle', '"LoBoRBK500Bomblet" createvehicle', 1),),
}
REQUIRED_CLASSES = ("LoBo_mk20Deployed", "LoBoMk20Bomblet", "LoBoRBK500Bomblet")
ADDONS = ("lobo_a4", "lobo_airammo", "lobo_f4", "lobof5", "loboil28")


def repair(documents):
    missing = sorted({ROCKEYE, AIRAMMO_CONFIG} - documents.keys())
    if missing:
        raise ValueError("Missing cluster-bomb destination or config: " + ", ".join(missing))
    if not documents[ROCKEYE].strip():
        raise ValueError("Empty cluster-bomb destination: " + ROCKEYE)
    for name in REQUIRED_CLASSES:
        if not re.search(r"(?im)^\s*class\s+" + re.escape(name) + r"\b", documents[AIRAMMO_CONFIG]):
            raise ValueError("Missing cluster-bomb ammo class: " + name)
    return apply_rules(documents, RULES, post={IL28_FIRED: _explicit_exit})
