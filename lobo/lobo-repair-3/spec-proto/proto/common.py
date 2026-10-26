"""SPEC PROTOTYPE (repair 2026-10-11.3), scratch only: shared helpers for the
new rule modules. The real implementation belongs in
arma_CWR/tools/lobo/repair/. Reuses the shipped .1 helpers unchanged.
"""
from repair.scripts import _explicit_exit, _replace, script_findings  # noqa: F401

BS = chr(92)


def p(*parts):
    """Absolute OFP path string, e.g. p('LoBo_airammo', 'Scripts') -> \\LoBo_airammo\\Scripts."""
    return BS + BS.join(parts)


def split_newline(source):
    newline = "\r\n" if "\r\n" in source else "\n"
    return newline, source.replace("\r\n", "\n")


def line_rule(text, old, new, count, baseline=0):
    """Whole-line, idempotent substitution on LF-normalized text.

    `old` is one line or a tuple of alternative original lines that all become
    `new`. Accepts the original, repaired or partially repaired state only: the
    lines equal to any `old` plus the lines equal to `new` must be exactly
    `count` sites plus `baseline`, the number of lines that already equal `new`
    in the untouched original (other classes that were authored correctly).
    Exact line equality cannot match a longer line containing `old`.
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


def apply_rules(documents, rules, *, expected_findings=None, post=None):
    """rules: {member: ((kind, before, after, count), ...)} with kind 'sub' or 'line'.

    Every member must exist. .sqs results must have exactly the expected static
    findings (default: none). Original newline style is restored.
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
