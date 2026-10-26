import sys, json
from pathlib import Path
sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
from repair.pbo import Archive
mod = Path(r'D:/Arma_CWA/@LoBo/addons')
out = Path(r'D:/Arma_CWA/tmp/lobo-repair-3/issues-4-5-cluster/dump')
tools = Path(r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe')
exts = ('.sqs','.sqf','.cpp','.hpp','.h','.ext','.sqm','.bin','.csv','.inc')
comp = []
for p in sorted(mod.glob('*.pbo')):
    raw = p.read_bytes()
    try:
        a = Archive(raw)
    except Exception as e:
        print('SKIP', p.name, e); continue
    for key, e in a.by_key.items():
        if not key.endswith(exts): continue
        if key.endswith('.bin') and not key.endswith('config.bin'): continue
        try:
            data = a.read(key, p, tools)
        except Exception as ex:
            comp.append((p.name, key, str(ex))); continue
        d = out / p.stem.lower() / key
        d.parent.mkdir(parents=True, exist_ok=True)
        d.write_bytes(data)
print(json.dumps(comp, indent=1))
