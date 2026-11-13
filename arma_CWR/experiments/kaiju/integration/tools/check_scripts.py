r"""A cheap static check of the script set before a game run (a script error ends a tri run).

    python check_scripts.py [folder ...]      default: ..\kj, ..\DedushkaDemo.Eden, ..\DedushkaLab.Eden, ..\tests

It knows nothing of the language beyond its brackets and strings:
  .sqf            brackets ( ) [ ] { } balanced over the whole file, strings closed
  .sqs, .test.sqf the same, per line (a statement is one line)
  every file      no control characters (a mangled "\r" or "\a" from a careless edit)
  .sqs            a `goto "label"` must name a label of the same file
Exit code 1 when anything is found.
"""
import os
import re
import sys

PAIRS = {")": "(", "]": "[", "}": "{"}


def strip_code(text, line_comment):
    """Returns text with strings and comments blanked, or raises ValueError."""
    out, i, n = [], 0, len(text)
    while i < n:
        c = text[i]
        if c == '"':
            j = i + 1
            while True:
                j = text.find('"', j)
                if j < 0:
                    raise ValueError("string not closed")
                if j + 1 < n and text[j + 1] == '"':  # doubled quote inside a string
                    j += 2
                    continue
                break
            if "\n" in text[i:j]:
                raise ValueError("string runs over a line end")
            out.append('""')
            i = j + 1
        elif line_comment and text.startswith("//", i):
            j = text.find("\n", i)
            i = n if j < 0 else j
        elif line_comment and text.startswith("/*", i):
            j = text.find("*/", i)
            i = n if j < 0 else j + 2
        else:
            out.append(c)
            i += 1
    return "".join(out)


def balance(code):
    stack = []
    for c in code:
        if c in "([{":
            stack.append(c)
        elif c in ")]}":
            if not stack or stack.pop() != PAIRS[c]:
                return "unbalanced %s" % c
    return "unclosed %s" % stack[-1] if stack else ""


def check(path):
    found = []
    raw = open(path, "rb").read()
    for m in re.finditer(rb"[\x00-\x08\x0b\x0c\x0e-\x1f]|\r(?!\n)", raw):
        line = raw.count(b"\n", 0, m.start()) + 1
        found.append("line %d: control character 0x%02x" % (line, raw[m.start()]))
    text = raw.decode("utf-8", "replace").replace("\r\n", "\n")
    low = path.lower()
    if low.endswith(".sqs") or low.endswith(".test.sqf"):
        labels = set()
        gotos = []
        for ln, line in enumerate(text.split("\n"), 1):
            s = line.strip()
            if low.endswith(".sqs"):
                if s.startswith(";") or not s:
                    continue
                if s.startswith("#"):
                    labels.add(s[1:].strip().lower())
                    continue
            elif s.startswith("//") or not s:
                continue
            try:
                code = strip_code(s, not low.endswith(".sqs"))
            except ValueError as e:
                found.append("line %d: %s" % (ln, e))
                continue
            b = balance(code)
            if b:
                found.append("line %d: %s" % (ln, b))
            for m in re.finditer(r'goto\s+"([^"]+)"', s):
                gotos.append((ln, m.group(1).lower()))
        if low.endswith(".sqs"):
            for ln, g in gotos:
                if g not in labels:
                    found.append("line %d: goto \"%s\" has no label" % (ln, g))
    elif low.endswith(".sqf"):
        try:
            b = balance(strip_code(text, True))
            if b:
                found.append(b)
        except ValueError as e:
            found.append(str(e))
    return found


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    roots = sys.argv[1:] or [os.path.join(here, "..", d) for d in ("kj", "DedushkaDemo.Eden", "DedushkaLab.Eden", "tests")]
    bad = 0
    count = 0
    for root in roots:
        for d, dirs, files in os.walk(root):
            # the staged copy of the set inside a mission is checked at its source
            if os.path.basename(d).lower().endswith(".eden"):
                dirs[:] = [x for x in dirs if x.lower() != "kj"]
            for f in files:
                if f.lower().endswith((".sqs", ".sqf")):
                    count += 1
                    p = os.path.join(d, f)
                    for msg in check(p):
                        bad += 1
                        print("%s: %s" % (os.path.relpath(p, os.path.join(here, "..")), msg))
    print("%d files checked, %d findings" % (count, bad))
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
