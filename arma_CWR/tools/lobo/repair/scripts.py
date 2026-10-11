"""Small, fail-closed transformations for issue 69's original LoBo SQS files.

No bundled script is redistributed here. The default wreck query uses UD's
nearestObjects command. The explicit Classic profile uses only the player and
wreck groups; it cannot discover unrelated nearby groups. Apply profiles to
separate mod copies, rather than distributing the UD query as Classic compatible.
"""

from collections import Counter
import re


MISSILE_PATHS = tuple(
    f"{addon}/{directory}/{name}.sqs"
    for addon, directory, names in (
        ("lobo_a4", "scripts", "firemissiles_a firemissiles_a2 firemissiles_a3"),
        ("lobo_f4", "scripts", "firemissiles_air firemissiles_aire firemissiles_recc firemissiles_ww"),
        ("lobo_kfir", "scripts", "firemissiles_cas1"),
        ("lobof15", "scr", "fire_c_aa fire_e fire_e_a120 fire_e_aa fire_e_cbu fire_e_gbu fire_e_mav fire_e_mk84"),
        ("lobof16", "scr", "firemissiles_aa firemissiles_ag firemissiles_f16i_aa firemissiles_f16i_bomber firemissiles_f16i_cas firemissiles_f16i_ps firemissiles_f16i_sead firemissiles_f16i_stand firemissiles_ps firemissiles_suw"),
        ("lobof5", "scripts", "firemissiles firemissiles2 firemissiles3"),
        ("lobomig", "mig23/scr", "firemissiles_ga firemissiles_ic"),
        ("lobomig", "mig25/sqs", "firemissiles_cap1"),
        ("lobomig", "mig29/scripts", "firemissiles_cap1"),
        ("lobomig", "scripts", "firemissiles_cas1 firemissiles_cas2 firemissiles_ic1 firemissiles_ic2 firemissiles_mr1 firemissiles_mr2"),
        ("lobomig", "scripts/syria", "firemissiles_cas1 firemissiles_ic1"),
    )
    for name in names.split()
)

FIRE_PATHS = tuple(
    f"{addon}/{directory}/fire.sqs"
    for addon, directory in (
        ("lobo_airammo", "scripts/smoke"),
        ("lobo_btr60", "scripts"),
        ("lobo_fab", "lcm/scripts"),
        ("lobo_fab", "saar5/scripts"),
        ("lobo_fab", "scripts"),
        ("lobo_zelda", "scripts"),
        ("lobo_zu23", "scripts"),
        ("lobocen", "scripts"),
        ("loboegapc", "scripts"),
        ("lobof15", "scr"),
        ("lobof16", "scr"),
        ("lobom60", "scripts"),
        ("lobomerkava2", "scripts"),
        ("lobomerkava4", "scripts"),
        ("lobosa6", "scripts"),
        ("loboter", "scripts"),
    )
)
NUMBER_PATHS = ("lobo_fab/lcm/scripts/number.sqs", "lobo_fab/lcm/scripts/number2.sqs")
DISPLAY_PATHS = ("lobojordan/config.cpp", "lobosa6/config.cpp", "loboweapat/config.cpp")
ADDONS = tuple(sorted({p.split("/")[0] for p in MISSILE_PATHS + FIRE_PATHS + NUMBER_PATHS + DISPLAY_PATHS}))


def _replace(text: str, before: str, after: str, count: int = 1) -> str:
    """Accept original, repaired, or partially repaired known occurrences only."""
    repaired = text.count(after)
    remaining = text.replace(after, "").count(before)
    if repaired + remaining != count:
        raise ValueError(f"Unexpected script source: expected {count} occurrence(s) of {before!r}")
    # A replacement may include the original anchor. Protect repaired occurrences.
    return after.join(part.replace(before, after) for part in text.split(after))


def _active_lines(text: str) -> list[str]:
    return [line for line in text.splitlines() if not line.lstrip().startswith(";")]


