"""Compare the seven pinned icons across TheHKpack copies (read-only)."""
import hashlib
import sys
from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
sys.path.insert(0, '.')
from repair.pbo import Archive  # noqa: E402
from proto.weapons import ICONS  # noqa: E402

for f in sys.argv[1:]:
    raw = Path(f).read_bytes()
    a = Archive(raw)
    ok = []
    for key, (_, member, digest) in ICONS.items():
        e = a.by_key.get(member)
        ok.append(bool(e) and e.fields[0] == 0 and hashlib.sha256(bytes(a.stored(e))).hexdigest() == digest)
    print(hashlib.sha256(raw).hexdigest()[:16], len(raw), sum(ok), '/7 match', f)
