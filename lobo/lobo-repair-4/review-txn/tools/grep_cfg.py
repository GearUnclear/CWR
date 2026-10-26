import sys, re
from pathlib import Path
sys.path.insert(0, 'D:/Arma_CWA/.claude/worktrees/lobo-repair-4/arma_CWR/tools/lobo')
from repair.pbo import Archive
pats = [b'soundss', b'black_matte', b'rktf5', b'backfuse', b'tailgun']
for p in sorted(Path('D:/Arma_CWA/@LoBo/addons').glob('*.pbo')):
    try: a = Archive(p.read_bytes())
    except Exception as e: print(p.name, 'ERR', e); continue
    for k, e in a.by_key.items():
        if e.fields[0] != 0: continue
        if not (k.endswith(('.cpp', '.sqs', '.sqf', '.hpp', '.h', '.ext', '.p3d'))): continue
        d = bytes(a.stored(e)).lower()
        for pat in pats:
            n = d.count(pat)
            if n: print(p.name, k, pat.decode(), n)
