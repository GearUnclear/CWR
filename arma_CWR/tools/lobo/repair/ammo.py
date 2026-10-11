"""Issue #69 ammunition repair rules; no third-party config payloads.

All input validation is completed in memory. Unknown tuning is an error rather
than an invitation to overwrite a user's balance changes. See AMMUNITION-POLICY.md.
"""
from __future__ import annotations

import math
import re

from .config import class_block, class_spans, replace_exact, scalar, set_scalar

ADDONS = ("loboammo", "loboweapons", "lobo_airammo", "loboter", "loboart")

# min distance/probability, preferred distance/probability, max distance/probability.
MG_PROFILE = (1, 0.90, 100, 0.50, 450, 0.05)
RIFLE_PROFILE = (0.5, 0.10, 50, 0.38, 250, 0.04)
SUPPRESSED_PROFILE = (0.5, 0.25, 70, 0.50, 250, 0.08)
LEGACY_PROFILE = (1, 0.80, 50, 0.35, 109, 0.08)
RANGE_KEYS = ("minRange", "minRangeProbab", "midRange", "midRangeProbab",
              "maxRange", "maxRangeProbab")


def _number(text, path, key):
    value = scalar(text, path, key)
    if value is None:
        return None
    try:
        number = float(value)
    except ValueError as exc:
        raise ValueError(f"Expected literal number at {'/'.join(path)}/{key}: {value}") from exc
    if not math.isfinite(number):
        raise ValueError(f"Non-finite value at {'/'.join(path)}/{key}")
    return number


def _change(text, path, key, original, repaired):
    current = _number(text, path, key)
    if current not in (original, repaired):
        raise ValueError(f"Unexpected {'/'.join(path)}/{key}: {current}; "
                         f"expected {original} or repaired {repaired}")
    if current == repaired:
        return text
    return set_scalar(text, path, key, f"{repaired:g}")


def _profile(text, name, desired):
    path = ("CfgAmmo", name)
    for key, old, new in zip(RANGE_KEYS, LEGACY_PROFILE, desired):
        text = _change(text, path, key, old, new)
    return text


def _literal_in_class(text, path, old, new, expected=1):
    start, end = class_block(text, path)
    body = replace_exact(text[start:end], old, new, expected=expected)
    return text[:start] + body + text[end:]


def _new_class(text, name, parent, properties):
    """Append original project-authored data; reject a conflicting local class."""
    start, end = class_block(text, ("CfgAmmo",))
    matches = [(n, a, b) for n, a, b in class_spans(text[start:end])
               if n.lower() == name.lower()]
    body = "\n".join(f"\t\t{key}={value};" for key, value in properties.items())
    if matches:
        if len(matches) != 1:
            raise ValueError(f"Duplicate generated CfgAmmo/{name}")
        _, a, b = matches[0]
        if re.sub(r"\s+", "", text[start+a:start+b]) != re.sub(r"\s+", "", body):
            raise ValueError(f"Conflicting CfgAmmo/{name}; generated repair was modified")
        # Verify inheritance, not merely the values, for idempotent reapplication.
        header = re.search(r"\bclass\s+" + re.escape(name) + r"\s*:\s*" +
                           re.escape(parent) + r"\s*\{$", text[start:start+a], re.I)
        if not header:
            raise ValueError(f"Unexpected parent for generated CfgAmmo/{name}")
        return text
    nl = "\r\n" if "\r\n" in text else "\n"
    addition = f"\n\tclass {name}: {parent}\n\t{{\n{body}\n\t}};\n"
    return text[:end] + addition.replace("\n", nl) + text[end:]


