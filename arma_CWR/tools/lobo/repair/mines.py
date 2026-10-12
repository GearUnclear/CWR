"""LoBo issue 3: placed-mine poll interval, deleted-mine exit and the flare
mine's leaked helper, in LoBo_Mines and its JOF_Objects1 source.

Each placed mine scanned nearestObject every frame for the session, and a
deleted mine kept polling and could fire a ghost explosion. isNull retires the
script with its mine. 0.1 s keeps AP detection reliable at sprint speed and AT
lag under 2 m at 70 km/h; the about 1 m flare tripwire keeps 0.03 s.
"""
from .rules import apply_rules

POLL = "~0.1"
FLARE_POLL = "~0.03"
GUARD = "? isNull _L1 : exit"
FLARE_GUARD = "? isNull _L1 : deleteVehicle _Mess; exit"
PLAIN = (("sub", "#Zut\n~0.001\n_J = nearestobject [_X,_Y,_Z]\n",
          f"#Zut\n{POLL}\n{GUARD}\n_J = nearestobject [_X,_Y,_Z]\n", 1),)
LADUNG = (("sub", "#Zut\n~0.001\n_W = _D + 45\n", f"#Zut\n{POLL}\n{GUARD}\n_W = _D + 45\n", 1),)
CLAYMORE = (("sub", "#Zut\n~0.01\n_M = _L1\n", f"#Zut\n{POLL}\n{GUARD}\n_M = _L1\n", 1),)
# The flare mine also deletes its global HeliHEmpty helper on the trigger and
# pickup exits. The '? not alive _L1:exit' rule touches only leucht_mine.
LEUCHT = (("sub", "#Zut\n~0.0001\n", f"#Zut\n{FLARE_POLL}\n{FLARE_GUARD}\n", 1),
          ("sub", '#Ende\n_L1 animate ["aus", 1]\n', '#Ende\ndeleteVehicle _Mess\n_L1 animate ["aus", 1]\n', 1),
          ("sub", "? not alive _L1:exit\n", "? not alive _L1:deleteVehicle _Mess; exit\n", 1))
RULES = {f"lobo_mines/scripts/{name}.sqs": PLAIN
         for name in ("ap_mine", "at1_mine", "at2_mine", "at3_mine", "at4_mine", "at5_mine",
                      "at6_mine", "egyptnavmine", "m08navmine", "mk118_mine")}
RULES.update({
    "lobo_mines/scripts/ladung.sqs": LADUNG,
    "lobo_mines/scripts/leucht_mine.sqs": LEUCHT,
    "lobo_mines/scripts/claymore.sqs": CLAYMORE,
    "jof_objects1/ap_mine.sqs": PLAIN,
    "jof_objects1/ladung.sqs": LADUNG,
    "jof_objects1/leucht_mine.sqs": LEUCHT,
    "jof_objects1/claymore.sqs": CLAYMORE,
})
ADDONS = ("jof_objects1", "lobo_mines")


def repair(documents: dict[str, str]) -> dict[str, str]:
    return apply_rules(documents, RULES)
