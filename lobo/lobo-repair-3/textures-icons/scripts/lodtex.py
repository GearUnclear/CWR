"""List every LOD's distinct live textures with face counts. Usage: lodtex.py <scriptsdir> model.p3d"""
import sys
sys.path.insert(0, sys.argv[1])
import mlodtex
d = open(sys.argv[2], 'rb').read()
ls, end, size = mlodtex.lods(d)
for l in ls:
    c = {}
    for off, name, raw in l['textures']:
        c[name] = c.get(name, 0) + 1
    print(f"LOD {l['index']} {mlodtex.resname(l['resolution'])} faces={l['faces']}")
    for k, v in sorted(c.items()):
        print(f'    {v:5d}  {k!r}')
