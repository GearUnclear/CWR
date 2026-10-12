"""Shared, fail-closed rule helpers for the 2026-10-11.3 repair modules.

Rules work on LF-normalized text and restore each member's own newline, as
scripts.py does. A rule accepts the original, the repaired and a partially
repaired state only; anything else raises before any archive is written.
"""
from .scripts import _replace, script_findings

BS = chr(92)


def p(*parts: str) -> str:
    """Absolute OFP path, e.g. p('LoBo_airammo', 'Scripts') gives a leading backslash path."""
    return BS + BS.join(parts)


def split_newline(source: str) -> tuple[str, str]:
    newline = "\r\n" if "\r\n" in source else "\n"
    return newline, source.replace("\r\n", "\n")


def line_rule(text: str, old, new: str, count: int, baseline: int = 0) -> str:
    """Whole-line, idempotent substitution on LF-normalized text.

    `old` is one line or a tuple of alternative original lines that all become
    `new`. The lines equal to any `old` plus the lines equal to `new` must be
    exactly `count` sites plus `baseline`, the number of lines that already
    equal `new` in the untouched original (classes authored correctly), and at
    least `baseline` lines must already equal `new`. Exact line equality cannot
    match a longer line that merely contains `old`.
    """
    olds = (old,) if isinstance(old, str) else tuple(old)
    if new in olds:
        raise ValueError("line_rule needs a change")
    lines = text.split("\n")
    present = lines.count(new)
    found = sum(lines.count(o) for o in olds) + present
    if found != count + baseline or present < baseline:
        raise ValueError(f"Expected {count} line(s) {olds!r} (+{baseline} existing {new!r}); "
                         f"found {found - present} original and {present} {new!r}")
    return "\n".join(new if line in olds else line for line in lines)


def apply_rules(documents: dict[str, str], rules: dict, expected_findings: dict | None = None,
                post: dict | None = None) -> dict[str, str]:
    """Apply {member: ((kind, before, after, count[, baseline]), ...)}.

    Kind 'sub' has scripts._replace semantics (never write a rule whose `after`
    is a substring of its `before`: it would read as already repaired); kind
    'line' is line_rule. Every member must exist. The optional post[member]
    runs after the rules. Every .sqs result must have exactly the expected
    static findings (default none). Returns only changed members.
    """
    expected_findings = expected_findings or {}
    post = post or {}
    missing = sorted(set(rules) - documents.keys())
    if missing:
        raise ValueError("Missing members: " + ", ".join(missing))
    changed = {}
    for path, items in rules.items():
        source = documents[path]
        newline, text = split_newline(source)
        updated = text
        try:
            for kind, before, after, count, *baseline in items:
                if kind == "sub":
                    if baseline:
                        raise ValueError("A 'sub' rule takes no baseline")
                    if after in before:
                        raise ValueError("A 'sub' rule must not be contained in its original")
                    updated = _replace(updated, before, after, count)
                elif kind == "line":
                    updated = line_rule(updated, before, after, count, *baseline)
                else:
                    raise ValueError(f"Unknown rule kind {kind}")
            if path in post:
                updated = post[path](updated)
            if path.endswith(".sqs"):
                found = script_findings(updated)
                if found != list(expected_findings.get(path, ())):
                    raise ValueError("Unexpected static findings: " + "; ".join(found))
        except ValueError as exc:
            raise ValueError(f"{path}: {exc}") from exc
        updated = updated.replace("\n", newline)
        if updated != source:
            changed[path] = updated
    return changed
