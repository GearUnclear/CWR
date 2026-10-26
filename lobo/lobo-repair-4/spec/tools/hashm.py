import sys,hashlib
sys.path.insert(0,r'D:/Arma_CWA/.claude/worktrees/lobo-repair-4/arma_CWR/tools/lobo')
from repair.pbo import Archive
from pathlib import Path
tools=r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe'
for spec in sys.argv[1:]:
    pbo,k=spec.split('::');path=Path(pbo);a=Archive(path.read_bytes());d=a.read(k,path,tools)
    print(hashlib.sha256(d).hexdigest(),len(d),d[:16].hex(),path.stem+'/'+k)
