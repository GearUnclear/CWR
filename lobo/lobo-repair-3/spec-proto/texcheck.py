"""Compare dumped before/after models: byte diff count, texture names in touched LODs, idempotency."""
import sys
sys.dont_write_bytecode = True
sys.path.insert(0, '.')
sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
from proto import textures  # noqa: E402

KEYS = ('uh1ffarpod', 'uh1podend', 'ba_frt', 'ba_bk', 'crew', 'tankistae2', 'syr_crew')
for k in textures.MODEL_MEMBERS:
    b = open('dump/before/' + k, 'rb').read()
    a = open('dump/after/' + k, 'rb').read()
    diff = sum(1 for x, y in zip(a, b) if x != y)
    print(k, len(b), len(a), 'bytes differing', diff)
    for res, c in textures.textures(a):
        sel = {n.decode('latin1'): v for n, v in c.items() if any(s in n.decode('latin1').lower() for s in KEYS)}
        if sel:
            print('  ', res, sel)
    print('  idempotent', textures.repair_models({**{m: open('dump/after/' + m, 'rb').read() for m in textures.MODEL_MEMBERS}}) == {})