def script_findings(text: str) -> list[str]:
    """Conservative static audit, not a full SQS parser or dataflow proof.

    Labels are case-insensitive in Script::GoTo. Ignore full-line SQS comments,
    and quoted asset paths when finding locals. String-based forEach code remains
    opaque: the supported repaired files use _x as the engine-provided iterator.
    A never-assigned local is actionable; an assignment alone does not prove every
    branch initializes it. The repaired wreck branch is tested separately.
    """
    code = "\n".join(_active_lines(text))
    labels = Counter(label.lower() for label in re.findall(r"^\s*#([^\s;]+)", code, re.M))
    jumps = {label.lower() for label in re.findall(r'\bgoto\s+"([^"\n]+)"', code, re.I)}
    findings = [f"missing label: {label}" for label in sorted(jumps - labels.keys())]
    findings += [f"duplicate label: {label}" for label, n in sorted(labels.items()) if n > 1]
    unquoted = re.sub(r'"(?:[^"\n]|"")*"', '""', code)
    assigned = {name.lower() for name in re.findall(r"\b(_\w+)\s*=(?!=)", unquoted)}
    references = {name.lower() for name in re.findall(r"\b_\w+\b", unquoted)}
    findings += [f"never-assigned local: {name}" for name in sorted(references - assigned - {"_this", "_x", "_time"})]
    return findings


def _explicit_exit(text: str) -> str:
    """Make old goto Exit termination explicit without changing its behavior."""
    if not re.search(r"^\s*#exit\s*$", text, re.M | re.I):
        text = text.rstrip() + "\n\n#Exit\nexit\n"
    return text


def _missile(text: str, path: str) -> str:
    if path in ("lobomig/scripts/firemissiles_ic1.sqs", "lobomig/scripts/firemissiles_ic2.sqs"):
        text = _replace(text, 'goto "sp4away"', 'goto "sw4away"')
    if path in ("lobof5/scripts/firemissiles2.sqs", "lobof5/scripts/firemissiles3.sqs"):
        text = _replace(text, 'goto "aa2way"', 'goto "aa2away"')
    if path == "lobomig/mig25/sqs/firemissiles_cap1.sqs":
        for old, new, slot in (("R60_1away", "R60_3away", 5), ("R60_2away", "R60_4away", 6)):
            tail = f'\n_plane setobjecttexture[{slot}, ""]'
            text = _replace(text, f"#{old}{tail}", f"#{new}{tail}")

    # Keep texture changes on every event recipient, but move a projectile and
    # create helpers only on its owner. Missing nearestObject results exit safely.
    anchor = "_missobj = nearestobject[_plane, _ammoname]"
    text = _replace(text, anchor, anchor + '\n? isNull _missobj: goto "Exit"\n? !(local _missobj): goto "Exit"')

    # A4's two cluster-bomb scripts contain a second, already-cleaned helper.
    # Repair just the launch helper: its unique plane-position anchor disambiguates it.
    start = '_sensor = "EmptyDetector" createVehicle [0,0,0]'
    read = "_result = getpos _sensor select 2;"
    first = text.find(start)
    end = text.find(read, first) + len(read)
    if first < 0 or end < len(read):
        raise ValueError(f"Missing missile terrain helper in {path}")
    helper = text[first:end]
    if helper.count(start) != 1 or "_sensor setpos [getpos _plane select 0, getpos _plane select 1]" not in helper:
        raise ValueError(f"Ambiguous missile terrain helper in {path}")
    # This helper has only three uses before the added delete. No branch/yield is
    # allowed between creation and release, so every created launch helper dies.
    if helper.lower().count("_sensor") != 3 or re.search(r"(?im)^\s*(?:[?~@#]|goto\b|exit\b)", helper):
        raise ValueError(f"Unexpected missile terrain-helper lifetime in {path}")
    cleanup = "\ndeleteVehicle _sensor"
    if not text[end:].startswith(cleanup):
        text = text[:end] + cleanup + text[end:]
    # Reject unreviewed copies with an extra use after the first read. A4's second
    # sensor owns its own three uses and delete; the expected counts cover both.
    expected_helpers = 2 if path in ("lobo_a4/scripts/firemissiles_a2.sqs", "lobo_a4/scripts/firemissiles_a3.sqs") else 1
    active = "\n".join(_active_lines(text)).lower()
    if active.count(start.lower()) != expected_helpers or active.count("_sensor") != 4 * expected_helpers:
        raise ValueError(f"Unexpected missile sensor references in {path}")
    return _explicit_exit(text)


# nearestObjects is registered by GameStateExt.cpp. setDammage calls
# Object::SetDammageNetAware, which forwards remote requests to their owner.
# Killed handlers themselves run on the wreck owner (EntityAI::Destroy).
AI_QUERY = (
    '_AI = {private ["_near", "_units"]; '
    '_near = nearestObjects [_this select 0, ["Man"], _this select 1]; '
    '_units = []; {if (alive _x && vehicle _x == _x && _x != player) then '
    '{_units = _units + [_x]}} forEach _near; _units}'
)
AI_QUERY_CLASSIC = (
    '_AI = {private ["_near", "_units"]; '
    '_near = (units group player) + (units group _object); '
    '_units = []; {if (alive _x && vehicle _x == _x && _x != player '
    '&& _x distance (_this select 0) < (_this select 1) && !(_x in _units)) then '
    '{_units = _units + [_x]}} forEach _near; _units}'
)


