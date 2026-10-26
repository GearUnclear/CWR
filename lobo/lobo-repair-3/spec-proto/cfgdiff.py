"""Parse before/after config.cpp with PoseidonTools (engine preprocessor+parser),
flatten the JSON and print every changed path. Usage: cfgdiff.py DUMPDIR"""
import json
import subprocess
import sys
import tempfile
from pathlib import Path

TOOLS = r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe'


def parse(path):
    with tempfile.TemporaryDirectory() as d:
        out = Path(d) / 'o.json'
        r = subprocess.run([TOOLS, 'config', 'tojson', str(path), '-o', str(out)], capture_output=True, text=True)
        if r.returncode or not out.exists():
            r2 = subprocess.run([TOOLS, 'config', 'tojson', str(path)], capture_output=True, text=True)
            if r2.returncode:
                return None, (r.stderr + r2.stderr)[-400:]
            return json.loads(r2.stdout), ''
        return json.loads(out.read_text(encoding='utf8', errors='replace')), ''


def flat(node, prefix, out):
    if isinstance(node, dict):
        for k, v in node.items():
            flat(v, prefix + '/' + str(k), out)
    elif isinstance(node, list):
        out[prefix] = json.dumps(node)
    else:
        out[prefix] = json.dumps(node)
    return out


root = Path(sys.argv[1])
for before in sorted((root / 'before').rglob('config.cpp')):
    rel = before.relative_to(root / 'before')
    after = root / 'after' / rel
    b, eb = parse(before)
    a, ea = parse(after)
    if b is None or a is None:
        print('==', rel, 'PARSE FAILED', 'before:', eb, 'after:', ea)
        continue
    fb, fa = flat(b, '', {}), flat(a, '', {})
    keys = sorted(set(fb) | set(fa))
    diff = [(k, fb.get(k), fa.get(k)) for k in keys if fb.get(k) != fa.get(k)]
    print('==', rel, len(diff), 'changed paths')
    for k, x, y in diff[:80]:
        print('   ', k, ':', x, '->', y)
