"""Scan extracted @LoBo configs for unquoted non-numeric scalar/array values.

Mimics ParamFile GetWord: a scalar value is everything after '=' up to ';' '}'
or end of line (legacy newline termination), trimmed. Array elements split on
',' at depth. Strings in quotes, comments and preprocessor lines are handled.
Object-like #define names in the same file (and its includes) are expanded
token-wise, as the engine preprocessor would.
"""
import json, re, sys
from pathlib import Path
from collections import defaultdict, Counter

ROOT = Path(r'D:/Arma_CWA/tmp/lobo-repair-3/config-identifiers/extract')
OUT = Path(r'D:/Arma_CWA/tmp/lobo-repair-3/config-identifiers')

NUM = re.compile(r'^[+-]?(\d+\.?\d*([eE][+-]?\d+)?|\.\d+([eE][+-]?\d+)?|0x[0-9A-Fa-f]+)$')
IDENT = re.compile(r'^[A-Za-z_]\w*$')


def strip_comments(text):
    out = []
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if c == '"':
            j = i + 1
            while j < n:
                if text[j] == '"':
                    if j + 1 < n and text[j + 1] == '"':
                        j += 2
                        continue
                    break
                if text[j] in '\r\n':
                    break
                j += 1
            out.append(text[i:j + 1])
            i = j + 1
        elif text.startswith('//', i):
            j = text.find('\n', i)
            if j < 0:
                j = n
            i = j
        elif text.startswith('/*', i):
            j = text.find('*/', i + 2)
            if j < 0:
                j = n - 2
            seg = text[i:j + 2]
            out.append('\n' * seg.count('\n'))
            i = j + 2
        else:
            out.append(c)
            i += 1
    return ''.join(out)


def defines(text):
    d = {}
    fn = set()
    for m in re.finditer(r'^[ \t]*#[ \t]*define[ \t]+([A-Za-z_]\w*)(\([^)]*\))?[ \t]*(.*)$', text, re.M):
        if m.group(2):
            fn.add(m.group(1))
        else:
            d[m.group(1)] = m.group(3).strip()
    return d, fn


def class_path_at(text, pos):
    # crude: walk class openings up to pos
    stack = []
    for m in re.finditer(r'\bclass\s+([A-Za-z_]\w*)[^{;]*\{|\{|\}', text[:pos]):
        tok = m.group(0)
        if tok.startswith('class'):
            stack.append(m.group(1))
        elif tok == '{':
            stack.append(None)
        else:
            if stack:
                stack.pop()
    return '/'.join(s for s in stack if s)


def scan(path, text):
    clean = strip_comments(text)
    defs, fdefs = defines(clean)
    # remove preprocessor lines
    body = re.sub(r'^[ \t]*#.*$', '', clean, flags=re.M)
    hits = []
    for m in re.finditer(r'([A-Za-z_]\w*)\s*(\[\s*\])?\s*=\s*', body):
        key = m.group(1)
        is_array = bool(m.group(2))
        start = m.end()
        if is_array:
            if start >= len(body) or body[start] != '{':
                continue
            depth = 0
            j = start
            elems = []
            cur = []
            inq = False
            while j < len(body):
                c = body[j]
                if inq:
                    cur.append(c)
                    if c == '"':
                        inq = False
                    j += 1
                    continue
                if c == '"':
                    inq = True
                    cur.append(c)
                elif c == '{':
                    depth += 1
                    if depth > 1:
                        cur.append(c)
                elif c == '}':
                    depth -= 1
                    if depth == 0:
                        elems.append(''.join(cur))
                        break
                    cur.append(c)
                elif c == ',' and depth == 1:
                    elems.append(''.join(cur))
                    cur = []
                else:
                    cur.append(c)
                j += 1
            for idx, e in enumerate(elems):
                v = e.strip().strip('{}').strip()
                classify(path, body, m.start(), key + '[]', idx, v, defs, fdefs, hits)
        else:
            j = start
            while j < len(body) and body[j] not in ';}\n':
                if body[j] == '"':
                    k = body.find('"', j + 1)
                    j = k + 1 if k > 0 else len(body)
                    continue
                j += 1
            v = body[start:j].strip()
            classify(path, body, m.start(), key, None, v, defs, fdefs, hits)
    return hits, defs, fdefs


def expand(v, defs, depth=0):
    if depth > 8:
        return v
    if IDENT.match(v) and v in defs:
        return expand(defs[v], defs, depth + 1)
    return v


def classify(path, body, pos, key, idx, v, defs, fdefs, hits):
    if not v or v.startswith('"'):
        return
    ev = expand(v, defs)
    if NUM.match(ev) or ev.startswith('"'):
        return
    kind = 'ident' if IDENT.match(ev) else 'other'
    if kind == 'ident' and ev in fdefs:
        kind = 'fmacro'
    line = body.count('\n', 0, pos) + 1
    hits.append({'file': str(path.relative_to(ROOT)).replace('\\', '/'), 'line': line, 'key': key, 'idx': idx,
                 'value': v, 'expanded': ev, 'kind': kind, 'cls': class_path_at(body, pos)})


def main():
    allhits = []
    deftable = {}
    for p in sorted(ROOT.rglob('*')):
        if not p.is_file() or p.suffix.lower() not in ('.cpp', '.hpp', '.h', '.ext'):
            continue
        if p.name.endswith('.debin.cpp'):
            continue
        text = p.read_bytes().decode('latin1')
        try:
            hits, defs, fdefs = scan(p, text)
        except Exception as exc:
            print('FAIL', p, exc)
            continue
        deftable[str(p.relative_to(ROOT))] = sorted(defs)
        allhits.extend(hits)
    (OUT / 'unquoted-values.json').write_text(json.dumps(allhits, indent=1))
    c = Counter((h['key'], h['expanded']) for h in allhits if h['kind'] == 'ident')
    for (k, v), n in c.most_common():
        print(f'{n:5d} {k} = {v}')
    print('--- other unquoted (non-ident) by key')
    c2 = Counter(h['key'] for h in allhits if h['kind'] != 'ident')
    for k, n in c2.most_common(60):
        print(f'{n:5d} {k}')


main()
