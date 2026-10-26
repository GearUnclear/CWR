import sys
from pathlib import Path
sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
from repair.pbo import Archive
MOD = Path(r'D:/Arma_CWA/@LoBo/addons')
names = sys.argv[1:]
for n in names:
    p = next((q for q in MOD.glob('*.pbo') if q.stem.lower() == n.lower()), None)
    if not p:
        print(n, 'MISSING')
        continue
    a = Archive(p.read_bytes())
    e = a.by_key.get('config.cpp')
    print(f"{p.name:26s} stuffpbo={a.stuffpbo} sha1={a.checksummed} trailer={len(a.trailer)} config_packing={'none' if not e else hex(e.fields[0])}")
