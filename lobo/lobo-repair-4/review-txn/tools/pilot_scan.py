import sys, collections
from pathlib import Path
sys.path.insert(0, 'D:/Arma_CWA/.claude/worktrees/lobo-repair-4/arma_CWR/tools/lobo')
from repair.pbo import Archive
from repair import textures
BS = chr(92)
TOOLS = Path('D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe')
p = Path(sys.argv[1]); stem = p.stem.lower()
arc = Archive(p.read_bytes())
for key in arc.by_key:
    if not key.endswith('.p3d'): continue
    data = arc.read(key, p, TOOLS)
    try:
        lods = textures.face_textures(data)
    except Exception as e:
        print(key, 'UNREADABLE', e); continue
    miss = collections.Counter()
    for res, fields in lods:
        for f in fields:
            n = f.split(b'\0', 1)[0].decode('latin1').lstrip(BS).replace(BS, '/').lower()
            if n.startswith(stem + '/') and n.split('/', 1)[1] not in arc.by_key:
                miss[(res, n)] += 1
    print(key, dict(miss) if miss else 'clean')
