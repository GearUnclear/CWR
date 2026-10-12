"""LoBo issue 14: file references that name a file LoBo never shipped.

Each repoint names a file the same addon ships and already uses for the same
purpose; REQUIRED_MEMBERS makes plan() fail closed when a reinstall lacks one.

Text rules:
- LoBoMig: soundLandCrash[] of LoBo_E_mig21bis_MR1 named the folder `soundss`;
  the file ships under `sounds`, where soundCrash[] on the line above finds it.
- LoBo_Pilot: five wounds[] healthy-texture entries name a `.pac` that ships
  only as `.paa` (EAF back and legsbk, RJAF back and legsbk, IAF tex2 legsbk).
  UD's WoundInfo::Load matches the healthy texture by name in the shape, so
  these pairs never matched a face. The three IAF pairs whose `.pac` ships
  (IAF tex back and legsbk, IAF tex2 back) stay as authored. Each pair is
  counted in the class that authored it (PILOT_CLASSES), because
  repair.wounds later writes the repaired IAF tex2 legsbk pair into the
  grey2 and grey3 classes too; an unrepaired `.pac` pair anywhere else in
  the config still fails closed.

Model repoints (fixed-width MLOD texture fields, textures.repoint):
- Hilux and Hilux MG: LODs 3.0 and 5.65 name `black_matte.pac`; LODs 0, 10 and
  1100 of the same models name the shipped `black matte.pac`. Both models end
  with an Oxygen default-path buffer whose path is empty (NUL first, then
  stale memory), pinned byte-exact per member.
- EAF pilot: LODs 1.0 and 5.0 name EAF back and legsbk `.pac`; LODs 0.15 and
  1100 name the shipped `.paa`.
- AIM-9: the Geometry LOD (1e13, never drawn) names `rktf5\\swtex.paa` from a
  lost addon. Its 14 faces take the texture of all faces of the model's only
  visual LOD, so the rendered missile and the loaded texture set do not change.

`LoBoF16\\{IAF,EAF,com}\\backfuse.pac` (one face in LOD 10 of each F-16) has no
source anywhere and stays as authored (known missing, CONTENT-REPAIRS.md).
Profile-neutral: config string literals and texture names only.
"""
from .config import class_block
from .rules import apply_rules, p, split_newline
from .scripts import _replace
from .textures import _b, repoint

MIG = "lobomig/config.cpp"
PILOT = "lobo_pilot/config.cpp"


def _wound(folder: tuple, name: str) -> tuple:
    """`"<healthy>.pac", "<wounded>_d` becomes `"<healthy>.paa", ...` once.

    The match stops before the wounded name's extension, so it holds both for
    the shipped `<name>_d.paa` and once repair.wounds (which runs later) has
    pointed an EAF or IAF tex2 pair at its generated `<name>_dr.paa`.
    """
    wounded = '"' + p("LoBo_Pilot", *folder, name + "_d")
    return ("sub", '"' + p("LoBo_Pilot", *folder, name + ".pac") + '", ' + wounded,
            '"' + p("LoBo_Pilot", *folder, name + ".paa") + '", ' + wounded, 1)


# CfgVehicles class: its LoBo_Pilot rules, each counted inside that class only.
PILOT_CLASSES = {
    "LoBo_Egypt_Pilot": (_wound(("EAF", "tex"), "back"), _wound(("EAF", "tex"), "legsbk")),
    "LoBo_IAF_Pilot_grey": (_wound(("IAF", "tex2"), "legsbk"),),
    "LoBo_Jordan_Pilot": (_wound(("RJAF", "tex"), "back"), _wound(("RJAF", "tex"), "legsbk")),
}
RULES = {
    MIG: (("line", '\t\tsoundLandCrash[]={"' + p("LoBoMig", "soundss", "jet_crash.wss") + '",0.006000,1};',
           '\t\tsoundLandCrash[]={"' + p("LoBoMig", "sounds", "jet_crash.wss") + '",0.006000,1};', 1),),
    PILOT: tuple(rule for rules in PILOT_CLASSES.values() for rule in rules),
}

