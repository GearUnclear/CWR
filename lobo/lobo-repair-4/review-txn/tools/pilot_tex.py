import sys, collections
from pathlib import Path
sys.path.insert(0, 'D:/Arma_CWA/.claude/worktrees/lobo-repair-4/arma_CWR/tools/lobo')
from repair.pbo import Archive
from repair import textures
TOOLS = Path('D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe')
p = Path('D:/Arma_CWA/@LoBo/addons/LoBo_Pilot.pbo')
a = Archive(p.read_bytes())
for key in ('lobo_iafpilot.p3d', 'lobo_iafpilot_grey.p3d', 'lobo_iafpilot2.p3d', 'lobo_jpilot.p3d', 'lobo_spilot.p3d'):
    d = a.read(key, p, TOOLS)
    for tail in (None, d[-32:]):
        try:
            lods = textures.face_textures(d, tail); break
        except ValueError as e:
            err = e
    c = collections.Counter()
    for res, fields in lods:
        for f in fields:
            n = f.split(b'\0', 1)[0].decode('latin1').lower()
            if 'back' in n or 'legsbk' in n: c[(res, n)] += 1
    print(key, 'tail', d[-32:][:16].hex()); [print('   ', k, v) for k, v in sorted(c.items())]
