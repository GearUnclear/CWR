import sys
sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
from pathlib import Path
from repair.pbo import Archive
src = Path(sys.argv[1]); out = Path(sys.argv[2])
tool = Path(r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe')
raw = src.read_bytes(); a = Archive(raw)
for key, e in a.by_key.items():
    if key.endswith(('.sqs','.sqf','.cpp','.bin','.ext','.hpp','.csv')):
        data = a.read(key, src, tool)
        p = out / key; p.parent.mkdir(parents=True, exist_ok=True); p.write_bytes(data)
        print(key, len(data), 'compressed' if e.fields[0] else '')
