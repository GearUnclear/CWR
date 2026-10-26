"""Raw case-insensitive byte scan of every member (decoded when compressed) of
pbos under the given directories for the given names. Read-only."""
import re
import sys
from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
from repair.pbo import Archive  # noqa: E402

TOOLS = Path(r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe')
names = [n.encode() for n in ('M1Smoker', 'M60Smoker', 'KioGlowFire', 'LoBo_WreckGlow')]
rx = re.compile(b'(?i)(' + b'|'.join(re.escape(n) for n in names) + rb'|\bSmoker\b)')
for d in sys.argv[1:]:
    for p in sorted(Path(d).glob('*.pbo')):
        a = Archive(p.read_bytes())
        for key, e in a.by_key.items():
            if not key.endswith(('.bin', '.cpp', '.hpp', '.sqs', '.sqf')):
                continue
            try:
                data = a.read(key, p, TOOLS)
            except Exception as exc:
                print('ERR', p.name, key, exc)
                continue
            hits = sorted({m.group().decode('latin1') for m in rx.finditer(data)})
            if hits:
                print(p.name, key, hits)
print('done')
