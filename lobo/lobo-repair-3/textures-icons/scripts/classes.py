"""Collect top-level classes per section (CfgPatches/CfgWeapons/CfgAmmo/CfgVehicles...)
from a config and report collisions against other configs that DEFINE (body or
inheritance) the same class.

Usage: classes.py new_config.cpp other_config.cpp [...]
"""
import re, sys


def defs(path):
    text = open(path, encoding='latin-1').read()
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
    text = re.sub(r'//[^\n]*', '', text)
    out = {}
    depth = 0
    section = None
    i = 0
    for m in re.finditer(r'class\s+(\w+)\s*(:\s*\w+)?\s*([{;])|[{}]', text):
        tok = m.group(0)
        if tok == '{':
            depth += 1; continue
        if tok == '}':
            depth -= 1
            if depth == 0:
                section = None
            continue
        name, base, end = m.group(1), m.group(2), m.group(3)
        if depth == 0 and end == '{':
            section = name.lower()
        elif depth == 1 and section:
            kind = 'body' if end == '{' else ('decl' if not base else 'inherit-decl')
            out.setdefault((section, name.lower()), set()).add(kind if not base else kind + ':' + base.strip(': ').lower())
        if end == '{':
            depth += 1
    return out


new = defs(sys.argv[1])
print('new classes', len(new))
for other in sys.argv[2:]:
    o = defs(other)
    for key in sorted(set(new) & set(o)):
        nk = new[key]
        ok = o[key]
        if any(k.startswith('body') for k in nk) and any(k.startswith('body') for k in ok):
            print(f'COLLISION {key[0]}::{key[1]}  new={sorted(nk)}  {other.split("/")[-2]}={sorted(ok)}')
