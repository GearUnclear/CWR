"""List texture-like path strings inside one pbo member (MLOD/ODOL) and check whether they resolve."""
import re, sys, subprocess
from pathlib import Path
from collections import Counter
sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
from repair.pbo import Archive

pbo = Path(sys.argv[1])
member = sys.argv[2]
a = Archive(pbo.read_bytes())
data = a.read(member, pbo, r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe')
paths = Counter(m.group().decode('latin1').lower() for m in re.finditer(rb'[\\A-Za-z0-9_\-. ]{3,}\.(?:pa[ac]|tga|jpg)', data))
for p, n in sorted(paths.items()):
    print(n, p)