def _fire(text: str, compatibility: str = "ud") -> str:
    if compatibility not in ("ud", "classic"):
        raise ValueError(f"Unknown script compatibility profile: {compatibility}")
    query = AI_QUERY if compatibility == "ud" else AI_QUERY_CLASSIC
    existing = sum(text.count(candidate) for candidate in (AI_QUERY, AI_QUERY_CLASSIC))
    if existing > 1:
        raise ValueError("Ambiguous wreck-fire query definitions")
    for candidate in (AI_QUERY, AI_QUERY_CLASSIC):
        if candidate in text:
            text = text.replace(candidate, query)
    anchor = "_object = _this select 0"
    text = _replace(text, anchor, anchor + "\n" + query)
    anchor = '?! (local _object):goto "sound"'
    text = _replace(text, anchor, "_glow = _object\n" + anchor)
    # Non-owner clients use the wreck as their sound anchor. Never move or delete
    # that fallback: only the creator's separate glow object has that lifetime.
    anchor = '#sound\n_soundloop = 0\n_glow setpos [getpos _object select 0, getpos _object select 1,1]'
    text = _replace(text, anchor, '#sound\n_soundloop = 0\n? _glow != _object && local _glow: _glow setpos [getpos _object select 0, getpos _object select 1,1]')
    text = _replace(text, "deletevehicle _glow", "? _glow != _object && local _glow: deletevehicle _glow")
    # One burn scheduler per wreck; setDammage forwards each remote victim's hit.
    text = _replace(text, "#client\n", '#client\n? !(local _object): goto "fire"\n')
    text = _replace(text, '? player != (vehicle player): goTo "fire"', '? player != (vehicle player): goTo "here"')
    text = _replace(text, "? player distance _glow < 10 && _ouch != 1:", "? local player && alive player && player distance _object < 10 && _ouch != 1:")
    text = _replace(text, "#here\n", '#here\n? _ouchAI == 1: goto "fire"\n')
    return text


def _number(text: str) -> str:
    text = _replace(text, "_number = _this select 1", "; Init provides only the vehicle; an unused second argument is unnecessary.")
    # Thirty texture assignments, ten for each digit. Keep stable model slots.
    text = _replace(text, "_unit setObjectTexture", "_plane setObjectTexture", 30)
    text = _replace(text, 'goto "n2"', 'goto "Num2"', 10)
    text = _replace(text, 'goto "n3"', 'goto "Num3"', 10)
    # Normalize legacy commas: EvaluateMultiple tolerates them at runtime, but
    # rejects them in check-only parsing. This is compatibility polish, not a
    # newly claimed runtime defect.
    text = _replace(text, '], goto "', ']; goto "', 30)
    return _explicit_exit(text)


def _display(text: str, path: str) -> str:
    if path == "lobojordan/config.cpp":
        for role in ("Operator", "Team Leader", "Radioman", "Medic", "Breacher", "Sniper"):
            text = _replace(text, f'"Jordanian Security Assualt {role}"', f'"Jordanian Security Assault {role}"')
    elif path == "lobosa6/config.cpp":
        text = _replace(text, 'displayNameMagazine="Hawk Missle";', 'displayNameMagazine="Hawk Missile";')
    elif path == "loboweapat/config.cpp":
        text = _replace(text, 'displayNameMagazine="Dragon Missle";', 'displayNameMagazine="Dragon Missile";')
    return text


def repair(documents: dict[str, str], compatibility: str = "ud") -> dict[str, str]:
    """Validate the entire known script set, returning only changed members."""
    if compatibility not in ("ud", "classic"):
        raise ValueError(f"Unknown script compatibility profile: {compatibility}")
    required = MISSILE_PATHS + FIRE_PATHS + NUMBER_PATHS + DISPLAY_PATHS
    missing = sorted(set(required) - documents.keys())
    if missing:
        raise ValueError("Missing required LoBo scripts/configs: " + ", ".join(missing))
    changed = {}
    for path in required:
        source = documents[path]
        newline = "\r\n" if "\r\n" in source else "\n"
        normalized = source.replace("\r\n", "\n")
        try:
            if path in MISSILE_PATHS:
                updated = _missile(normalized, path)
            elif path in FIRE_PATHS:
                updated = _fire(normalized, compatibility)
            elif path in NUMBER_PATHS:
                updated = _number(normalized)
            else:
                updated = _display(normalized, path)
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
