"""Resolve git conflict hunks in a file by a list of choices.

usage: resolve.py FILE CHOICE [CHOICE ...]
CHOICE per hunk in order: o (ours), t (theirs), ot (ours then theirs),
to (theirs then ours), or @path (replacement text from a file), or
ot:SEP@path (ours, then the text of path, then theirs).
"""
import sys, re

path = sys.argv[1]
choices = sys.argv[2:]
src = open(path, encoding="utf-8", newline="").read()
nl = "\r\n" if "\r\n" in src else "\n"
lines = src.split(nl)
out = []
i = 0
h = 0
while i < len(lines):
    ln = lines[i]
    if ln.startswith("<<<<<<< "):
        ours, theirs = [], []
        i += 1
        while not lines[i].startswith("======="):
            ours.append(lines[i]); i += 1
        i += 1
        while not lines[i].startswith(">>>>>>> "):
            theirs.append(lines[i]); i += 1
        i += 1
        c = choices[h]
        h += 1
        if c == "o":
            out += ours
        elif c == "t":
            out += theirs
        elif c == "ot":
            out += ours + theirs
        elif c == "to":
            out += theirs + ours
        elif c.startswith("ot:"):
            mid = open(c[3:], encoding="utf-8").read().rstrip("\n").split("\n")
            out += ours + mid + theirs
        elif c.startswith("@"):
            rep = open(c[1:], encoding="utf-8").read().rstrip("\n").split("\n")
            out += rep
        else:
            raise SystemExit("bad choice " + c)
        continue
    out.append(ln)
    i += 1
if h != len(choices):
    raise SystemExit(f"{path}: {h} hunks, {len(choices)} choices")
open(path, "w", encoding="utf-8", newline="").write(nl.join(out))
print(f"{path}: resolved {h}")
