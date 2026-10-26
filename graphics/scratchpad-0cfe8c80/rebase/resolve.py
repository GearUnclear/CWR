"""Resolve git conflict hunks: resolve.py <file> <spec> [<spec>...]
spec per hunk in order: ours | theirs | both | @<path to a text file with the replacement>"""
import sys
from pathlib import Path

p = Path(sys.argv[1])
specs = sys.argv[2:]
raw = p.read_bytes()
crlf = b"\r\n" in raw
text = raw.decode("utf-8")
lines = text.split("\n")
out = []
i = 0
h = 0
while i < len(lines):
    ln = lines[i]
    if ln.startswith("<<<<<<<"):
        ours, theirs = [], []
        i += 1
        while not lines[i].startswith("======="):
            ours.append(lines[i]); i += 1
        i += 1
        while not lines[i].startswith(">>>>>>>"):
            theirs.append(lines[i]); i += 1
        i += 1
        s = specs[h]
        h += 1
        if s == "ours":
            out += ours
        elif s == "theirs":
            out += theirs
        elif s == "both":
            out += ours + theirs
        elif s.startswith("@"):
            rep = Path(s[1:]).read_text(encoding="utf-8").rstrip("\n").split("\n")
            if crlf:
                rep = [r.rstrip("\r") + "\r" for r in rep]
            out += rep
        else:
            raise SystemExit("bad spec " + s)
        continue
    out.append(ln)
    i += 1
if h != len(specs):
    raise SystemExit(f"{p}: {h} hunks, {len(specs)} specs")
p.write_bytes("\n".join(out).encode("utf-8"))
print(p.name, "resolved", h)
