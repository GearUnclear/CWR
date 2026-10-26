import sys
from pathlib import Path
sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
from repair.pbo import Archive
p = Path(sys.argv[1]); a = Archive(p.read_bytes())
for k, e in a.by_key.items():
    if any(s in k for s in sys.argv[2:]): print(k, e.fields)
