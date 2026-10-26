"""ed.py <file> <old file> <new file>: replace exactly one occurrence, keep LF."""
import sys
from pathlib import Path
p = Path(sys.argv[1])
old = Path(sys.argv[2]).read_bytes().decode("utf-8").replace("\r\n", "\n").rstrip("\n")
new = Path(sys.argv[3]).read_bytes().decode("utf-8").replace("\r\n", "\n").rstrip("\n")
t = p.read_bytes().decode("utf-8")
n = t.count(old)
if n != 1:
    raise SystemExit(f"{p.name}: {n} matches")
p.write_bytes(t.replace(old, new).encode("utf-8"))
print("ok", p.name)
