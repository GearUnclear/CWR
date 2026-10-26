"""Run the CURRENT textures.repoint/_lods on the target models (read-only) to see what the
existing framework accepts and where the proposed extension is needed."""
import hashlib
import sys
from pathlib import Path

sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-4/arma_CWR/tools/lobo')
from repair import models, textures  # noqa: E402
from repair.pbo import Archive  # noqa: E402

TOOLS = r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe'
A = Path(r'D:/Arma_CWA/@LoBo/addons')
BS = chr(92)


def member(pbo, key):
    path = A / pbo
    return Archive(path.read_bytes()).read(key, path, TOOLS)


def b(*parts):
    return BS.join(parts).encode()


aim9 = member('LoBo_airammo.pbo', 'lobo_aim9.p3d')
fixed = textures.repoint(aim9, (1e13,), {b('rktf5', 'swtex.paa'): (b('LoBo_airammo', 'tex', 'sw.pac'), 14, False)})
print('aim9 current repoint ok', hashlib.sha256(fixed).hexdigest(), 'idempotent',
      textures.repoint(fixed, (1e13,), {b('rktf5', 'swtex.paa'): (b('LoBo_airammo', 'tex', 'sw.pac'), 14, False)}) == fixed)
epilot = member('LoBo_Pilot.pbo', 'lobo_epilot.p3d')
print('epilot lods', [round(l.resolution, 3) for l in models._lods(epilot)])
try:
    textures.repoint(epilot, (1.0, 5.0), {b('LoBo_Pilot', 'EAF', 'tex', 'back.pac'): (b('LoBo_Pilot', 'EAF', 'tex', 'back.paa'), 21, False)})
except ValueError as exc:
    print('epilot with one count per LOD fails as expected:', exc)
for key in ('lobo_terhilux.p3d', 'lobo_terhiluxmg.p3d'):
    try:
        models._lods(member('LoBoTer.pbo', key))
    except ValueError as exc:
        print(key, 'current _lods rejects:', exc)
