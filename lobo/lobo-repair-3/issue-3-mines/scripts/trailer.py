import sys, hashlib
sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
from pathlib import Path
from repair.pbo import Archive
for p in sys.argv[1:]:
    raw = Path(p).read_bytes(); a = Archive(raw)
    print(Path(p).name, 'sha256', hashlib.sha256(raw).hexdigest(), 'checksummed', a.checksummed, 'stuffpbo', a.stuffpbo, 'trailer_len', len(a.trailer), 'members', len(a.entries), 'prefix', a.prefix[:80])
    for k in sorted(a.by_key):
        if k.endswith('.sqs') and any(s in k for s in ('mine','ladung','claymore')):
            e = a.by_key[k]; print('   ', k, 'packing', hex(e.fields[0]), 'size', e.fields[4], 'sha256', hashlib.sha256(a.read(k)).hexdigest()[:16])
