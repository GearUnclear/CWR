"""Read-only: compare installed number-script members against the .1 backup
originals, and report each target archive's member storage (packed or not)."""
import hashlib
import sys
from pathlib import Path

sys.path.insert(0, r"D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo")
from repair.pbo import Archive  # noqa: E402

TOOLS = Path(r"D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe")
MOD = Path(r"D:/Arma_CWA/@LoBo")
BACKUPS = sorted((MOD / ".ud-backups/issue-69").iterdir())
TARGETS = {
    "LoBoF15.pbo": ["scr/number.sqs"],
    "LoBoF16.pbo": ["scr/number.sqs", "scr/number2.sqs"],
    "LoBoIL28.pbo": ["script/number.sqs"],
    "LoBo_F4.pbo": ["scripts/number.sqs"],
}
h = lambda b: hashlib.sha256(b).hexdigest()
for name, keys in TARGETS.items():
    live = MOD / "addons" / name
    raw = live.read_bytes()
    arc = Archive(raw)
    print(f"{name}: sha256={h(raw)} size={len(raw)} checksummed={arc.checksummed} stuffpbo={arc.stuffpbo} trailer_len={len(arc.trailer)} prefix={bytes(arc.prefix)[:80]!r}")
    for key in keys:
        e = arc.by_key[key]
        data = arc.read(key, live, TOOLS)
        line = f"   {key}: packing={e.fields[0]:#x} size={len(data)} sha256={h(data)}"
        for b in BACKUPS:
            bp = b / name
            if bp.exists():
                barc = Archive(bp.read_bytes())
                bdata = barc.read(key, bp, TOOLS)
                line += f" | backup {b.name}: {'same' if bdata == data else 'DIFFERENT'}"
        print(line)
