"""Resolve the effective CfgVehicles 'type' (VehicleKind) for every LoBo class,
before and after a literal-number repair, across the merged base + addon tree.
"""
import json, re
from pathlib import Path
from collections import Counter, defaultdict

OUT = Path(r'D:/Arma_CWA/tmp/lobo-repair-3/config-identifiers')
EX = OUT / 'extract'
JS = OUT / 'json'
MAP = {'vsoft': 0, 'varmor': 1, 'vair': 2, 'armor': 1}
KIND = {0: 'VSoft', 1: 'VArmor', 2: 'VAir'}


def strip(text):
    text = re.sub(r'/\*.*?\*/', ' ', text, flags=re.S)
    text = re.sub(r'//[^\n]*', ' ', text)
    text = re.sub(r'"(?:""|[^"\n])*"', '""', text)
    text = re.sub(r'^\s*#.*$', ' ', text, flags=re.M)
    return text


def cfgvehicles_bases(text):
    t = strip(text)
    m = re.search(r'\bclass\s+CfgVehicles\s*\{', t, re.I)
    if not m:
        return {}
    i = m.end()
    depth = 1
    bases = {}
    for tok in re.finditer(r'\bclass\s+([A-Za-z_]\w*)\s*(?::\s*([A-Za-z_]\w*))?\s*([{;])|[{}]', t[i:]):
        s = tok.group(0)
        if s == '{':
            depth += 1
        elif s == '}':
            depth -= 1
            if depth == 0:
                break
        else:
            if depth == 1:
                bases[tok.group(1).lower()] = (tok.group(1), (tok.group(2) or '').lower() or None, tok.group(3))
            if tok.group(3) == '{':
                depth += 1
    return bases


# base game
base_text = (OUT / 'remaster-config.cpp').read_text(errors='replace')
classes = {}  # lower -> dict(name, base, type, origin)
bb = cfgvehicles_bases(base_text)
# explicit base types
cur = None
start = base_text.find('class CfgVehicles')
for line in base_text[start:].splitlines():
    m = re.match(r'^\tclass (\w+)(?:: (\w+))?', line)
    if m:
        cur = m.group(1).lower()
        classes[cur] = {'name': m.group(1), 'base': (m.group(2) or '').lower() or None, 'type': None, 'origin': 'BASE'}
        continue
    if line.startswith('class '):
        break
    m = re.match(r'^\t\ttype=(\d+);', line)
    if m and cur:
        classes[cur]['type'] = int(m.group(1))

# addons in alphabetical order (approximation of merge order)
explicit = {}
for jp in sorted(JS.glob('*.json')):
    pbo = jp.stem
    data = json.loads(jp.read_text(encoding='utf-8', errors='replace'))
    cv = None
    for k, v in data.items():
        if k.lower() == 'cfgvehicles':
            cv = v
    if not isinstance(cv, dict):
        continue
    cfgp = next((p for p in (EX / pbo).iterdir() if p.name.lower() == 'config.cpp'), None)
    bases = cfgvehicles_bases(cfgp.read_bytes().decode('latin1')) if cfgp else {}
    for cname, body in cv.items():
        if not isinstance(body, dict):
            continue
        low = cname.lower()
        b = bases.get(low)
        is_decl = b is not None and b[2] == ';'
        if not body and (low in classes) and (b is None or b[1] is None):
            continue  # forward reference to an existing class
        entry = classes.get(low)
        if entry is None or entry['origin'] == 'BASE' and body:
            entry = {'name': cname, 'base': b[1] if b else (entry['base'] if entry else None), 'type': entry['type'] if entry and entry['origin'] == 'BASE' else None,
                     'origin': pbo}
            classes[low] = entry
        elif b and b[1]:
            entry['base'] = b[1]
        tv = None
        for k, v in body.items():
            if k.lower() == 'type' and not isinstance(v, dict):
                tv = v
        if tv is not None:
            entry['type'] = tv
            entry['origin'] = pbo
            if isinstance(tv, str):
                explicit[low] = (pbo, cname, tv)


def eff(low, fixed, seen=None):
    seen = seen or set()
    if low in seen or low not in classes:
        return None, None
    seen.add(low)
    e = classes[low]
    t = e['type']
    if t is not None:
        if isinstance(t, str):
            key = t.strip().lower()
            if fixed and key in MAP:
                return MAP[key], low
            return 0, low  # unresolved -> nil -> 0
        return int(t), low
    if e['base']:
        return eff(e['base'], fixed, seen)
    return None, None


changes = defaultdict(list)
for low, e in classes.items():
    if e['origin'] == 'BASE':
        continue
    before, src = eff(low, False)
    after, _ = eff(low, True)
    if before != after:
        changes[e['origin']].append((e['name'], KIND.get(before), KIND.get(after), classes[src]['name'] if src else None))

total = 0
for pbo in sorted(changes):
    print(f'== {pbo}: {len(changes[pbo])} classes change kind')
    for name, b, a, src in sorted(changes[pbo]):
        print(f'   {name:34s} {b} -> {a}   (type site in {src})')
    total += len(changes[pbo])
print('TOTAL classes whose kind changes:', total)
print('explicit unresolved string sites:', Counter(v[2].strip('"') for v in explicit.values()))
json.dump({k: v for k, v in changes.items()}, open(OUT / 'kind-changes.json', 'w'), indent=1)
