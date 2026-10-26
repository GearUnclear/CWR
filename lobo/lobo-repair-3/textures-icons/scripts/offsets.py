"""Exact face texture-field offsets for live names matching a needle, plus member SHA-256.
Usage: offsets.py <scriptsdir> needle model.p3d [...]
"""
import hashlib, sys
sys.path.insert(0, sys.argv[1])
import mlodtex
needle = sys.argv[2].lower()
for path in sys.argv[3:]:
    d = open(path, 'rb').read()
    print('==', path, 'bytes', len(d), 'sha256', hashlib.sha256(d).hexdigest())
    ls, end, size = mlodtex.lods(d)
    for l in ls:
        groups = {}
        for off, name, raw in l['textures']:
            if needle in name.lower():
                groups.setdefault(name, []).append(off)
        for name, offs in groups.items():
            print(f"   LOD {l['index']} {mlodtex.resname(l['resolution'])} {name!r} faces={len(offs)} offsets={offs}")
