"""Narrow, fail-closed fixes for LoBo issues 6, 7 and 10.

Only original transformation rules belong here; installed addon scripts remain
local. Preserve all unrelated bytes, including the legacy newline sequences.
"""
from .scripts import _replace

AP_MINE = "lobo_mines/scripts/ap_mine.sqs"
FAST_INSERTION = "loboidfhelo/uh60/fx/fastins.sqs"
SWING_WING = "lobomig/mig23/scr/swingwing.sqs"
AP_PICKUP = "lobo_mines/scripts/ap_mine1.sqs"
INSERTION_TARGET = "loboidfhelo/uh60/fx/fastins1.sqs"
ADDONS = ("lobo_mines", "loboidfhelo", "lobomig")
RULES = {
    AP_MINE: ((r'"\LoBo_Mines\scripts\AT_Mine1.sqs"',
               r'"\LoBo_Mines\scripts\AP_Mine1.sqs"'),),
    FAST_INSERTION: ((r'"\LoboIDFhelo\uh60\FXfastins1.sqs"',
                      r'"\LoboIDFhelo\uh60\FX\fastins1.sqs"'),),
    SWING_WING: (("?(fraction > l) : _fraction =1", "?(_fraction > 1) : _fraction =1"),
                 ("?(fraction < 0) : _fraction =0", "?(_fraction < 0) : _fraction =0")),
}


def repair(documents: dict[str, str]) -> dict[str, str]:
    required = set(RULES) | {AP_PICKUP, INSERTION_TARGET}
    missing = sorted(required - documents.keys())
    if missing:
        raise ValueError("Missing follow-up script or destination: " + ", ".join(missing))
    for destination in (AP_PICKUP, INSERTION_TARGET):
        if not documents[destination].strip():
            raise ValueError("Empty follow-up destination: " + destination)
    changed = {}
    for path, rules in RULES.items():
        source = documents[path]
        updated = source
        try:
            for before, after in rules:
                updated = _replace(updated, before, after)
        except ValueError as exc:
            raise ValueError(f"{path}: {exc}") from exc
        if updated != source:
            changed[path] = updated
    return changed
