"""List CfgVehicles classes whose effective value of KEY comes from an unresolved string site."""
import json, re, sys
from pathlib import Path

OUT = Path(r'D:/Arma_CWA/tmp/lobo-repair-3/config-identifiers')
EX = OUT / 'extract'
JS = OUT / 'json'
KEY = sys.argv[1].lower()
IDENT = re.compile(r'[A-Za-z_]\w*')
SQF = {'true', 'false', 'pi', 'random'}


def strip(text):
    text = re.sub(r'/\*.*?\*/', ' ', text, flags=re.S)
    text = re.sub(r'//[^\n]*', ' ', text)
    text = re.sub(r'"(?:""|[^"\n])*"', '""', text)
    text = re.sub(r'^\s*#.*$', ' ', text, flags=re.M)
    return text


def bases_of(text):
    t = strip(text)
    m = re.search(r'\bclass\s+CfgVehicles\s*\{', t, re.I)
    if not m:
        return {}
    i = m.end(); depth = 1; out = {}
    for tok in re.finditer(r'\bclass\s+([A-Za-z_]\w*)\s*(?::\s*([A-Za-z_]\w*))?\s*([{;])|[{}]', t[i:]):
        s = tok.group(0)
        if s == '{':
            depth += 1
        elif s == '}':
            depth -= 1
            if depth == 0:
                break
        else:
            if depth == 1 and tok.group(2):
                out[tok.group(1).lower()] = tok.group(2).lower()
            if tok.group(3) == '{':
                depth += 1
    return out


classes = {}
base_text = (OUT / 'remaster-config.cpp').read_text(errors='replace')
s = base_text.find('class CfgVehicles'); e = base_text.find('\nclass ', s + 10)
for m in re.finditer(r'\n\tclass (\w+)(?:: (\w+))?', base_text[s:e]):
    classes[m.group(1).lower()] = {'name': m.group(1), 'base': (m.group(2) or '').lower() or None, 'val': None, 'origin': 'BASE'}
base_vars = set(re.findall(r'^\s+(\w+) = ', (OUT / 'remaster-config-vars.txt').read_text(), re.M))
for jp in sorted(JS.glob('*.json')):
    pbo = jp.stem
    data = json.loads(jp.read_text(encoding='utf-8', errors='replace'))
    cv = next((v for k, v in data.items() if k.lower() == 'cfgvehicles'), None)
    if not isinstance(cv, dict):
        continue
    cfgp = next((p for p in (EX / pbo).iterdir() if p.name.lower() == 'config.cpp'), None)
    bases = bases_of(cfgp.read_bytes().decode('latin1')) if cfgp else {}
    for cname, body in cv.items():
        if not isinstance(body, dict):
            continue
        low = cname.lower()
        own = next((v for k, v in body.items() if k.lower() == KEY and not isinstance(v, dict)), None)
        if low in classes and not bases.get(low) and own is None:
            continue
        ent = classes.get(low) or {'name': cname, 'base': None, 'val': None, 'origin': pbo}
        if bases.get(low):
            ent['base'] = bases[low]
        if own is not None:
            ent['val'] = own; ent['origin'] = pbo
        elif ent['origin'] == 'BASE':
            ent['origin'] = pbo
        classes[low] = ent


def unresolved(v):
    return isinstance(v, str) and any(w.lower() not in base_vars and w.lower() not in SQF for w in IDENT.findall(v))


def src(low, seen=()):
    if low not in classes or low in seen:
        return None
    e = classes[low]
    if e['val'] is not None:
        return low
    return src(e['base'], seen + (low,)) if e['base'] else None


rows = []
for low, e in classes.items():
    if e['origin'] == 'BASE':
        continue
    s_ = src(low)
    if s_ and unresolved(classes[s_]['val']):
        rows.append((e['origin'], e['name'], classes[s_]['name'], classes[s_]['val']))
for r in sorted(rows):
    print(*r, sep=' | ')
print('classes affected:', len(rows))
