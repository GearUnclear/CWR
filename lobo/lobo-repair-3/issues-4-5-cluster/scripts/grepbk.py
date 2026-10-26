import sys
from pathlib import Path
sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
from repair.pbo import Archive
bk = Path(r'D:/Arma_CWA/@LoBo/.ud-backups/issue-69/20261011T105837Z-33d55270')
tools = Path(r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe')
for pbo, key in [('LoBo_A4.pbo','scripts/firemissiles_a2.sqs'),('LoBo_A4.pbo','scripts/firemissiles_a3.sqs'),('LoBo_F4.pbo','scripts/firemissiles_ww.sqs'),('LoBo_airammo.pbo','scripts/cbu/rbk500.sqs'),('LoBo_airammo.pbo','scripts/cbu/rockeye.sqs'),('LoboF5.pbo','scripts/rockeye.sqs')]:
    p = bk/pbo; a = Archive(p.read_bytes()); d = a.read(key, p, tools).decode('latin1')
    for i, line in enumerate(d.splitlines(), 1):
        if 'rockeye.sqs' in line or 'createvehicle' in line.lower(): print(pbo, key, i, line)
