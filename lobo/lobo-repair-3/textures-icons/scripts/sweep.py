"""Sweep every .p3d member in the given pbos: MLOD live face textures per LOD,
resolved against the index files. Reports unresolved live textures with LOD
kind and face counts, plus which models are referenced from root configs.

Usage: sweep.py <scriptsdir> out.json idx[,idx...] cfgdir pbo [pbo...]
"""
import json, os, re, struct, sys
sys.path.insert(0, sys.argv[1])
import pboscan
import mlodtex

B = chr(92)
index = set()
prefixes = set()
for fn in sys.argv[3].split(','):
    for line in open(fn, encoding='utf-8'):
        p, pre, name, *_ = line.rstrip('\n').split('\t')
        index.add((pre + B + name).lower().replace('/', B))
        prefixes.add(pre.lower())

# model references in root configs (model=, modelOptics, proxies are separate)
cfgtext = ''
for root, _, files in os.walk(sys.argv[4]):
    for f in files:
        cfgtext += open(os.path.join(root, f), encoding='latin-1').read().lower() + '\n'

out = []
odol = []
for pbo in sys.argv[5:]:
    data = open(pbo, 'rb').read()
    stem = os.path.splitext(os.path.basename(pbo))[0]
    pos, entries = pboscan.header(data)
    for name, method, orig, size in entries:
        body = data[pos:pos + size]
        pos += size
        if not name.lower().endswith('.p3d'):
            continue
        if method == pboscan.CPRS and orig:
            body = pboscan.lzss(body, orig)
        sig = body[:4]
        if sig != b'MLOD':
            odol.append((stem, name, sig.decode('latin-1')))
            continue
        try:
            ls, end, sz = mlodtex.lods(body)
        except Exception as e:
            out.append(dict(pbo=stem, model=name, error=str(e)))
            continue
        modelpath = (stem + B + name[:-4]).lower()
        referenced = modelpath in cfgtext or modelpath.replace(B, B + B) in cfgtext
        for l in ls:
            counts = {}
            for off, tname, raw in l['textures']:
                if tname:
                    counts[tname] = counts.get(tname, 0) + 1
            for tname, n in counts.items():
                key = tname.lower().replace('/', B)
                if key.startswith(B):
                    key = key[1:]
                if key.startswith('#'):
                    continue
                if key in index:
                    continue
                first = key.split(B, 1)[0]
                out.append(dict(pbo=stem, model=name, referenced_in_cfg=referenced, lod=l['index'],
                                kind=mlodtex.resname(l['resolution']), texture=tname, faces=n,
                                status='missing-prefix' if first not in prefixes else 'missing-member'))
json.dump(dict(missing=out, odol=odol), open(sys.argv[2], 'w'), indent=1)
agg = {}
for r in out:
    if 'error' in r:
        print('ERROR', r); continue
    k = (r['pbo'], r['model'], r['texture'], r['status'], r['referenced_in_cfg'])
    a = agg.setdefault(k, [0, []])
    a[0] += r['faces']
    a[1].append(r['kind'])
for (p, m, t, s, ref), (n, kinds) in sorted(agg.items()):
    print(f'{p:<18} {m:<40} cfgref={str(ref):<5} {s:<15} {t:<40} faces={n:<4} lods={",".join(kinds)}')
print('ODOL models skipped:', len(odol))
