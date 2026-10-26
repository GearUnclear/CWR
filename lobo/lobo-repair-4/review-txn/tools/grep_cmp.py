import sys
from pathlib import Path
sys.path.insert(0, 'D:/Arma_CWA/.claude/worktrees/lobo-repair-4/arma_CWR/tools/lobo')
from repair.pbo import Archive
TOOLS = Path('D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe')
pats = [b'soundss', b'black_matte', b'rktf5', b'backfuse', b'_tunits select']
n = 0
for p in sorted(Path('D:/Arma_CWA/@LoBo/addons').glob('*.pbo')):
    a = Archive(p.read_bytes())
    for k, e in a.by_key.items():
        if e.fields[0] == 0: continue
        if not k.endswith(('.cpp', '.sqs', '.sqf', '.hpp', '.p3d')): continue
        n += 1
        try: d = a.read(k, p, TOOLS).lower()
        except Exception as ex: print(p.name, k, 'DECODE-ERR', str(ex)[:100]); continue
        for pat in pats:
            c = d.count(pat)
            if c: print(p.name, k, pat.decode(), c)
print('compressed scanned', n)
