import sys
sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
from pathlib import Path
from repair.scripts import script_findings
root = Path(sys.argv[1])
for p in sorted(root.rglob('*.sqs')):
    f = script_findings(p.read_bytes().decode('latin1'))
    if f: print(p.relative_to(root).as_posix(), f)
