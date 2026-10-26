import sys, hashlib
from pathlib import Path
sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
from repair.pbo import Archive
for arg in sys.argv[1:]:
    p = Path(arg); raw = p.read_bytes(); a = Archive(raw)
    print('==', p, 'sha256', hashlib.sha256(raw).hexdigest()[:16], 'prefix', a.prefix[:200], 'checksummed', a.checksummed, 'trailer', a.trailer[:40])
    for k, e in a.by_key.items():
        if any(s in k for s in ('rockeye','rbk','cluster','fire_mk20','firemissiles_a2','firemissiles_a3','firemissiles_ww','config.cpp')):
            print('  ', k, e.fields)
