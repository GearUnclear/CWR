"""LoBo issue 9 and its KioGlowFire sibling: undefined wreck-glow classes.

M1Smoker, M60Smoker, Smoker and KioGlowFire are created by the wreck fire.sqs
scripts but defined nowhere, so createVehicle returned objNull on the owner
path: no light, no Fireplace, no crackle and no say. Define them once in
LoBo_airammo's config as an invisible vanilla Fire. The base chain is declared
with empty bodies because a merge only adds classes.

A live KioGlowFire also makes the BAS helicopter burn line live. It measured
`player distance _glow`, which was 1e10 while the glow was objNull; with a real
glow it would add 0.04 damage every 0.25 s within 10 m, with no on-foot test
(a seated player's position is the vehicle's) and no reset of its warning. The
five helicopter copies therefore get the burn policy the ground scripts already
follow (scripts._fire): one scheduler on the wreck owner, on-foot, local and
alive player only, measured to the wreck, at most once per 40 ticks (10 s). The
authored 0.04 per hit stays. Plain OFP 1.x SQS, identical under both profiles.
"""
import re

from .rules import apply_rules, split_newline

CONFIG = "lobo_airammo/config.cpp"
ANCHOR = "};\nclass CfgNonAIVehicles\n"
NAMES = ("LoBo_WreckGlow", "M1Smoker", "M60Smoker", "Smoker", "KioGlowFire")
BLOCK = "\n".join((
    "class CfgVehicles",
    "{",
    "\tclass All {};",
    "\tclass Static: All {};",
    "\tclass Building: Static {};",
    "\tclass NonStrategic: Building {};",
    "\tclass Fire: NonStrategic {};",
    "\t// UD repair, LoBo issue 9: invisible fireplace that the wreck fire.sqs",
    "\t// scripts create for inflame and say. Vanilla Fire supplies simulation,",
    "\t// sound, Light and Smoke; only the campfire mesh is removed.",
    "\tclass LoBo_WreckGlow: Fire",
    "\t{",
    "\t\tscope=1;",
    "\t\tmodel=\"empty\";",
    "\t\tdisplayName=\"Wreck fire\";",
    "\t};",
    "\tclass M1Smoker: LoBo_WreckGlow {};",
    "\tclass M60Smoker: LoBo_WreckGlow {};",
    "\tclass Smoker: LoBo_WreckGlow {};",
    "\tclass KioGlowFire: LoBo_WreckGlow {};",
    "};",
    "",
))
# The five BAS helicopter wreck scripts that create KioGlowFire.
HELI_FIRE = ("loboeghelo/engine/fire.sqs", "loboeghelo/engine2/fire.sqs", "loboeghelo/engine3/fire.sqs",
             "loboidfhelo/ch53/engine/fire.sqs", "loboidfhelo/uh1/engine/fire.sqs")
HELI_BURN = (
    # Counter for the warning reset, as the ground scripts keep it.
    ("sub", "_lifeTick = _lifeTicks\n_ouch = 0\n", "_lifeTick = _lifeTicks\n_ouch = 0\n_fire = 0\n", 1),
    # One burn scheduler on the wreck owner; a seated player is not burnt; only a
    # local, living player within 10 m of the wreck, once until the reset.
    ("sub", '#client\n\n;burn player\n'
            '? player distance _glow <10: player setdammage ((getdammage player) +0.04),goto "scream"\n',
            '#client\n? !(local _object): goto "fire"\n\n;burn player\n'
            '? player != (vehicle player): goto "fire"\n'
            '? local player && alive player && player distance _object < 10 && _ouch != 1: '
            'player setdammage ((getdammage player) +0.04); goto "scream"\n', 1),
    # Allow the next burn and warning after 40 ticks (10 s).
    ("sub", "_lifeTick = _lifeTick - 1\n_soundloop = _soundloop +1\n",
            "_lifeTick = _lifeTick - 1\n_soundloop = _soundloop +1\n"
            "_fire = _fire + 1\n? _fire >= 40: _fire = 0; _ouch = 0\n", 1),
)
RULES = {path: HELI_BURN for path in HELI_FIRE}


def _one_guarded_burn(text: str) -> str:
    # Reject an unreviewed copy with a second, unguarded burn line.
    lowered = text.lower()
    if lowered.count("setdammage") != 1 or "distance _glow" in lowered:
        raise ValueError("Expected exactly one, guarded, helicopter burn line")
    return text


POST = {path: _one_guarded_burn for path in HELI_FIRE}
ADDONS = tuple(sorted({"lobo_airammo"} | {path.split("/")[0] for path in HELI_FIRE}))


def _declared(text: str, name: str) -> bool:
    return bool(re.search(r"(?im)^\s*class\s+" + re.escape(name) + r"\b", text))


def _glow_classes(documents: dict[str, str]) -> dict[str, str]:
    if CONFIG not in documents:
        raise ValueError("Missing " + CONFIG)
    source = documents[CONFIG]
    newline, text = split_newline(source)
    declared = [name for name in NAMES if _declared(text, name)]
    cfgvehicles = len(re.findall(r"(?im)^\s*class\s+CfgVehicles\b", text))
    if text.count(BLOCK) == 1 and cfgvehicles == 1 and len(declared) == len(NAMES):
        return {}
    fragment = any(line.strip() and line in text for line in BLOCK.split("\n")[7:10])
    if text.count(BLOCK) or cfgvehicles or declared or fragment:
        raise ValueError(f"{CONFIG}: partial or foreign CfgVehicles/wreck-glow state: {declared}")
    if text.count(ANCHOR) != 1:
        raise ValueError(f"{CONFIG}: expected one CfgNonAIVehicles anchor")
    text = text.replace(ANCHOR, "};\n" + BLOCK + "class CfgNonAIVehicles\n")
    return {CONFIG: text.replace("\n", newline)}


def repair(documents: dict[str, str]) -> dict[str, str]:
    # Both halves are required: the glow class is never defined without the
    # helicopter burn guard it makes reachable.
    changed = _glow_classes(documents)
    changed.update(apply_rules(documents, RULES, post=POST))
    return changed