HILUX, HILUX_MG = "loboter/lobo_terhilux.p3d", "loboter/lobo_terhiluxmg.p3d"
EPILOT, AIM9 = "lobo_pilot/lobo_epilot.p3d", "lobo_airammo/lobo_aim9.p3d"
MODEL_MEMBERS = (HILUX, HILUX_MG, EPILOT, AIM9)
OLD_MATTE = _b("LoBoTer", "Toy", "t", "black_matte.pac")
NEW_MATTE = _b("LoBoTer", "Toy", "t", "black matte.pac")
PLANS = {  # member: (resolutions, mapping, pinned trailer or None)
    HILUX: ((3.0, 5.65), {OLD_MATTE: (NEW_MATTE, (176, 151), False)},
            bytes.fromhex("000000000000000000000000e03ea1000d000000f8fb1e0100b5a000b4f3c600")),
    HILUX_MG: ((3.0, 5.65), {OLD_MATTE: (NEW_MATTE, (178, 151), False)},
               bytes.fromhex("000000000000000000000000e03ea1000e000000787e710100b5a000b4f3c600")),
    EPILOT: ((1.0, 5.0), {
        _b("LoBo_Pilot", "EAF", "tex", "back.pac"): (_b("LoBo_Pilot", "EAF", "tex", "back.paa"), (21, 14), False),
        _b("LoBo_Pilot", "EAF", "tex", "legsbk.pac"): (_b("LoBo_Pilot", "EAF", "tex", "legsbk.paa"), (34, 0), False),
    }, None),
    AIM9: ((1e13,), {_b("rktf5", "swtex.paa"): (_b("LoBo_airammo", "tex", "sw.pac"), 14, False)}, None),
}
REQUIRED_MEMBERS = ("lobomig/sounds/jet_crash.wss", "loboter/toy/t/black matte.pac",
                    "lobo_pilot/eaf/tex/back.paa", "lobo_pilot/eaf/tex/legsbk.paa",
                    "lobo_pilot/iaf/tex2/legsbk.paa", "lobo_pilot/rjaf/tex/back.paa",
                    "lobo_pilot/rjaf/tex/legsbk.paa", "lobo_airammo/tex/sw.pac")
ADDONS = ("lobo_airammo", "lobo_pilot", "lobomig", "loboter")


def _repair_pilot(source: str) -> str:
    """PILOT_CLASSES rules inside their classes; no unrepaired original may remain anywhere."""
    newline, text = split_newline(source)
    try:
        for cls, rules in PILOT_CLASSES.items():
            start, end = class_block(text, ("CfgVehicles", cls))
            body = text[start:end]
            for _, before, after, count in rules:
                body = _replace(body, before, after, count)
            text = text[:start] + body + text[end:]
        for _, before, _, _ in RULES[PILOT]:
            if before in text:
                raise ValueError(f"Unexpected {before!r} outside its class")
    except ValueError as exc:
        raise ValueError(f"{PILOT}: {exc}") from exc
    return text.replace("\n", newline)


def repair(documents: dict[str, str]) -> dict[str, str]:
    if PILOT not in documents:
        raise ValueError("Missing members: " + PILOT)
    changed = apply_rules(documents, {MIG: RULES[MIG]})
    updated = _repair_pilot(documents[PILOT])
    if updated != documents[PILOT]:
        changed[PILOT] = updated
    return changed


def repair_models(documents: dict[str, bytes]) -> dict[str, bytes]:
    changed = {}
    for key, (resolutions, mapping, trailer) in PLANS.items():
        if key not in documents:
            raise ValueError("Missing model " + key)
        try:
            fixed = repoint(documents[key], resolutions, mapping, trailer)
        except ValueError as exc:
            raise ValueError(f"{key}: {exc}") from exc
        if fixed != documents[key]:
            changed[key] = fixed
    return changed
