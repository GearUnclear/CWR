"""Static resolver for file references in root addon configs.

Usage: cfgrefs.py <scriptsdir> <tools.exe> out.json idx.tsv[,idx...] pbo [pbo...]

Only root config.cpp/config.bin are read (AddonSystem::ParseAddonConfig loads
nothing else). For picture/icon the engine applies GetPictureName (leading
backslash = absolute, else data\\ prefix, default .paa). model/modelOptics use
GetShapeName (data3d\\, .p3d). Other quoted strings ending in a texture/model
extension are reported as 'literal'.
"""
import json, os, re, subprocess, sys, tempfile
sys.path.insert(0, sys.argv[1])
import pboscan  # noqa: E402  (reuses header + lzss)

TOOLS = sys.argv[2]
OUT = sys.argv[3]
index = {}
prefixes = set()
for fn in sys.argv[4].split(','):
    for line in open(fn, encoding='utf-8'):
        p, pre, name, m, o, s = line.rstrip('\n').split('\t')
        key = (pre + '\\' + name).lower().replace('/', '\\')
        index.setdefault(key, []).append(p)
        prefixes.add(pre.lower())


def default_name(value, ddir, dext):
    v = value.strip()
    if not v:
        return None
    if v[0] in '\\/':
        v = v[1:]
    elif not v.lower().startswith(ddir.rstrip('\\')):
        v = ddir + v
    leaf = v.replace('/', '\\').rsplit('\\', 1)[-1]
    if '.' not in leaf:
        v += dext
    return v.replace('/', '\\')


def resolve(path):
    key = path.lower()
    if key in index:
        return 'found', index[key][0]
    first = key.split('\\', 1)[0]
    if first not in prefixes:
        return 'missing-prefix', first
    return 'missing-member', first


def config_text(pbo):
    data = open(pbo, 'rb').read()
    pos, entries = pboscan.header(data)
    for name, method, orig, size in entries:
        body = data[pos:pos + size]
        pos += size
        low = name.lower()
        if low in ('config.cpp', 'config.bin'):
            if method == pboscan.CPRS and orig:
                body = pboscan.lzss(body, orig)
            if low == 'config.cpp':
                return name, body.decode('latin-1')
            with tempfile.TemporaryDirectory() as td:
                src = os.path.join(td, 'config.bin')
                dst = os.path.join(td, 'config.cpp')
                open(src, 'wb').write(body)
                r = subprocess.run([TOOLS, 'config', 'debin', src, dst], capture_output=True)
                if r.returncode or not os.path.exists(dst):
                    r = subprocess.run([TOOLS, 'config', 'convert', src, dst], capture_output=True)
                if not os.path.exists(dst):
                    return name, None
                return name, open(dst, 'rb').read().decode('latin-1')
    return None, None


KEYRE = re.compile(r'(?im)^[ \t]*(picture|icon|model|modelOptics|texture|gunnerOpticsModel|driverOpticsModel|commanderOpticsModel)\s*=\s*("([^"]*)"|([^;\s]*))\s*;')
LITRE = re.compile(r'"([^"\r\n]*\.(?:paa|pac|jpg|p3d))"', re.I)
results = []
for pbo in sys.argv[5:]:
    cname, text = config_text(pbo)
    short = os.path.basename(pbo)
    if text is None:
        results.append(dict(pbo=short, config=cname, status='no-config-or-unreadable'))
        continue
    # strip // comments (keeps line numbers)
    lines = text.split('\n')
    clean = '\n'.join(re.sub(r'//.*$', '', l) for l in lines)
    seen = set()
    for m in KEYRE.finditer(clean):
        key = m.group(1)
        raw = m.group(3) if m.group(3) is not None else m.group(4)
        line = clean.count('\n', 0, m.start()) + 1
        if key.lower() in ('picture', 'icon'):
            path = default_name(raw, 'data\\', '.paa') if raw else None
        elif key.lower() in ('model', 'modeloptics', 'gunneropticsmodel', 'driveropticsmodel', 'commanderopticsmodel'):
            path = default_name(raw, 'data3d\\', '.p3d') if raw else None
        else:
            path = default_name(raw, 'data\\', '.paa') if raw else None
        if not path:
            continue
        st, where = resolve(path)
        seen.add(m.start())
        results.append(dict(pbo=short, config=cname, line=line, key=key, value=raw, path=path, status=st, where=where))
    for m in LITRE.finditer(clean):
        raw = m.group(1)
        if raw[:1] not in '\/':
            continue
        line = clean.count('\n', 0, m.start()) + 1
        path = raw[1:] if raw[:1] in '\\/' else raw
        st, where = resolve(path.replace('/', '\\'))
        results.append(dict(pbo=short, config=cname, line=line, key='literal', value=raw, path=path, status=st, where=where))
json.dump(results, open(OUT, 'w'), indent=1)
bad = [r for r in results if r.get('status') not in ('found',)]
for r in bad:
    print(f"{r['pbo']:<22} {r.get('line','')!s:>5} {r.get('key',''):<12} {r.get('status'):<16} {r.get('value','')}")
print('refs', len(results), 'unresolved', len(bad))