def _ammo(text):
    # Keep the existing magazine/projectile identifier, introducing an explicit
    # M833 baseline rather than redirecting the magazine to the M900 projectile.
    if scalar(text, ("CfgWeapons", "LoBo_105Sabot2"), "ammo") != '"LoBo_105Sabot2"':
        raise ValueError("Unexpected M833 magazine projectile reference")
    if _number(text, ("CfgAmmo", "LoBo_105Sabot1"), "hit") != 450:
        raise ValueError("M900 reference baseline changed; review M833 policy")
    text = _new_class(text, "LoBo_105Sabot2", "Shell105", {
        "hit": "400", "indirectHit": "0", "indirectHitRange": "0",
        "explosive": "0", "cost": "300", "minRange": "10",
        "minRangeProbab": "0.5", "midRange": "1000", "midRangeProbab": "0.85",
        "maxRange": "2000", "maxRangeProbab": "0.65",
    })
    # Explicit ball/AP families only. HE/HEI cannon rounds and the M40 spotting
    # bullet intentionally retain their effects; inheritance alone is not proof
    # that a projectile is non-explosive.
    ordinary = {
        "LoBo_M240Coax": (None, None), "LoBo_50cal": (None, None),
        "loBo_DTMCoax": (2, 1), "loBo_PKCoax": (2, 1),
        "loBo_DShK": (5, 1), "loBo_KPVT": (5, 1),
        "LoBo_25AP": (5, 1), "LoBo_BMP2_30APFSDS": (0, 1),
        "LoBo_zpu4Bullet": (2, 0.2),
    }
    for name, (hit, radius) in ordinary.items():
        text = _change(text, ("CfgAmmo", name), "indirectHit", hit, 0)
        text = _change(text, ("CfgAmmo", name), "indirectHitRange", radius, 0)
    # Keep authored impact/strength values; repair the two documented oversized
    # plateaus. Heavy mortars remain the existing comparison baseline.
    text = _change(text, ("CfgAmmo", "LoBo_60HVMSHE"), "indirectHitRange", 30, 4)
    text = _change(text, ("CfgAmmo", "LoBo_C4"), "indirectHitRange", 50, 10)
    path = ("CfgWeapons", "lobo_122D25AP")
    value = scalar(text, path, "shortNameMagazine")
    if value not in ('"1225mm AP"', '"122mm AP"'):
        raise ValueError("Unexpected 122mm AP magazine label")
    return set_scalar(text, path, "shortNameMagazine", '"122mm AP"')


def _weapons(text):
    text = _profile(text, "LoBo762Bullet", MG_PROFILE)
    # LoBo556Bullet also serves rifles. Give MG magazines a separate derived
    # projectile so correcting the Minimi/Negev does not turn rifles into MGs.
    text = _profile(text, "LoBo556Bullet", RIFLE_PROFILE)
    text = _profile(text, "LoBo556Bullet_SD", SUPPRESSED_PROFILE)
    text = _new_class(text, "LoBo556MGBullet", "LoBo556Bullet", {
        key: f"{value:g}" for key, value in zip(RANGE_KEYS, MG_PROFILE)
    })
    for magazine in ("LoBoM249Mag", "LoBoNegevMag"):
        path = ("CfgWeapons", magazine)
        value = scalar(text, path, "ammo")
        if value not in ('"LoBo556Bullet"', '"LoBo556MGBullet"'):
            raise ValueError(f"Unexpected {magazine} projectile: {value}")
        text = set_scalar(text, path, "ammo", '"LoBo556MGBullet"')
    path = ("CfgWeapons", "LoBoNegevSDMag")
    value = scalar(text, path, "ammo")
    if value not in ('"LoBo556Bullet"', '"LoBo556Bullet_SD"'):
        raise ValueError(f"Unexpected suppressed Negev projectile: {value}")
    text = set_scalar(text, path, "ammo", '"LoBo556Bullet_SD"')
    # Verify the intended high-dispersion magazine exists before repairing the
    # compatibility array. Preserve every stable identifier, including Guage.
    class_block(text, ("CfgWeapons", "LoBoMUZIHDmag"))
    text = _literal_in_class(text, ("CfgWeapons", "LoBoMUZIbase"),
                             "LoBoMUZIDDmag", "LoBoMUZIHDmag")
    for name in ("LoBo_m590base", "LoBo_spas12base", "LoBo_r870base"):
        text = _literal_in_class(text, ("CfgWeapons", name),
                                 '"12 Guage Shells"', '"12 Gauge Shells"', expected=2)
    return text


