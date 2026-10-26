import sys, re
sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
from pathlib import Path
from repair.pbo import Archive
tool = Path(r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe')
pat = re.compile(r'(?im)^\s*~\s*0?\.0\d*\s*$')
refs = re.compile(r'(?i)(ap_mine|at\d_mine|navmine|mk118|ladung|leucht_mine|claymore|LoBo_Mines)')
for p in sorted(Path(r'D:/Arma_CWA/@LoBo/addons').glob('*.pbo')):
    try: a = Archive(p.read_bytes())
    except Exception as ex: print('ERR', p.name, ex); continue
    for key, e in a.by_key.items():
        if not key.endswith(('.sqs','.sqf','.cpp')): continue
        try: t = a.read(key, p, tool).decode('latin1')
        except Exception as ex: print('ERR', p.name, key, ex); continue
        hits = pat.findall(t)
        nearest = 'nearestobject' in t.lower()
        if hits and nearest: print('POLL', p.name, key, [h.strip() for h in hits])
        if p.stem.lower()!='lobo_mines' and refs.search(t): print('REF', p.name, key, sorted({m.group(0) for m in refs.finditer(t)}))
