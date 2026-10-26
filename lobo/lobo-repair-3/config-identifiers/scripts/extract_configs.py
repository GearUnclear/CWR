"""Extract every config.cpp / config.bin (and other text members of interest) from @LoBo pbos.

Read-only on the mod. Writes under OUT.
"""
import sys, subprocess, json
from pathlib import Path
sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
from repair.pbo import Archive

MOD = Path(r'D:/Arma_CWA/@LoBo/addons')
OUT = Path(r'D:/Arma_CWA/tmp/lobo-repair-3/config-identifiers/extract')
TOOLS = Path(r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe')
TEXT_EXT = ('.cpp', '.hpp', '.h', '.sqs', '.sqf', '.ext', '.bin', '.csv', '.txt', '.inc', '.cfg', '.sqm')

index = {}
for pbo in sorted(MOD.glob('*.pbo')):
    data = pbo.read_bytes()
    try:
        a = Archive(data)
    except Exception as exc:
        index[pbo.name] = {'error': str(exc)}
        continue
    members = []
    for e in a.entries:
        name = e.name.decode('latin1')
        low = name.lower()
        if not low.endswith(TEXT_EXT):
            continue
        try:
            body = a.read(e.key, pbo, TOOLS)
        except Exception as exc:
            members.append({'name': name, 'error': str(exc)})
            continue
        dest = OUT / pbo.stem / name.replace('\\', '/')
        dest.parent.mkdir(parents=True, exist_ok=True)
        dest.write_bytes(body)
        info = {'name': name, 'size': len(body), 'packing': e.fields[0]}
        if low.endswith('.bin') and body[:4] == b'\0raP':
            txt = dest.with_suffix('.debin.cpp')
            r = subprocess.run([str(TOOLS), 'config', 'debin', str(dest), '-o', str(txt)], capture_output=True)
            info['debin_rc'] = r.returncode
            if r.returncode:
                info['debin_err'] = (r.stderr or r.stdout).decode(errors='replace')[:400]
        members.append(info)
    index[pbo.name] = {'members': members}

(OUT / 'index.json').write_text(json.dumps(index, indent=1))
print('pbos', len(index))
