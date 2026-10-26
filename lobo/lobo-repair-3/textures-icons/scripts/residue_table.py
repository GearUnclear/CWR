"""Ledger of HYK-family texture strings in MLOD face texture fields.

For each model/LOD: count faces whose 32-byte texture field contains a needle,
split into LIVE (the engine-visible name, up to the first NUL, contains it) and
RESIDUE (field starts with NUL; the name is dead bytes). Residue faces are
further split into proxy triangles, non-visual LODs and other visual faces.

Usage: residue_table.py <scriptsdir> out.json needle[,needle] model.p3d [...]
"""
import json, math, sys
sys.path.insert(0, sys.argv[1])
from emptyfaces_lib import parse, visual, area  # noqa

needles = [n.lower() for n in sys.argv[3].split(',')]
rows = []
for path in sys.argv[4:]:
    lods = parse(open(path, 'rb').read())
    for lod in lods:
        for k, f in enumerate(lod['faces']):
            raw = f['raw'].decode('latin-1').lower()
            hit = next((n for n in needles if n in raw), None)
            if not hit:
                continue
            live = hit in f['name'].lower()
            sels = [s for s, w in lod['sels'].items() if w[k]]
            proxy = any(s.lower().startswith('proxy:') for s in sels)
            vis = visual(lod['res'])
            if live:
                cls = 'LIVE'
            elif not vis:
                cls = 'residue-nonvisual-lod'
            elif proxy:
                cls = 'residue-proxy'
            else:
                cls = 'residue-visual-face'
            text = f['raw'][1:].split(b'\0', 1)[0].decode('latin-1') if not live else f['name']
            rows.append(dict(model=path.replace('\\', '/').split('/x/')[-1], lod=lod['index'], res=lod['res'],
                             cls=cls, text=text, face=k, offset=f['off'],
                             area=round(area(lod['pts'], f), 6), sels=sorted(sels),
                             first_byte=f['raw'][:1].hex()))
json.dump(rows, open(sys.argv[2], 'w'), indent=1)
agg = {}
for r in rows:
    key = (r['model'], r['cls'], r['text'])
    a = agg.setdefault(key, [0, set()])
    a[0] += 1
    a[1].add(r['lod'])
for (m, c, t), (n, ls) in sorted(agg.items()):
    print(f'{m:<45} {c:<22} {t:<34} faces={n:<4} lods={sorted(ls)}')
tot = {}
for r in rows:
    tot[r['cls']] = tot.get(r['cls'], 0) + 1
print('TOTAL', tot)
