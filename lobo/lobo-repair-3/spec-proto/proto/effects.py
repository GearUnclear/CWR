"""SPEC PROTOTYPE: LoBo issue 9 (and the KioGlowFire sibling). Defines the
undefined wreck-glow classes once, in LoBo_airammo's config, as an invisible
vanilla Fire. No script changes, identical bytes under ud and classic.
"""
import re

from .common import split_newline

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
ADDONS = ("lobo_airammo",)


def repair(documents):
    if CONFIG not in documents:
        raise ValueError("Missing " + CONFIG)
    source = documents[CONFIG]
    newline, text = split_newline(source)
    declared = [n for n in NAMES if re.search(r"(?im)^\s*class\s+" + re.escape(n) + r"\b", text)]
    cfgvehicles = len(re.findall(r"(?im)^\s*class\s+CfgVehicles\b", text))
    if text.count(BLOCK) == 1 and cfgvehicles == 1 and len(declared) == len(NAMES):
        return {}
    if text.count(BLOCK) or cfgvehicles or declared:
        raise ValueError(f"{CONFIG}: partial or foreign CfgVehicles/wreck-glow state: {declared}")
    if text.count(ANCHOR) != 1:
        raise ValueError(f"{CONFIG}: expected one CfgNonAIVehicles anchor")
    text = text.replace(ANCHOR, "};\n" + BLOCK + "class CfgNonAIVehicles\n")
    return {CONFIG: text.replace("\n", newline)}
