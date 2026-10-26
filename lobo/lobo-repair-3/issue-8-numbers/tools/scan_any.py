"""Read-only: grep every text member of every @LoBo pbo (and loose text files)
for the given case-insensitive regexes. Prints member:line hits."""
import re
import sys
from pathlib import Path

sys.path.insert(0, r"D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo")
from repair.pbo import Archive  # noqa: E402

TOOLS = Path(r"D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe")
MOD = Path(r"D:/Arma_CWA/@LoBo")
TEXT = (".cpp", ".sqs", ".sqf", ".hpp", ".ext", ".sqm", ".h", ".csv", ".txt", ".fsm", ".bin")
pats = [re.compile(p, re.I) for p in sys.argv[1:]]
counts = {"pbos": 0, "members": 0, "binary_cfg": 0}
for path in sorted(MOD.rglob("*.pbo")):
    if ".ud-backups" in path.parts or "_ud-orig" in path.parts:
        continue
    counts["pbos"] += 1
    arc = Archive(path.read_bytes())
    for key in arc.by_key:
        if not key.endswith(TEXT):
            continue
        data = arc.read(key, path, TOOLS)
        counts["members"] += 1
        if key.endswith(".bin") or data[:4] == b"\0raP":
            counts["binary_cfg"] += 1
        text = data.decode("latin1")
        for i, line in enumerate(text.splitlines(), 1):
            if any(p.search(line) for p in pats):
                print(f"{path.relative_to(MOD)}:{key}:{i}: {line.strip()[:200]}")
for f in MOD.rglob("*"):
    if f.is_file() and f.suffix.lower() in TEXT and ".ud-backups" not in f.parts and "_ud-orig" not in f.parts:
        for i, line in enumerate(f.read_bytes().decode("latin1").splitlines(), 1):
            if any(p.search(line) for p in pats):
                print(f"LOOSE {f.relative_to(MOD)}:{i}: {line.strip()[:200]}")
print(counts)
