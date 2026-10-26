"""Convert every ROOT config.cpp of @LoBo pbos to JSON with PoseidonTools and list
string values that contain identifiers which the engine evaluator could not resolve
(base CONFIG.BIN variable table + SQF nular/operator words).
"""
import json, re, subprocess, sys
from pathlib import Path
from collections import defaultdict, Counter

EX = Path(r'D:/Arma_CWA/tmp/lobo-repair-3/config-identifiers/extract')
OUT = Path(r'D:/Arma_CWA/tmp/lobo-repair-3/config-identifiers')
TOOLS = r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe'

base_vars = set()
for line in (OUT / 'remaster-config-vars.txt').read_text().splitlines():
    m = re.match(r'\s+(\w+) = (-?\d+)', line)
    if m:
        base_vars.add(m.group(1).lower())
base_vars.add('cpcommander')
SQF = {'true', 'false', 'pi', 'random', 'abs', 'sqrt', 'sin', 'cos', 'tan', 'deg', 'rad', 'mod', 'min', 'max',
       'floor', 'ceil', 'round', 'exp', 'ln', 'log', 'and', 'or', 'not', 'time', 'atan', 'asin', 'acos', 'atan2'}
IDENT = re.compile(r'[A-Za-z_]\w*')
NUMLIKE = re.compile(r'^[\sA-Za-z0-9_+\-*/().,]+$')

rows = []
fails = []
for pbo_dir in sorted(p for p in EX.iterdir() if p.is_dir()):
    cfg = None
    for name in ('config.cpp', 'Config.cpp', 'CONFIG.CPP', 'CONFIG.cpp'):
        if (pbo_dir / name).exists():
            cfg = pbo_dir / name
            break
    if not cfg:
        continue
    jpath = OUT / 'json' / (pbo_dir.name + '.json')
    jpath.parent.mkdir(exist_ok=True)
    r = subprocess.run([TOOLS, 'config', 'tojson', str(cfg), '-o', str(jpath)], capture_output=True)
    if r.returncode or not jpath.exists():
        fails.append((pbo_dir.name, r.returncode, (r.stderr or r.stdout).decode(errors='replace')[:300]))
        continue
    try:
        data = json.loads(jpath.read_text(encoding='utf-8', errors='replace'))
    except Exception as exc:
        fails.append((pbo_dir.name, 'json', str(exc)[:200]))
        continue

    def walk(node, path):
        if isinstance(node, dict):
            for k, v in node.items():
                walk(v, path + [k])
        elif isinstance(node, list):
            for i, v in enumerate(node):
                walk(v, path + [f'[{i}]'])
        elif isinstance(node, str):
            v = node.strip()
            if not v or '\\' in v or '"' in v or not NUMLIKE.match(v):
                return
            idents = [w for w in IDENT.findall(v)]
            if not idents:
                return
            unresolved = [w for w in idents if w.lower() not in base_vars and w.lower() not in SQF]
            key = next((p for p in reversed(path) if not p.startswith('[')), '?')
            rows.append({'pbo': pbo_dir.name, 'path': '/'.join(path), 'key': key, 'value': v,
                         'unresolved': unresolved, 'resolved_by_base': [w for w in idents if w.lower() in base_vars]})

    walk(data, [])

(OUT / 'json-string-values.json').write_text(json.dumps(rows, indent=1))
print('fails', len(fails))
for f in fails:
    print('  FAIL', f)
byk = defaultdict(Counter)
for r in rows:
    if r['unresolved']:
        byk[r['key'].lower()][r['value']] += 1
for k in sorted(byk, key=lambda k: -sum(byk[k].values())):
    print(f"{sum(byk[k].values()):5d} {k:24s} " + ' | '.join(f'{v}({n})' for v, n in byk[k].most_common(8)))
print('---- resolved by base var table, by key')
byb = defaultdict(Counter)
for r in rows:
    if r['resolved_by_base'] and not r['unresolved']:
        byb[r['key'].lower()][r['value']] += 1
for k in sorted(byb, key=lambda k: -sum(byb[k].values())):
    print(f"{sum(byb[k].values()):5d} {k:24s} " + ' | '.join(f'{v}({n})' for v, n in byb[k].most_common(8)))
