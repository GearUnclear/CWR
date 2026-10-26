import sys,hashlib
sys.path.insert(0,r'D:/Arma_CWA/.claude/worktrees/lobo-repair-4/arma_CWR/tools/lobo')
from repair.pbo import Archive
from pathlib import Path
path=Path(sys.argv[1]);tools=r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe'
a=Archive(path.read_bytes())
for k in sys.argv[2:]:
    d=a.read(k,path,tools)
    print(f'===== {k} bytes={len(d)} sha256={hashlib.sha256(d).hexdigest()} crlf={d.count(b"\r\n")} lf={d.count(b"\n")}')
    if '--raw' in sys.argv[2:]: pass
    sys.stdout.write(d.decode('latin1'))
    print()
