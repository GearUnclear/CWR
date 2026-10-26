"""Read-only scan: every text member in every @LoBo pbo (addons, missions) that
mentions a number script or a nums texture folder. Uses the worktree's Archive."""
import re
import sys
from pathlib import Path

sys.path.insert(0, r"D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo")
from repair.pbo import Archive  # noqa: E402

TOOLS = Path(r"D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe")
MOD = Path(r"D:/Arma_CWA/@LoBo")
TEXT = (".cpp", ".sqs", ".sqf", ".hpp", ".ext", ".sqm", ".h", ".csv", ".txt", ".fsm")
PAT = re.compile(r"[^\r\n]*number[^\r\n]*\.sqs[^\r\n]*", re.I)

pbos = sorted(MOD.rglob("*.pbo"))
pbos = [p for p in pbos if ".ud-backups" not in p.parts and "_ud-orig" not in p.parts]
for path in pbos:
    raw = path.read_bytes()
    try:
        arc = Archive(raw)
    except ValueError as exc:
        print("!! cannot parse", path, exc)
        continue
    for key, entry in arc.by_key.items():
        if not key.endswith(TEXT):
            continue
        try:
            data = arc.read(key, path, TOOLS)
        except ValueError as exc:
            print("!! cannot read", path.name, key, exc)
            continue
        text = data.decode("latin1")
        for i, line in enumerate(text.splitlines(), 1):
            if re.search(r"number2?\.sqs", line, re.I):
                print(f"{path.relative_to(MOD)}:{key}:{i}: {line.strip()[:220]}")
# loose files too
for f in MOD.rglob("*"):
    if f.is_file() and f.suffix.lower() in TEXT and ".ud-backups" not in f.parts:
        try:
            text = f.read_bytes().decode("latin1")
        except OSError:
            continue
        for i, line in enumerate(text.splitlines(), 1):
            if re.search(r"number2?\.sqs", line, re.I):
                print(f"LOOSE {f.relative_to(MOD)}:{i}: {line.strip()[:220]}")
