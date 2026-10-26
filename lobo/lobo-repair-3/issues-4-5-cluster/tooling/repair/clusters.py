"""SCRATCH PROTOTYPE (issues-4-5-cluster investigation): LoBo issues 4 and 5.

Repoints the three orphaned Mk-20 callers at the consolidated LoBo_airammo
rockeye script and corrects two class-name literals. Fails closed when a
destination script or the referenced config class is missing.
"""
import re

from .scripts import _explicit_exit, _replace, script_findings

BS = chr(92)


def _path(*parts):
    return BS + BS.join(parts)


ROCKEYE = "lobo_airammo/scripts/cbu/rockeye.sqs"
AIRAMMO_CONFIG = "lobo_airammo/config.cpp"
AIRAMMO_EXEC = 'exec "' + _path("LoBo_airammo", "Scripts", "cbu", "rockeye.sqs") + '"'
F16_EXEC = 'exec "' + _path("LoBoF16", "scr", "rockeye.sqs") + '"'
F15_EXEC = 'exec "' + _path("LoBoF15", "scr", "rockeye.sqs") + '"'
DEPLOYED = ('"LoBoMk20Deployed" createvehicle', '"LoBo_mk20Deployed" createvehicle')
NEAREST = "_missobj = nearestobject[_plane, _ammoname]"
IL28_FIRED = "loboil28/script/fire_mk20.sqs"
RULES = {
    "lobo_a4/scripts/firemissiles_a2.sqs": ((F16_EXEC, AIRAMMO_EXEC),),
    "lobo_a4/scripts/firemissiles_a3.sqs": ((F16_EXEC, AIRAMMO_EXEC),),
    "lobo_f4/scripts/firemissiles_ww.sqs": ((F16_EXEC, AIRAMMO_EXEC),),
    "loboil28/script/fire_mk20.sqs": ((F15_EXEC, AIRAMMO_EXEC),
                                      (NEAREST, NEAREST + '\n? isNull _missobj: goto "Exit"\n? !(local _missobj): goto "Exit"')),
    "lobof5/scripts/rockeye.sqs": (DEPLOYED,),
    "loboil28/script/rockeye.sqs": (DEPLOYED,),
    "lobo_airammo/scripts/cbu/rbk500.sqs": (('"LoBoRBKBomblet" createvehicle', '"LoBoRBK500Bomblet" createvehicle'),),
}
REQUIRED_CLASSES = ("LoBo_mk20Deployed", "LoBoMk20Bomblet", "LoBoRBK500Bomblet")
ADDONS = ("lobo_a4", "lobo_airammo", "lobo_f4", "lobof5", "loboil28")


def repair(documents: dict[str, str]) -> dict[str, str]:
    required = set(RULES) | {ROCKEYE, AIRAMMO_CONFIG}
    missing = sorted(required - documents.keys())
    if missing:
        raise ValueError("Missing cluster-bomb script, destination or config: " + ", ".join(missing))
    if not documents[ROCKEYE].strip():
        raise ValueError("Empty cluster-bomb destination: " + ROCKEYE)
    config = documents[AIRAMMO_CONFIG]
    for name in REQUIRED_CLASSES:
        if not re.search(r"(?im)^\s*class\s+" + re.escape(name) + r"\b", config):
            raise ValueError("Missing cluster-bomb ammo class: " + name)
    changed = {}
    for path, rules in RULES.items():
        source = documents[path]
        newline = "\r\n" if "\r\n" in source else "\n"
        updated = source.replace("\r\n", "\n")
        try:
            for before, after in rules:
                updated = _replace(updated, before, after)
            if path == IL28_FIRED:
                updated = _explicit_exit(updated)
            if path.endswith(".sqs"):
                findings = script_findings(updated)
                if findings:
                    raise ValueError("; ".join(findings))
        except ValueError as exc:
            raise ValueError(f"{path}: {exc}") from exc
        updated = updated.replace("\n", newline)
        if updated != source:
            changed[path] = updated
    return changed