def _airammo(text):
    # One warhead policy, keeping guidance, cost, model and magazine data intact.
    desired = {"hit": 1000, "indirectHit": 500, "indirectHitRange": 15}
    for name in ("LoBo_MK82", "LoBo_MK82_LDGB"):
        for key, value in desired.items():
            if _number(text, ("CfgAmmo", name), key) != value:
                raise ValueError(f"Mk82 family reference changed: {name}/{key}")
    for key, old in (("hit", 500), ("indirectHit", 150), ("indirectHitRange", 10)):
        text = _change(text, ("CfgAmmo", "LoBo_GBU12"), key, old, desired[key])
    return text


def _terror(text):
    text = _change(text, ("CfgAmmo", "LoBo_Carbomb"), "indirectHitRange", 50, 10)
    for key, old in (("indirectHit", 5), ("indirectHitRange", 1)):
        text = _change(text, ("CfgAmmo", "loBo_DShK_Ter"), key, old, 0)
    return text


def _mortars(text):
    for name in ("LoBo_120MortarShellHE", "LoBo_M160MortarShell"):
        for key, value in (("indirectHit", 45), ("indirectHitRange", 8)):
            if _number(text, ("CfgAmmo", name), key) != value:
                raise ValueError(f"Mortar comparison baseline changed: {name}/{key}")
    return text


def repair(documents: dict[str, str]) -> dict[str, str]:
    """Return changed config members only, raising before callers write anything."""
    transforms = {"loboammo": _ammo, "loboweapons": _weapons,
                  "lobo_airammo": _airammo, "loboter": _terror, "loboart": _mortars}
    changed = {}
    for addon, transform in transforms.items():
        key = f"{addon}/config.cpp"
        if key not in documents:
            raise ValueError(f"Missing required repair source {key}")
        updated = transform(documents[key])
        if updated != documents[key]:
            changed[key] = updated
    return changed


def blast_sample(strength, radius, distance, *, unshielded=1.0, armor=1.0):
    """Offline point-sample of CalcDammage, not a target lethality simulation.

    The four-radius spatial search/finite object bounds are reported separately;
    do not treat this pure attenuation curve as a radial gameplay cutoff.
    """
    values = (strength, radius, distance, unshielded, armor)
    if not all(math.isfinite(v) for v in values):
        raise ValueError("Non-finite blast scenario")
    if strength < 0 or radius < 0 or distance < 0 or not 0 <= unshielded <= 1 or armor <= 0:
        raise ValueError("Invalid blast scenario")
    if strength == 0 or radius == 0:
        return 0.0
    attenuation = 1 if distance <= radius else (radius / distance) ** 4
    return strength * attenuation * unshielded / armor


def blast_comparisons():
    """Reproducible numeric comparison rows for the no-game balance report."""
    families = (
        ("60mm HVMS", 15, 30, 4), ("120/160mm mortar baseline", 45, 8, 8),
        ("C4 car bomb", 900, 50, 10), ("Ter car bomb", 500, 50, 10),
    )
    rows = []
    for name, strength, old_radius, new_radius in families:
        for distance in (0, 5, 10, 20, 40, 50):
            for cover in (1.0, 0.25, 0.0):
                for armor in (1.0, 10.0):
                    rows.append({"family": name, "distance": distance,
                                 "unshielded": cover, "armor": armor,
                                 "before": blast_sample(strength, old_radius, distance,
                                                       unshielded=cover, armor=armor),
                                 "after": blast_sample(strength, new_radius, distance,
                                                      unshielded=cover, armor=armor),
                                 "search_radius_before": old_radius * 4,
                                 "search_radius_after": new_radius * 4})
    return rows
