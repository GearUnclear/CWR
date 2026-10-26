import sys,hashlib
sys.path.insert(0,r'D:/Arma_CWA/.claude/worktrees/lobo-repair-4/arma_CWR/tools/lobo')
from repair.pbo import Archive
from pathlib import Path
path=Path(sys.argv[1]);pat=[a.lower() for a in sys.argv[2:]]
raw=path.read_bytes();a=Archive(raw)
print(path.name,'sha256',hashlib.sha256(raw).hexdigest(),'prefix',a.prefix[:80],'trailer',len(a.trailer),'stuff',a.stuffpbo,'members',len(a.entries))
for e in a.entries:
    if not pat or any(p in e.key for p in pat):
        print(f'{e.key}\tpack={e.fields[0]:#x}\torig={e.fields[1]}\tsize={e.fields[4]}')
