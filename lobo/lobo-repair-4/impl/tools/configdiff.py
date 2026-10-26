"""Read-only: extract the .4 configs before (backup) and after (installed), parse both with
PoseidonTools config tojson, and list every differing JSON leaf."""
import json
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-4/arma_CWR/tools/lobo')
from repair.pbo import Archive  # noqa: E402

TOOLS = r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe'
BATCH = Path(sys.argv[1])
OUT = Path(sys.argv[2])
ADDONS = Path(r'D:/Arma_CWA/@LoBo/addons')
TARGETS = {'LoBoMig.pbo': 'config.cpp', 'LoBo_Pilot.pbo': 'config.cpp'}


def leaves(node, path=()):
    if isinstance(node, dict):
        for k, v in node.items():
            yield from leaves(v, path + (k,))
    elif isinstance(node, list):
        for i, v in enumerate(node):
            yield from leaves(v, path + (i,))
    else:
        yield path, node


result = {}
for pbo, member in TARGETS.items():
    jsons = {}
    for state, path in (('before', BATCH / pbo), ('after', ADDONS / pbo)):
        data = Archive(path.read_bytes()).read(member, path, TOOLS)
        folder = OUT / state / pbo[:-4]
        folder.mkdir(parents=True, exist_ok=True)
        (folder / member).write_bytes(data)
        run = subprocess.run([TOOLS, 'config', 'tojson', str(folder / member), '-o', str(folder / 'config.json')],
                             capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(f'tojson failed for {state} {pbo}: {run.returncode} {run.stderr}')
        jsons[state] = json.loads((folder / 'config.json').read_text(encoding='utf8', errors='replace'))
    before, after = dict(leaves(jsons['before'])), dict(leaves(jsons['after']))
    diff = [{'path': '/'.join(map(str, k)), 'before': before.get(k), 'after': after.get(k)}
            for k in sorted(set(before) | set(after), key=lambda x: tuple(map(str, x)))
            if before.get(k, object()) != after.get(k, object())]
    result[pbo] = {'leaves_before': len(before), 'leaves_after': len(after), 'differences': diff}
(OUT / 'configdiff.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf8')
for pbo, r in result.items():
    print(pbo, 'leaves', r['leaves_before'], r['leaves_after'], 'differences', len(r['differences']))
    for d in r['differences']:
        print('  ', d['path'], '|', d['before'], '->', d['after'])
