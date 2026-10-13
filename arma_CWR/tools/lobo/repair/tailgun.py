"""LoBo issue 13: the IL-28 tail turret script divides by zero without a pilot.

LoBoIL28's tailgun.sqs runs 0.1 s after an IL-28 spawns. It skipped gunner
creation only for an empty plane: with a gunner or cargo but no pilot it built
the gunner from `group driver`, so createunit got grpNull, `units grpNull` was
empty and `_tunits select (count(_tunits) -1)` raised "Zero divisor" (fatal
under --autotest).

TG-1 (issue option 1) is the primary guard: no pilot routes the plane down the
existing empty-plane path, so the script never calls `group objNull`,
createunit into a null group or `units grpNull`. TG-2 (issue option 2) guards
the failing statement itself, for a driver seat that holds a unit with no group
(group returns grpNull without an AIUnit). It is unreachable in the normal
cases, because a living driver's group always contains the driver.

The authored dead-pilot bailout and both eject lines are pinned unchanged
(POST): a pilotless IL-28 above 20 m still ejects the plane's gunner, exactly
as it does for a dead pilot. Plain OFP 1.x SQS (isNull, driver, count),
identical under both compatibility profiles.
"""
from .rules import apply_rules

TAILGUN = "loboil28/script/tailgun.sqs"
GUARD_OLD = '? count (crew _plane) ==0: goto "crtGun"'
GUARD_NEW = '? isNull (driver _plane): goto "crtGun"'
GUNNER = "_gunner = _tunits select (count(_tunits) -1)"
EMPTY_GROUP = '? count _tunits == 0: goto "crtGun"'
BAILOUT = ('? (!(driver _plane in _plane) || !(alive driver _plane)) and _exit == 0 '
           'and (position _plane select 2) > 20: goto "loop2"')
EJECT_TURRET = ('if (alive gunner _gun) then {_gunner = gunner _gun; _gunner action ["EJECT",_gun];'
                '_para = "ParachuteEast" createVehicle [0,0,0];_para setpos getpos _camera;_gunner moveindriver _para;}')
EJECT_PLANE = 'if (alive gunner _plane) then {gunner _plane action ["EJECT",_plane]}'
PINNED = (BAILOUT, EJECT_TURRET, EJECT_PLANE)

RULES = {TAILGUN: (
    # TG-1 (issue option 1): no pilot means no group to build the gunner from.
    ("line", GUARD_OLD, GUARD_NEW, 1),
    # TG-2 (issue option 2): never select index -1 from an empty group.
    ("sub", GUNNER, EMPTY_GROUP + "\n" + GUNNER, 1),
)}


def _bailout_kept(text: str) -> str:
    # The dead-pilot bailout and its two ejects stay exactly as authored, and
    # the TG-2 guard exists once (a stray second copy is not a known state).
    lines = text.split("\n")
    for line in PINNED + (EMPTY_GROUP,):
        if lines.count(line) != 1:
            raise ValueError(f"Expected the line exactly once: {line!r}")
    return text


POST = {TAILGUN: _bailout_kept}
ADDONS = ("loboil28",)


def repair(documents: dict[str, str]) -> dict[str, str]:
    return apply_rules(documents, RULES, post=POST)
