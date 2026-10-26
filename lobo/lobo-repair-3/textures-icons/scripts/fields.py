"""Dump raw texture fields for faces containing a needle, plus the LOD's live texture list.

Usage: fields.py <scriptsdir> <needle> model.p3d [max]
"""
import sys
sys.path.insert(0, sys.argv[1])
import mlodtex

needle = sys.argv[2].lower().encode('latin-1')
path = sys.argv[3]
limit = int(sys.argv[4]) if len(sys.argv) > 4 else 3
d = open(path, 'rb').read()
ls, end, size = mlodtex.lods(d)
for l in ls:
    shown = 0
    hit = [(off, name, raw) for off, name, raw in l['textures'] if needle in raw.lower()]
    if not hit:
        continue
    live = {}
    for off, name, raw in l['textures']:
        live[name] = live.get(name, 0) + 1
    print(f"LOD {l['index']} {mlodtex.resname(l['resolution'])} faces={l['faces']} hits={len(hit)}")
    for off, name, raw in hit[:limit]:
        print('   ', off, raw)
    print('    live textures:', sorted(live.items()))
