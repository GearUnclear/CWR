import sys, collections
from pathlib import Path
sys.path.insert(0, 'D:/Arma_CWA/.claude/worktrees/lobo-repair-4/arma_CWR/tools/lobo')
from repair.pbo import Archive
from repair import textures, references
BS = chr(92)
TOOLS = Path('D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe')
A = Path('D:/Arma_CWA/@LoBo/addons')
B = Path('D:/Arma_CWA/@LoBo/.ud-backups/issue-69/20261012T085301Z-fdbc36a3')
names = {'loboter': 'LoBoTer.pbo', 'lobo_pilot': 'LoBo_Pilot.pbo', 'lobo_airammo': 'LoBo_airammo.pbo'}
for key in references.MODEL_MEMBERS:
    stem, member = key.split('/', 1)
    for label, root in (('before', B), ('after', A)):
        p = root / names[stem]
        arc = Archive(p.read_bytes())
        data = arc.read(member, p, TOOLS)
        trailer = references.PLANS[key][2]
        print('==', key, label, len(data), 'tail', data[-32:].hex() if trailer else '')
        for res, fields in textures.face_textures(data, trailer):
            c = collections.Counter(f.split(b'\0', 1)[0].lower() for f in fields)
            interesting = {k.decode('latin1'): v for k, v in c.items() if any(s in k for s in (b'matte', b'back', b'legsbk', b'swtex', b'sw.pac'))}
            missing = {}
            for k, v in c.items():
                n = k.decode('latin1').lstrip(BS).replace(BS, '/').lower()
                if n.startswith(stem + '/') and n.split('/', 1)[1] not in arc.by_key:
                    missing[n] = v
            print('   LOD %-10g faces %4d %s missing-in-addon %s' % (res, len(fields), interesting, missing))
