import sys
from pathlib import Path
sys.path.insert(0, 'D:/Arma_CWA/.claude/worktrees/lobo-repair-4/arma_CWR/tools/lobo')
from repair.pbo import Archive
TOOLS = Path('D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe')
B=bytes([92])
pats = [b'soundss', b'black_matte', b'rktf5'] + [B.join(x.encode().split(b'/')) for x in ('eaf/tex/back.pac','eaf/tex/legsbk.pac','rjaf/tex/back.pac','rjaf/tex/legsbk.pac','tex2/legsbk.pac')]
for p in sorted(Path('D:/Arma_CWA/@LoBo/.ud-backups/issue-69/20261012T085301Z-fdbc36a3').glob('*.pbo')):
    a = Archive(p.read_bytes())
    for k, e in a.by_key.items():
        if k.endswith(('.paa', '.pac', '.wss', '.ogg', '.wav', '.jpg')): continue
        if e.fields[0] == 0: d = bytes(a.stored(e)).lower()
        elif k.endswith(('.bin', '.cpp', '.sqs', '.sqf', '.p3d', '.hpp', '.ext', '.h')):
            try: d = a.read(k, p, TOOLS).lower()
            except Exception: continue
        else: continue
        for pat in pats:
            c = d.count(pat)
            if c: print(p.name, k, pat.decode(), c)
print('done')
