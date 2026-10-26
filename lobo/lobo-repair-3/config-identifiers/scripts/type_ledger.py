"""Ledger of unresolved numeric-identifier sites in @LoBo root configs with source line text.

Uses the JSON scan (macro-expanded, engine parser) to identify unresolved values,
then locates the exact source line in the extracted root config.cpp.
"""
import json, re, sys
from pathlib import Path
from collections import defaultdict, Counter

sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
from repair import config as C

EX = Path(r'D:/Arma_CWA/tmp/lobo-repair-3/config-identifiers/extract')
OUT = Path(r'D:/Arma_CWA/tmp/lobo-repair-3/config-identifiers')
rows = json.load(open(OUT / 'json-string-values.json'))
NUMERIC_KEYS = {'type', 'speed', 'duty', 'canlock', 'access', 'weapontype', 'unitinfotype', 'hascommander',
                'drivercansee', 'gunnercansee', 'pilotcansee', 'commandercansee', 'getinaction', 'getoutaction'}


def root_cfg(pbo):
    for name in ('config.cpp', 'Config.cpp', 'CONFIG.CPP', 'CONFIG.cpp'):
        p = EX / pbo / name
        if p.exists():
            return p


def class_bases(text):
    """name(lower) -> base for every class definition (any depth)."""
    out = {}
    for m in re.finditer(r'\bclass\s+([A-Za-z_]\w*)\s*(?::\s*([A-Za-z_]\w*))?\s*\{', text):
        out.setdefault(m.group(1).lower(), m.group(2))
    return out


ledger = []
for r in rows:
    key = r['key'].lower()
    if key not in NUMERIC_KEYS or not r['unresolved']:
        continue
    parts = r['path'].split('/')
    if key == 'type' and not (len(parts) == 3 and parts[0].lower() == 'cfgvehicles'):
        continue  # animation 'type' strings etc.
    if key in ('speed', 'duty') and parts[0].lower() != 'cfgmovesmc':
        continue
    cfg = root_cfg(r['pbo'])
    raw = cfg.read_bytes()
    text = raw.decode('latin1')
    # locate the class block (case-insensitive path) and the assignment inside its direct body
    try:
        start, end = C.class_block(text, parts[:-1])
    except Exception as exc:
        ledger.append({**r, 'error': str(exc)})
        continue
    body = text[start:end]
    # only direct members: blank out nested class bodies
    ts = C.tokens(body)
    hits = []
    depth = 0
    i = 0
    while i < len(ts):
        t = ts[i].text
        if t == '{':
            depth += 1
        elif t == '}':
            depth -= 1
        elif depth == 0 and t.lower() == parts[-1].lower():
            vs = C.assignment_at(ts, i)
            if vs is not None:
                ve = C.value_end(body, ts, vs)
                a = ts[i].start
                b = ts[ve].end if ve < len(ts) and ts[ve].text == ';' else ts[ve - 1].end
                hits.append((start + a, start + b))
        i += 1
    if len(hits) != 1:
        ledger.append({**r, 'error': f'{len(hits)} source sites'})
        continue
    a, b = hits[0]
    line_start = text.rfind('\n', 0, a) + 1
    line_end = text.find('\n', b)
    if line_end < 0:
        line_end = len(text)
    lineno = text.count('\n', 0, a) + 1
    bases = class_bases(text)
    cls = parts[-2]
    ledger.append({**r, 'file': cfg.name, 'line': lineno, 'site': text[a:b], 'line_text': text[line_start:line_end].rstrip('\r'),
                   'offset': a, 'length': b - a, 'base': bases.get(cls.lower())})

(OUT / 'numeric-identifier-ledger.json').write_text(json.dumps(ledger, indent=1))
for e in sorted(ledger, key=lambda e: (e['key'].lower(), e['pbo'], e.get('line', 0))):
    if 'error' in e:
        print('ERR', e['pbo'], e['path'], e['error'])
        continue
    print(f"{e['key']:14s} {e['pbo']:16s} {e['file']}:{e['line']:<5d} {e['path']:60s} base={e['base']} site={e['site']!r}")
